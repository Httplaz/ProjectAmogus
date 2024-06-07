#version 430 core
layout (location = 0) in vec3 aPos;

out vec2 Pos;

uniform mat4 cameraMatrix;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vec4 pos1 = cameraMatrix*vec4(aPos, 1.);
    Pos = (pos1.xy+vec2(1., 1.))/2.;
}
