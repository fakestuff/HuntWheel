#pragma once
#include <MathHelper.h>

namespace TF
{
class Transform
{
public:
    /*
    TODO:
    float3 GetTranslate()
    {
       
    }
    float3 GetScale()
    {

    }
    float3 GetRotation()
    {

    }
    */
    float4x4 m_localMatrix;
    float4x4 m_worldModelMatrix;
};
}
