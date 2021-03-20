#define TINYGLTF_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#define TINYGLTF_USE_RAPIDJSON

#include <ModelImporter.h>

#include <tiny_gltf.h>

using namespace tinygltf;

namespace TF
{
    const std::string ATTRIBUTE_POSITION = "POSITION";
    const std::string ATTRIBUTE_NORMAL = "NORMAL";
    const std::string ATTRIBUTE_TANGENT = "TANGENT";
    const std::string ATTRIBUTE_TEXCOORD_0 = "TEXCOORD_0";

    //void LoadNode(World* world, const tinygltf::Model& glTFModel, const tinygltf::Node& inputNode)
    //{
    //    std::cout << "Node:" << inputNode.name << std::endl;
    //    if (inputNode.mesh > -1)
    //    {
    //        std::cout << "Mesh:" << glTFModel.meshes[inputNode.mesh].name << std::endl;
    //        const tinygltf::Mesh glTFMesh = glTFModel.meshes[inputNode.mesh];
    //        int vertexCount = 0;
    //        for (auto i = 0; i < glTFMesh.primitives.size(); i++)
    //        {
    //            const auto& glTFPrimitive = glTFMesh.primitives[i];
    //            const float* positionBuffer = nullptr; // POSITION
    //            const float* normalsBuffer = nullptr;  // NORMAL
    //            const float* tangentBuffer = nullptr;  // TANGENT
    //            const float* texCoordsBuffer = nullptr;// TEXCOORD_0
    //            if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
    //            {
    //                const tinygltf::Accessor& accessor = glTFModel.accessors[glTFPrimitive.attributes.find("POSITION")->second];
    //                const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
    //                positionBuffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
    //                vertexCount = accessor.count;
    //                std::cout << vertexCount << std::endl;
    //            }
    //        }
    //        
    //    }
    //    for (auto i = 0; i < inputNode.children.size(); i++)
    //    {
    //        LoadNode(glTFModel, glTFModel.nodes[inputNode.children[i]]);
    //    }
    //}

    int AccessAttributeBuffer(const tinygltf::Model& glTFModel, const tinygltf::Primitive& glTFPrimitive, std::string attribute, const float*& buffer)
    {
        if (glTFPrimitive.attributes.find(attribute) != glTFPrimitive.attributes.end())
        {
            const tinygltf::Accessor& accessor = glTFModel.accessors[glTFPrimitive.attributes.find(attribute)->second];
            const tinygltf::BufferView& view = glTFModel.bufferViews[accessor.bufferView];
            buffer = reinterpret_cast<const float*>(&(glTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            return accessor.count;
        }
        return 0;
    }

    uint32_t LoadIndexBuffer(const tinygltf::Model& glTFModel, const tinygltf::Primitive& glTFPrimitive, std::vector<uint32_t>& indexBuffer, int vertexStart)
    {
        const tinygltf::Accessor& accessor = glTFModel.accessors[glTFPrimitive.indices];
        const tinygltf::BufferView& bufferView = glTFModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = glTFModel.buffers[bufferView.buffer];

        int indexCount = static_cast<uint32_t>(accessor.count);
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
            return 0;
        }
        return indexCount;
    }

    void LoadMesh(const tinygltf::Model& glTFModel, const tinygltf::Node& glTFNode, std::vector<Vertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
    {
        if (glTFNode.mesh > -1)
        {
            const tinygltf::Mesh glTFMesh = glTFModel.meshes[glTFNode.mesh];
            for (auto i = 0; i < glTFMesh.primitives.size(); i++)
            {
                const float* positionBuffer = nullptr; // POSITION
                const float* normalsBuffer = nullptr;  // NORMAL
                const float* tangentBuffer = nullptr;  // TANGENT
                const float* texCoordsBuffer = nullptr;// TEXCOORD_0
                uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
                uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
                uint32_t indexCount = 0;
                size_t vertexCount = 0;
                vertexCount = AccessAttributeBuffer(glTFModel, glTFMesh.primitives[i], ATTRIBUTE_POSITION, positionBuffer);
                AccessAttributeBuffer(glTFModel, glTFMesh.primitives[i], ATTRIBUTE_NORMAL, normalsBuffer);
                AccessAttributeBuffer(glTFModel, glTFMesh.primitives[i], ATTRIBUTE_TANGENT, tangentBuffer);
                AccessAttributeBuffer(glTFModel, glTFMesh.primitives[i], ATTRIBUTE_TEXCOORD_0, texCoordsBuffer);
                for (auto v = 0; v < vertexCount; v++)
                {
                    Vertex vert{};
                    vert.pos = float3(positionBuffer[v * 3], positionBuffer[v * 3 + 1], positionBuffer[v * 3 + 2]);
                    vert.normal = normalsBuffer ? float3(normalsBuffer[v * 3], normalsBuffer[v * 3 + 1], normalsBuffer[v * 3 + 2]) : float3(0, 0, 0);
                    vert.tangent = tangentBuffer ? float3(tangentBuffer[v * 3], tangentBuffer[v * 3 + 1], tangentBuffer[v * 3 + 2]) : float3(0, 0, 0);
                    vert.uv = texCoordsBuffer ? float2(texCoordsBuffer[v * 2], texCoordsBuffer[v * 2 + 1]) : float2(0, 0);
                    vertexBuffer.push_back(vert);
                }
                indexCount = LoadIndexBuffer(glTFModel, glTFMesh.primitives[i], indexBuffer, vertexStart);
            }
        }

        if (glTFNode.children.size() > 0)
        {
            for (auto i = 0; i < glTFNode.children.size(); i++)
            {
                LoadMesh(glTFModel, glTFModel.nodes[glTFNode.children[i]], vertexBuffer, indexBuffer);
            }
        }

    }

    void LoadMesh(const tinygltf::Model& glTFModel, std::vector<Vertex>& vertexBuffer, std::vector<uint32_t>& indexBuffer)
    {
        for (auto sid = 0; sid < glTFModel.scenes.size(); sid++)
        {
            const Scene& scene = glTFModel.scenes[sid];
            for (auto i = 0; i < scene.nodes.size(); i++)
            {
                const Node node = glTFModel.nodes[scene.nodes[i]];
                LoadMesh(glTFModel, node, vertexBuffer, indexBuffer);
            }
        }
    }

    void LoadMaterials(const tinygltf::Model& glTFModel, std::vector<std::shared_ptr<Material>>& materials)
    {
        materials.resize(glTFModel.materials.size());
        for (size_t i = 0; i < glTFModel.materials.size(); i++)
        {
            auto glTFMaterial = glTFModel.materials[i];
            std::shared_ptr<Material> materialPtr = std::make_shared<Material>();
            if (glTFMaterial.values.find(GLTF_BASE_COLOR_FACTOR) != glTFMaterial.values.end())
            {
                double* baseColorPtr = glTFMaterial.values[GLTF_BASE_COLOR_FACTOR].ColorFactor().data();
                materialPtr.get()->m_baseColorFactor = float4(baseColorPtr[0], baseColorPtr[1], baseColorPtr[2], baseColorPtr[3]);
            }
            if (glTFMaterial.values.find(GLTF_METALLIC_FACTOR) != glTFMaterial.values.end())
            {
                materialPtr.get()->m_metallicFactor = glTFMaterial.values[GLTF_METALLIC_FACTOR].Factor();
            }
            if (glTFMaterial.values.find(GLTF_ROUGHNESS_FACTOR) != glTFMaterial.values.end())
            {
                materialPtr.get()->m_roughnessFactor = glTFMaterial.values[GLTF_ROUGHNESS_FACTOR].Factor();
            }

            std::cout << "Material ID:" << i << " : " << "baseColor: " << materialPtr.get()->m_baseColorFactor.x << " " <<
                materialPtr.get()->m_baseColorFactor.y << " " << materialPtr.get()->m_baseColorFactor.z << " " <<
                materialPtr.get()->m_baseColorFactor.w << std::endl << "metallic: " << materialPtr.get()->m_metallicFactor << " "
                << "roughness: " << materialPtr.get()->m_roughnessFactor << std::endl;
        }
    }

    void LoadGltfFModel(World* world, fs::path gltfPath)
    {

        Model glTFModel;
        TinyGLTF loader;
        std::string err;
        std::string warn;

        std::vector<uint32_t> indexBuffer; // prepare cpu data for uploading
        std::vector<Vertex> vertexBuffer;

        std::vector<std::shared_ptr<Material>> glTFMaterials;

        std::vector<uint32_t> rendererIndexStart;
        std::vector<uint32_t> rendererIndexCount;
        std::vector<std::shared_ptr<Mesh>> rendererMeshes;
        std::vector<std::shared_ptr<Material>>renderMeshes;
        std::vector<float3> rendererPos;
        std::vector<float4> rendererRot;
        std::vector<float3> rendererScale;

        bool fileLoaded = loader.LoadASCIIFromFile(&glTFModel, &err, &warn, gltfPath.generic_string());
        if (!fileLoaded)
        {
            throw std::runtime_error("failed to open gltf file!" + gltfPath.generic_string());
        }
        
        
        
        // Load Texture
        for (size_t i = 0; i < glTFModel.images.size(); i++)
        {
            tinygltf::Image& glTFImage = glTFModel.images[i];
            std::cout << glTFImage.name << "\t" <<glTFImage.uri << std::endl;
        }
        
        // Load Material
        LoadMaterials(glTFModel, glTFMaterials);
        // Load Mesh
        LoadMesh(glTFModel, vertexBuffer, indexBuffer);
        std::shared_ptr<Mesh> mesh = Mesh::GetOrCreateMesh(gltfPath.generic_string(), vertexBuffer, indexBuffer);
        // Instantiate objects in the scene
        //for (auto sid = 0; sid < glTFModel.scenes.size(); sid++)
        /*{
            const Scene& scene = glTFModel.scenes[sid];
            for (auto i = 0; i < scene.nodes.size(); i++)
            {
                const Node node = glTFModel.nodes[scene.nodes[i]];
                LoadNode(glTFModel, node, nullptr, vertexBuffer, indexBuffer);
            }
        }*/
        //UploadModel(vertexBuffer, indexBuffer);
        
        // Register Shader To RenderManager
        world->CreateEntity("");
    }
}