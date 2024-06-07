#version 450

in vec3 FragColor;
in vec3 TexCoords;
in flat int SpriteIndex;

uniform sampler2DArray textureArray;

out vec4 color;

void main()
{
    color = vec4(TexCoords, 1.);
    //uint chunkIndex = tiles[0];
    //uint tileIndex = chunkMap[chunkIndex];
    //uint tileIndex1 = chunkMap[chunkIndex+1];
    //uint tileIndex2 = chunkMap[chunkIndex+2];
    
    //float b = 1;

    //color = vec4(vec3(float(tiles[0])/255., float(tiles[1])/255., float(tiles[2])/255.), 1.0f);
    //color = texture(textureArray, vec3(TexCoords.xy, 1.));
}