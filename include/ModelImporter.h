#pragma once

#include <memory>
#include <TFVulkanDevice.h>
#include <Vfs.h>

#include <World.h>
#include <Mesh.h>
namespace TF
{
    // GLTF will generally be loaded as sperate objects
    // consider the design later
    void LoadGltfFModel(TFVkGfxDevice tfVulkanDevice, World* world, fs::path gltfPath);
    // On the other hand obj file will treated as single objects
    void LoadObjModel(TFVkGfxDevice tfVulkanDevice, World* world, fs::path objFilePath);

}