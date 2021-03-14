#pragma once
#include <unordered_map>
#include <Entity.h>
#include <iostream>
namespace TF
{
class World
{
public:
    std::unordered_map<int, Entity*> m_entities;
    int m_nextEid = 0;
    
    Entity* CreateEntity(const std::string & name)
    {
        Entity* entity = new Entity(name, m_nextEid);
        m_entities[m_nextEid] = entity;
        m_nextEid++;
        return entity;
    }
    Entity* GetEntity(int eid)
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
            delete it.second;
        }
    }
};
}