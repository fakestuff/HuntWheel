#pragma once
#include <ObjModel.h>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tiny_obj_loader.h>
#include <iostream>


ObjModel::ObjModel(fs::path objPath,TF::TFVulkanDevice tfVulkanDevice)
{
    m_vulkanDevice = tfVulkanDevice;
    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = objPath.parent_path().generic_string(); // Path to material files
    tinyobj::ObjReader reader;
    this->name = objPath.filename().generic_string();
    if (!reader.ParseFromFile(objPath.generic_string(), readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) 
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    int totalVerticesCount = 0;
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        for (size_t s = 0; s < shapes.size(); s++) 
        {
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
            {
                int fv = shapes[s].mesh.num_face_vertices[f];
                totalVerticesCount += fv;
            }
        }
    }
    m_vertexBuffer.reserve(totalVerticesCount);


    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) 
        {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
                tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
                tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
                tinyobj::real_t nx = attrib.normals[3*idx.normal_index+0];
                tinyobj::real_t ny = attrib.normals[3*idx.normal_index+1];
                tinyobj::real_t nz = attrib.normals[3*idx.normal_index+2];
                tinyobj::real_t tx = attrib.texcoords[2*idx.texcoord_index+0];
                tinyobj::real_t ty = attrib.texcoords[2*idx.texcoord_index+1];
                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
                Vertex vertex;
                vertex.pos = float3(vx, vy, vz);
                vertex.normal = float3(nx, ny, nz);
                vertex.tangent = float3(0,1,0);
                vertex.uv = float2(tx,ty);
                m_vertexBuffer.push_back(vertex);

            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }
    size_t vertexBufferSize = m_vertexBuffer.size() * sizeof(ObjModel::Vertex);
    struct StagingBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    } vertexStaging;
    void* data;

    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStaging.buffer, vertexStaging.memory);     
    vkMapMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, m_vertexBuffer.data(), vertexBufferSize);
    vkUnmapMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory);
    

    CreateBuffer(m_vulkanDevice.physicsDevice,m_vulkanDevice.logicalDevice,vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_gpuVertexBuffer.buffer, m_gpuVertexBuffer.memory);
    
    CopyBuffer(m_vulkanDevice.logicalDevice, m_vulkanDevice.graphicsQueue, m_vulkanDevice.cmdPool,vertexStaging.buffer, m_gpuVertexBuffer.buffer, vertexBufferSize);

    vkDestroyBuffer(m_vulkanDevice.logicalDevice, vertexStaging.buffer, nullptr);
    vkFreeMemory(m_vulkanDevice.logicalDevice, vertexStaging.memory, nullptr);
}