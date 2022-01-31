#version 330

in vec2 TexCoord;
flat in uvec3 Texpage;
flat in uvec2 Clut;

out vec4 fragColor;

uniform usampler2DRect vramSampler;

void main(void)
{
    vec2 texpageOffset = vec2(Texpage.x * 64.0f, Texpage.y * 256.0f);
    vec2 flooredTexCoord = vec2(floor(TexCoord));
    uint pixelsInTexel = 1u << (2u - Texpage.z);
    vec2 inTexPageCoord = vec2(flooredTexCoord.x / pixelsInTexel, TexCoord.y);
    uint pixelIndex = uint(flooredTexCoord.x) % pixelsInTexel;
    uvec2 texpageTexel = texture(vramSampler, inTexPageCoord + texpageOffset).rg;
    uint colorIndices = texpageTexel.r + (texpageTexel.g << 8u);
    // TODO: does not work for 16bpp
    uint colorMask = (0x10u << (4u * Texpage.z)) - 1u;
    uint bitsPerColorIndex = 4u << Texpage.z;
    uint colorIndex = (colorIndices >> (pixelIndex * bitsPerColorIndex)) & colorMask;
    vec2 clutOffset = vec2(Clut.x * 16.0f, Clut.y);
    uvec2 packedColor = texture(vramSampler, vec2(colorIndex, 0.0f) + clutOffset).rg;
    if (packedColor == uvec2(0u, 0u))
    { discard; }
    vec3 color = vec3(
                packedColor.r & 0x1fu,
                (packedColor.r >> 5u) | ((packedColor.g & 0x3u) << 3u),
                ((packedColor.g >> 2u) & 0x1fu));
    color /= 31.0f;
    fragColor = vec4(color, (packedColor.g & 0x80u) == 0x80u ? 0.5f : 1.0f);
}
