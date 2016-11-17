#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

#define MAX_JOINTS 32

attribute vec4 inPosition;
attribute vec3 inNormal;

#if defined (SKINNING)
    attribute vec4 inJointIndex;
    attribute vec4 inJointWeight;
#endif

uniform mat4 worldViewProjMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 projMatrix;
uniform mat3 worldViewInvTransposeMatrix;

#if defined (SKINNING)
uniform vec4 jointPositions[MAX_JOINTS]; // (x, y, z, scale)
uniform vec4 jointQuaternions[MAX_JOINTS];    
#endif

uniform mediump float silhouetteScale;
uniform mediump float silhouetteExponent;

void main()
{
#if defined (SKINNING)
    int index = int(inJointIndex);
    vec4 weightedVertexPosition = jointPositions[index];
    vec4 weightedVertexQuaternion = jointQuaternions[index];

    vec3 t = 2.0 * cross(weightedVertexQuaternion.xyz, inPosition.xyz);
    vec3 jointTransformPosition = weightedVertexPosition.xyz + (inPosition.xyz + weightedVertexQuaternion.w * t + cross(weightedVertexQuaternion.xyz, t)) * weightedVertexPosition.w;
    vec4 position = vec4(jointTransformPosition, inPosition.w);

    t = 2.0 * cross(weightedVertexQuaternion.xyz, inNormal);
    vec3 jointTransformTangent = inNormal + weightedVertexQuaternion.w * t + cross(weightedVertexQuaternion.xyz, t);
    vec3 normal = normalize (worldViewInvTransposeMatrix * jointTransformTangent);
#else
    vec4 position = inPosition;
    vec3 normal = normalize(worldViewInvTransposeMatrix * inNormal.xyz);
#endif

    vec4 PosView = worldViewMatrix * position;

    mediump float distanceScale = length(PosView.xyz) / 100.0;

    PosView.xyz += normal * pow(silhouetteScale * distanceScale, silhouetteExponent);
    gl_Position = projMatrix * PosView;
}