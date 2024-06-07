#version 450

uniform mat4 cameraMatrix;

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec3 aInstancePos;
layout (location = 2) in float aInstanceScale;
layout (location = 3) in float aInstanceRotation;
layout (location = 4) in float aInstanceSpriteIdx;

out vec3 FragColor;
out vec3 TexCoords;
out int SpriteIndex;

void main()
{
    float s = sin(aInstanceRotation);
    float c = cos(aInstanceRotation);

    mat2 rotationMatrix = mat2(s, -c, c, s);
    vec2 rotPos = rotationMatrix * aPos.xy;
    gl_Position = cameraMatrix * vec4(rotPos * aInstanceScale + aInstancePos.xy, aInstancePos.z, 1.0);
    TexCoords = vec3(aPos.zw/4 + vec2(ivec2(int(aInstanceSpriteIdx)%4, int(aInstanceSpriteIdx)/4))*0.25, aInstancePos.z);
    SpriteIndex = int(aInstanceSpriteIdx);
}
