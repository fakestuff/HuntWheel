#pragma once
#include <World.h>
#include <iostream>
namespace TF
{
    void SpawnObjects(World* world)
    {
        auto entity0 = world->CreateEntity("Entity0");
        auto entity1 = world->CreateEntity("Entity1");
        for (auto& it: world->m_entities)
        {
            std::cout << it.first << " " << it.second->m_name << std::endl;
        }
    }
}