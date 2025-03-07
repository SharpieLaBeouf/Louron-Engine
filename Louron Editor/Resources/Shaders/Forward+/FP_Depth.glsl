#SHADER VERTEX

#version 450 core

layout (location = 0) in vec3   aPos;
layout (location = 5) in ivec4  aBoneIDs;
layout (location = 6) in vec4   aBoneWeights;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;

const int MAX_BONE_INFLUENCE = 4;
uniform bool u_Skinned;
uniform uint u_BoneOffset;
uniform uint u_BoneCount;

layout(std430, binding = 7) readonly buffer BoneTransformation_Buffer { mat4 data[]; } BoneTransformation_Buffer_Data;

void main() 
{
    vec4 local_pos = vec4(aPos, 1.0);

    if (u_Skinned)
    {
        vec4 total_position = vec4(0.0);
        
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(aBoneIDs[i] == -1) 
                continue;

            if(aBoneIDs[i] >= u_BoneCount) 
            {
                total_position = vec4(aPos, 1.0);
                break;
            }

            mat4 bone_transform = BoneTransformation_Buffer_Data.data[u_BoneOffset + aBoneIDs[i]];
            
            total_position += (bone_transform * vec4(aPos, 1.0)) * aBoneWeights[i];
         }
        
        local_pos = total_position;
    }


    gl_Position = u_Proj * u_View * u_Model * local_pos;
}

#SHADER FRAGMENT

#version 450 core

struct Entity
{
    uint entity_id;
    float depth;
};

layout(std430, binding = 10) buffer EntityBuffer { Entity data[]; } EntityBuffer_Data;

uniform uint u_EntityID;
uniform ivec2 u_ScreenSize;

void main() 
{
    uint index = uint(gl_FragCoord.y) * uint(u_ScreenSize.x) + uint(gl_FragCoord.x);
    
    float existingDepth = EntityBuffer_Data.data[index].depth;
    if (gl_FragCoord.z < existingDepth) {
        EntityBuffer_Data.data[index].entity_id = u_EntityID;
        EntityBuffer_Data.data[index].depth = gl_FragCoord.z;
    }
}