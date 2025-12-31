#version 450

#extension GL_EXT_nonuniform_qualifier : enable
layout (set = 0, binding = 0) uniform texture2D textures[];
layout (set = 0, binding = 1) uniform sampler samplers[];

layout (set = 1, binding = 0) uniform SceneUBO {
    mat4 projection;
    mat4 view;
    vec3 position;
    float unused_0;
} scene;

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint bindlessImageIndex;
} pushConstants;


//===STAGE IN===============================================
layout (location = 0) in StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
    vec3 worldPos;
} stageIn;
//==========================================================

//===STAGE OUT==============================================
layout (location = 0) out vec4 outFragColor;
//==========================================================

#define NEAREST_SAMPLER_ID 0
#define LINEAR_SAMPLER_ID  1

vec4 sampleTexture2DLinear(uint texID, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(textures[texID], samplers[LINEAR_SAMPLER_ID])), uv);
}

vec4 sampleTexture2DNearest(uint texID, vec2 uv) {
    return texture(nonuniformEXT(sampler2D(textures[texID], samplers[NEAREST_SAMPLER_ID])), uv);
}


void main() {

    if(scene.unused_0 > 0.1f){
        vec4 unusedCol = vec4(0, 1, 0, 1);
        outFragColor = unusedCol;
    }
    else{
        uint imageIndex = pushConstants.bindlessImageIndex;
        vec2 uv = {0.5f, 0.5f};
    	vec4 texCol = sampleTexture2DLinear(imageIndex, uv);
    	outFragColor = texCol;
    }
}
