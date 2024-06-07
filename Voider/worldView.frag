#version 430 core

in vec2 Pos;
out vec4 FragColor;

uniform vec2 chunkOffset;

uniform int chunkMapSide;

uniform int chunkSizeSq;

uniform sampler2DArray textureArray;

const int chunkSize = 32;

struct Chunk
{
 ivec2 offset;
 uint tiles[chunkSize*chunkSize];
};

layout(std430, binding = 0) buffer TilesBuffer
{
   Chunk chunks[];
};

void main()
{
    vec2 borderOffset = vec2(0.5-chunkMapSide/2);

    if(Pos.x>=chunkOffset.x+1+borderOffset.x && Pos.y>=chunkOffset.y+1+borderOffset.y && Pos.x<=chunkMapSide+chunkOffset.x-1+borderOffset.x && Pos.y<=chunkMapSide+chunkOffset.y-1+borderOffset.y)
    {
        ivec2 pos = ivec2(round(Pos));
        ivec2 worldPos = pos;
        pos%=chunkMapSide;
        uint chunkIndex = pos.y*chunkMapSide+pos.x;
        ivec2 localPos = ivec2(fract(Pos)*chunkSize);
        if(localPos.x>=chunkSize/2)
            localPos.x-=chunkSize/2;
        else
            localPos.x+=chunkSize/2;
        if(localPos.y>=chunkSize/2)
            localPos.y-=chunkSize/2;
        else
            localPos.y+=chunkSize/2;
        uint localIndex = min(chunkSize-1, localPos.y)*chunkSize+min(localPos.x, chunkSize-1);
        uint tile = chunks[chunkIndex].tiles[localIndex];
        uint tile0 = (tile & 0x000000FFu);
        uint tile1 = (tile & 0x0000FF00u) >> 8;
        uint tile2 = (tile & 0x00FF0000u) >> 16;
        vec3 color = vec3(tile0, tile1, tile2)/256.f;
        if(chunks[chunkIndex].offset!=worldPos)
        {
            color = vec3(1.);
        }
        FragColor = vec4(color, 1.0f);
    }
}
