#pragma once
#include <unordered_map>
#include <MathHelper.h>
namespace TF
{
//class Texture2D;

const std::string GLTF_SHADING_MODEL_PBR_METALLIC_ROUGHNESS = "pbrMetallicRoughness";


const std::string GLTF_BASE_COLOR_FACTOR = "baseColorFactor";
const std::string GLTF_METALLIC_FACTOR = "metallicFactor";
const std::string GLTF_ROUGHNESS_FACTOR = "roughnessFactor";
const std::string GLTF_BASE_COLOR_TEXTURE = "baseColorTexture";
const std::string GLTF_METALLIC_ROUGHNESS_TEXTURE = "metallicRoughnessTexture";

class Material
{
    // * pso
    // * pipeline layout
    // 
public:
    //std::unordered_map<std::string, float> m_floatProperty;
    //std::unordered_map<std::string, float> m_texturePeoperty;
    float4 m_baseColorFactor = { 1.0f,1.0f,1.0f,1.0f };
    float m_metallicFactor = 0.04f;
    float m_roughnessFactor = 0.5f;
    float3 m_emmisiveFactor = {0.0f,0.0f,0.0f};
};
}