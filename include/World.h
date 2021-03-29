#pragma once
#include <unordered_map>
#include <memory>
#include <Entity.h>
#include <iostream>
#include <entt/entt.hpp>

namespace TF
{
class World
{
public:
    std::unordered_map<int, std::shared_ptr<Entity>> m_entities;
    std::vector<std::shared_ptr<Entity>> m_sceneRoot;
    int m_nextEid = 0;
    
    std::shared_ptr<Entity> CreateEntity(const std::string & name)
    {
        auto entityPtr = std::make_shared<Entity>(name, m_nextEid);
        m_entities[m_nextEid] = std::make_shared<Entity>(name, m_nextEid);
        m_nextEid++;
        return entityPtr;
    }


    void UpdateMatrix()
    {

    }



    std::shared_ptr<Entity> GetEntity(int eid)
    {
        return m_entities[eid];
    }
    void Init()
    {
        
    }
    void Shutdown()
    {
        for (auto& it: m_entities)
        {
            it.second.reset();
        }
    }
};
}