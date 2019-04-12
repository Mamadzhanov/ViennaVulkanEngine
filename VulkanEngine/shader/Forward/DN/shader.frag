#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../common_defines.glsl"
#include "../light.glsl"

layout(location = 0) in vec3 fragPosW;
layout(location = 1) in vec3 fragNormalW;
layout(location = 2) in vec3 fragTangentW;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform cameraUBO_t {
    cameraData_t data;
} cameraUBO;

layout(set = 1, binding = 0) uniform lightUBO_t {
    lightData_t data;
} lightUBO;

layout(set = 2, binding = 0) uniform sampler2D shadowMap[NUM_SHADOW_CASCADE];

layout(set = 3, binding = 0) uniform objectUBO_t {
    objectData_t data;
} objectUBO;

layout(set = 4, binding = 0) uniform sampler2D texSampler;
layout(set = 4, binding = 1) uniform sampler2D normalSampler;


void main() {

    //shadow
    //int sIdx = shadowIdx( cameraUBO.data, lightUBO.data, gl_FragCoord );
    //cameraData_t s = lightUBO.data.shadowCameras[sIdx];
    float shadowFactor = 1.0; //shadowFunc(fragPosW, s.shadowView, s.shadowProj, shadowMap[sIdx] );

    //parameters
    vec3 camPosW   = cameraUBO.data.camModel[3].xyz;
    vec3 lightPosW = lightUBO.data.lightModel[3].xyz;
    vec3 lightDirW = normalize( lightUBO.data.lightModel[2].xyz );
    float nfac = dot( fragNormalW, -lightDirW)<0? 0.5:1;
    vec4 lightParam = lightUBO.data.param;
    vec4 texParam   = objectUBO.data.param;

    //TBN matrix
    vec3 N        = normalize( fragNormalW );
    vec3 T        = normalize( fragTangentW );
    T             = normalize( T - dot(T, N)*N );
    vec3 B        = normalize( cross( T, N ) );
    mat3 TBN      = mat3(T,B,N);
    vec3 mapnorm  = normalize( texture(normalSampler, (fragTexCoord + texParam.zw)*texParam.xy).xyz*2.0 - 1.0 );
    vec3 normal   = normalize( TBN * mapnorm );

    vec3 ambcol  = lightUBO.data.col_ambient.xyz;
    vec3 diffcol = lightUBO.data.col_diffuse.xyz;
    vec3 speccol = lightUBO.data.col_specular.xyz;

    vec3 fragColor = texture(texSampler, (fragTexCoord + texParam.zw)*texParam.xy).xyz;

    vec3 result = ambcol * fragColor;

    result += lightUBO.data.itype[0] == 0?
                  dirlight( camPosW,
                            lightDirW, lightParam, shadowFactor,
                            vec3(0,0,0), diffcol, speccol,
                            fragPosW, normal, fragColor) * nfac : vec3(0,0,0);

    result += lightUBO.data.itype[0] == 1?
                  pointlight( camPosW,
                              lightPosW, lightParam, shadowFactor,
                              vec3(0,0,0), diffcol, speccol,
                              fragPosW, normal, fragColor) * nfac : vec3(0,0,0);

    result += lightUBO.data.itype[0] == 2?
                  spotlight( camPosW,
                             lightPosW, lightDirW, lightParam, shadowFactor,
                             vec3(0,0,0), diffcol, speccol,
                             fragPosW, normal, fragColor) * nfac : vec3(0,0,0);

    outColor = vec4( result, 1.0 );
}
