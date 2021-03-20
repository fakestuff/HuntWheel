#pragma once

#include <string>

#include <Component/Transforms.h>
#include <Component/Renderers.h>
namespace TF
{
class Entity
{
public:
    std::string m_name;
    int m_entityId;
    std::unique_ptr<Transform> m_transform; // TODO: handle transforms in bucket allocation later
    // std::shared_ptr<Mesh> m_mesh;
    // std::unique_ptr<Material> m_material;
    std::shared_ptr<Renderer> m_renderer;

    Entity(const std::string& name, int eid): m_name(name), m_entityId(eid) 
    {
        m_transform = std::unique_ptr<Transform>(new Transform());
    }

};
}