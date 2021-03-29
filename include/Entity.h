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
    std::shared_ptr<Transform> m_transform; // TODO: handle transforms in bucket allocation later
    std::shared_ptr<Renderer> m_renderer;
    std::vector<std::shared_ptr<Entity>> m_children;

    Entity(const std::string& name, int eid): m_name(name), m_entityId(eid) 
    {
        m_transform = std::shared_ptr<Transform>(new Transform());
    }

};
}