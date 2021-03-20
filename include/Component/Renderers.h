#pragma once
#include <memory>
#include <Mesh.h>
#include <Material.h>
namespace TF
{
class Renderer
{
public:
    // Meshes
    std::shared_ptr<Mesh> m_mesh;
    // Materials
    std::shared_ptr<Material> m_material;
    uint32_t m_indexStart;
    uint32_t m_indexCount;
};
}