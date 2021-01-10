#define TINYGLTF_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define TINYGLTF_USE_RAPIDJSON
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
//#include <rapidjson
#include <iostream>
#include <gltf_loader.h>
#include <VulkanHelper.h>
#include <VulkanBufferHelper.h>
using namespace tinygltf;

void TestLoadModel()
{
    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "../../res/model/FlightHelmet/FlightHelmet.gltf");
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
    printf("Failed to parse glTF\n");
    }
    else
    {
        printf("parsing success\n");
    }
    for(int i = 0;i<model.nodes.size();i++)
    {
        printf("Node name: %s\n", model.nodes[i].name.c_str());
        if (model.nodes[i].mesh > -1)
        {
            const tinygltf::Mesh mesh = model.meshes[model.nodes[i].mesh];
            for (int j = 0; j < mesh.primitives.size(); j++)
            {
                const tinygltf::Primitive& glTFPrimitive = mesh.primitives[j];
                for(auto it = glTFPrimitive.attributes.begin();it != glTFPrimitive.attributes.end();it++)
                {
                    std::cout << it->first <<std::endl;
                }
            }
        }
    }
    
}


GltfModel::GltfModel(TF::TFVulkanDevice vulkanDevice,std::string path, float scale)
{
    m_scale = scale;
    m_vulkanDevice = vulkanDevice;
    Model modelAsset;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    std::vector<uint32_t> indexBuffer; // prepare cpu data for uploading
    std::vector<Vertex> vertexBuffer;

    bool fileLoaded = loader.LoadASCIIFromFile(&modelAsset, &err, &warn, path);
    if (!fileLoaded)
    {
        throw std::runtime_error("failed to open gltf file!");
    }
    else
    {
        // load images
        LoadImages(modelAsset);
        // load materials
        LoadMaterials(modelAsset);
        // load textures
        LoadTextures(modelAsset);
        const Scene& scene = modelAsset.scenes[0];
        for (auto i =0; i<scene.nodes.size();i++)
        {
            const Node node = modelAsset.nodes[scene.nodes[i]];
            LoadNode(node, modelAsset, nullptr, vertexBuffer,indexBuffer);
        }
        UploadModel(vertexBuffer,indexBuffer);
    }
}

GltfModel::~GltfModel()
{
    for (int i = 0; i<m_images.size();i++)
    {
        m_images[i].texture.Destroy();
    }
    vkDestroyBuffer(m_vulkanDevice.logicalDevice, m_vertexBuffer.buffer, nullptr);
    vkFreeMemory(m_vulkanDevice.logicalDevice, m_vertexBuffer.memory, nullptr);
    vkDestroyBuffer(m_vulkanDevice.logicalDevice, m_indexBuffer.buffer, nullptr);
    vkFreeMemory(m_vulkanDevice.logicalDevice, m_indexBuffer.memory, nullptr);
}

void GltfModel::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& modelAsset, 
                    SceneNode* parent,std::vector<Vertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
{
    SceneNode node{};
    node.matrix = Matrix::Identity;
    // Get the local node matrix
    // It's either made up from translation, rotation, scale or a 4x4 matrix
    if (inputNode.scale.size() == 3) {
        node.matrix*= Matrix::CreateScale(inputNode.scale[0],inputNode.scale[1],inputNode.scale[2]);
    }
    if (inputNode.rotation.size() == 4) {
        float4x4 rot = Matrix::CreateFromQuaternion(Quaternion(inputNode.rotation[0],inputNode.rotation[1],inputNode.rotation[2],inputNode.rotation[3]));

        node.matrix *= rot;
    }
    if (inputNode.translation.size() == 3) {
        node.matrix *= Matrix::CreateTranslation(float3(inputNode.translation[0], inputNode.translation[1], inputNode.translation[2]));
    }
    
    
    if (inputNode.matrix.size() == 16) {
        float m[16];
        for (int i = 0;i<16;i++)
        {
            m[i] = inputNode.matrix[i];
        }
        node.matrix = Matrix::Matrix(m);
    };

    if (inputNode.children.size() > 0)
    {
        for (auto i = 0;i < inputNode.children.size();i++)
        {
            LoadNode(modelAsset.nodes[inputNode.children[i]], modelAsset, &node, vertexBuffer, indexBuffer );
        }
    }
    // If the node contains mesh data, we load vertices and indices from the buffers
    // In glTF this is done via accessors and buffer views

    if (inputNode.mesh > -1)
    {
        const tinygltf::Mesh glTFMesh = modelAsset.meshes[inputNode.mesh];
        // Iterate through all primitives of node
        for (auto i = 0; i<glTFMesh.primitives.size();i++)
        {
            const tinygltf::Primitive& glTFPrimitive = glTFMesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            size_t vertexCount = 0;
            // Verticies
            {
                const float* positionBuffer = nullptr; // POSITION
                const float* normalsBuffer = nullptr;  // NORMAL
                const float* tangentBuffer = nullptr;  // TANGENT
                const float* texCoordsBuffer = nullptr;// TEXCOORD_0
                
                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = modelAsset.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = modelAsset.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(modelAsset.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }
                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = modelAsset.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = modelAsset.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(modelAsset.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = modelAsset.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
                    const tinygltf::BufferView& view = modelAsset.bufferViews[accessor.bufferView];
                    tangentBuffer = reinterpret_cast<const float*>(&(modelAsset.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = modelAsset.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = modelAsset.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(modelAsset.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                for (auto v = 0; v < vertexCount; v++)
                {
                    Vertex vert{};
                    vert.pos = float3(positionBuffer[v*3],positionBuffer[v*3+1],positionBuffer[v*3+2]);
                    vert.normal = normalsBuffer ? float3(normalsBuffer[v*3],normalsBuffer[v*3+1],normalsBuffer[v*3+2]) : float3(0,0,0);
                    vert.tangent = tangentBuffer ? float3(tangentBuffer[v*3],tangentBuffer[v*3+1],tangentBuffer[v*3+2]) : float3(0,0,0);
                    vert.uv = texCoordsBuffer ?float2(texCoordsBuffer[v*2],texCoordsBuffer[v*2+1]) : float2(0,0);
                    vertexBuffer.push_back(vert);
                }

            }
            // Indices
            {
                const tinygltf::Accessor& accessor = modelAsset.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView = modelAsset.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = modelAsset.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);
                // glTF supports different component types of indices
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: 
                {
                    uint32_t* buf = new uint32_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: 
                {
                    uint16_t* buf = new uint16_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: 
                {
                    uint8_t* buf = new uint8_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
                }
            }
            Primitive primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = glTFPrimitive.material;
            node.mesh.primitives.push_back(primitive);
        }     
    }


    if (parent)
    {
        parent->children.push_back(node);
    }
    else
    {
        m_nodes.push_back(node);
    }
}

void GltfModel::LoadImages(tinygltf::Model& modelAsset)
{
    m_images.resize(modelAsset.images.size());
    for (size_t i = 0; i < modelAsset.images.size(); i++) 
    {
        tinygltf::Image& glTFImage = modelAsset.images[i];

        unsigned char* buffer = nullptr;
        VkDeviceSize bufferSize = 0;
        bool deleteBuffer = false;
        // We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
        if (glTFImage.component == 3) 
        {
            bufferSize = glTFImage.width * glTFImage.height * 4;
            buffer = new unsigned char[bufferSize];
            unsigned char* rgba = buffer;
            unsigned char* rgb = &glTFImage.image[0];
            for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) 
            {
                memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                rgba += 4;
                rgb += 3;
            }
            deleteBuffer = true;
        }
        else 
        {
            buffer = &glTFImage.image[0];
            bufferSize = glTFImage.image.size();
        }
        std::cout<<glTFImage.uri<<std::endl;
        TF::TextureUsage usage = TF::TextureUsage::Albedo;
        if (glTFImage.uri.find("BaseColor") != std::string::npos)
        {
            usage = TF::TextureUsage::Albedo;
        }
        else if (glTFImage.uri.find("Normal")!= std::string::npos)
        {
            usage = TF::TextureUsage::Normal;
        }
        else if(glTFImage.uri.find("Metal")!= std::string::npos)
        {
            usage = TF::TextureUsage::MetallicRoughness;
        }
        m_images[i].texture.FromBuffer(buffer, bufferSize, VK_FORMAT_R8G8B8A8_UNORM, glTFImage.width, glTFImage.height, &m_vulkanDevice, m_vulkanDevice.graphicsQueue,VK_FILTER_LINEAR,4U,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,usage);
        if (deleteBuffer) {
            delete buffer;
        }
    }
}


void GltfModel::LoadTextures(tinygltf::Model& modelAsset)
{
    m_textures.resize(modelAsset.textures.size());
    for (size_t i = 0; i < modelAsset.textures.size(); i++) {
        m_textures[i].imageIndex = modelAsset.textures[i].source;
    }
}

void GltfModel::LoadMaterials(tinygltf::Model& modelAsset)
{
    m_materials.resize(modelAsset.materials.size());
    for (size_t i = 0; i < modelAsset.materials.size(); i++) {
        // We only read the most basic properties required for our sample
        tinygltf::Material glTFMaterial = modelAsset.materials[i];
        std::cout << glTFMaterial.name <<std::endl;

        // Get the base color factor
        // if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
        //     materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        // }
        // Get base color texture index
        if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            std::cout << glTFMaterial.values["baseColorTexture"].TextureIndex() <<std::endl;
            m_materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
            
            //"metallicRoughnessTexture"
            //"normalTexture"
        }
        
        if (glTFMaterial.normalTexture.index > -1) {
            std::cout << glTFMaterial.normalTexture.index <<std::endl;
            m_materials[i].normalTextureIndex = glTFMaterial.normalTexture.index;
            //"metallicRoughnessTexture"
        }
        
        if (glTFMaterial.values.find("metallicRoughnessTexture") != glTFMaterial.values.end()) {
            std::cout << glTFMaterial.values["metallicRoughnessTexture"].TextureIndex() <<std::endl;
            m_materials[i].metallicRoughnessTextureIndex = glTFMaterial.values["metallicRoughnessTexture"].TextureIndex();
            //"metallicRoughnessTexture"
        }
            
    }
}

void GltfModel::UploadModel( const std::vector<Vertex>& uploadingVertexBuffer,const std::vector<uint32_t>& uploadingIndexBuffer)
{
    size_t vertexBufferSize = uploadingVertexBuffer.size() * sizeof(GltfModel::Vertex);
    size_t indexBufferSize = uploadingIndexBuffer.size() * sizeof(uint32_t);
    std::cout <<vertexBufferSize<<std::endl;
    m_indexBuffer.count = static_cast<uint32_t>(uploadingIndexBuffer.size());
    struct StagingBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertexStaging, indexStaging;


    // m_logicalDevice.
    // Create host visible staging buffers (source)
    // VK_CHECK_RESULT(m_logicalDevice.createBuffer(
    //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //     vertexBufferSize,
    //     &vertexStaging.buffer,
    //     &vertexStaging.memory,
    //     vertexBuffer.data()));
    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStaging.buffer, vertexStaging.memory);
        
    void* data;
    vkMapMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, uploadingVertexBuffer.data(), vertexBufferSize);
    vkUnmapMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory);

    // // Index data
    // VK_CHECK_RESULT(vulkanDevice.createBuffer(
    //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //     indexBufferSize,
    //     &indexStaging.buffer,
    //     &indexStaging.memory,
    //     indexBuffer.data()));
    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStaging.buffer, indexStaging.memory);
        
    vkMapMemory(m_vulkanDevice.logicalDevice, indexStaging.memory, 0, indexBufferSize, 0, &data);
    memcpy(data, uploadingIndexBuffer.data(), indexBufferSize);
    vkUnmapMemory(m_vulkanDevice.logicalDevice, indexStaging.memory);

    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer.buffer, m_vertexBuffer.memory);
    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer.buffer, m_indexBuffer.memory);

    CopyBuffer(m_vulkanDevice.logicalDevice, m_vulkanDevice.graphicsQueue, m_vulkanDevice.cmdPool,vertexStaging.buffer, m_vertexBuffer.buffer, vertexBufferSize);
    CopyBuffer(m_vulkanDevice.logicalDevice, m_vulkanDevice.graphicsQueue, m_vulkanDevice.cmdPool,indexStaging.buffer, m_indexBuffer.buffer, indexBufferSize);

    vkDestroyBuffer(m_vulkanDevice.logicalDevice, vertexStaging.buffer, nullptr);
    vkFreeMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory, nullptr);
    vkDestroyBuffer(m_vulkanDevice.logicalDevice, indexStaging.buffer, nullptr);
    vkFreeMemory(m_vulkanDevice.logicalDevice, indexStaging.memory, nullptr);

}

void GltfModel::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    VkBuffer vertexBuffers[] = {m_vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    //vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indexBuffer.count), 1, 0, 0, 0);

    for (auto& node : m_nodes) {
        DrawNode(commandBuffer, pipelineLayout, node);
    }
}

void GltfModel::DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const SceneNode& node)
{
    if (node.mesh.primitives.size() > 0) {
			// Pass the node's matrix via push constants
			// Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
			//glm::mat4 nodeMatrix = node.matrix;
			SceneNode* currentParent = node.parent;
			// while (currentParent) {
			// 	nodeMatrix = currentParent->matrix * nodeMatrix;
			// 	currentParent = currentParent->parent;
			// }
			// Pass the final matrix to the vertex shader using push constants
			//vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
			for (const GltfModel::Primitive& primitive : node.mesh.primitives) {
				if (primitive.indexCount > 0) {

					// Bind the descriptor for the current primitive's texture
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &m_materials[primitive.materialIndex].descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
				}
			}
		}
		for (auto& child : node.children) {
			DrawNode(commandBuffer, pipelineLayout, child);
		}
}

