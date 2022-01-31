#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aTexpage;
layout (location = 3) in vec2 aClut;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 TexCoord;
flat out uvec3 Texpage;
flat out uvec2 Clut;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
    TexCoord = aTexCoord;
    Texpage = uvec3(aTexpage);
    Clut = uvec2(aClut);
}
