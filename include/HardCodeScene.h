#pragma once
#include <World.h>
#include <iostream>
#include <ModelImporter.h>
namespace TF
{
    void SpawnObjects(World* world)
    {
        auto entity0 = world->CreateEntity("Entity0");
        auto entity1 = world->CreateEntity("Entity1");
        LoadGltfFModel(s_tfVulkanDevice, world, Vfs::ConcatPath(Vfs::GetResPath(), "model/MetalRoughSpheresNoTextures/MetalRoughSpheresNoTextures.gltf"));
        for (auto& it: world->m_entities)
        {
            std::cout << it.first << " " << it.second->m_name << std::endl;
        }
    }
}