#pragma once

#include "Texture.hpp"
#include <vector>

class TextureAtlas {
private:
	GLuint textureID;
	int atlasSize;
	int numLayers;
	int numChannels = 3;
	std::vector<unsigned char> atlasData;

public:
	void init(int size, int channels);
	void place(const TextureData& textureData, int layer, int startX, int startY, int startChannel);
	void use(uint32_t textureSlotNum) const;
	~TextureAtlas();
};

void TextureAtlas::init(int size, int layers) {
	atlasSize = size, numLayers = layers;
	atlasData.resize(size * size * numChannels * numLayers);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, size, size, layers, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void TextureAtlas::place(const TextureData& textureData, int layer, int startX, int startY, int startChannel = 0) {
	// Проверка размеров
	if (startX + textureData.width > atlasSize || startY + textureData.height > atlasSize || startChannel + textureData.channels !=3) {
		std::cout << "invalid texture data\n";
		return;
	}

	// Копирование данных текстуры в атлас
	for (int i = 0; i < textureData.height; ++i) {
		for (int j = 0; j < textureData.width; ++j) {
			for (int k = 0; k < textureData.channels; ++k) {
				int atlasIndex = ((startY + i) * atlasSize + (startX + j)) * 3 + (startChannel + k) % 3;
				int textureIndex = (i * textureData.width + j) * textureData.channels + k;
				atlasData[atlasIndex] = textureData.bytes[textureIndex];
			}
		}
	}

	// Обновление текстуры на GPU
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, atlasSize, atlasSize, 1, GL_RGB, GL_UNSIGNED_BYTE, atlasData.data());
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void TextureAtlas::use(uint32_t textureSlotNum) const {
	glActiveTexture(GL_TEXTURE0 + textureSlotNum);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
}

TextureAtlas::~TextureAtlas() {
	// Очистка текстуры
	glDeleteTextures(1, &textureID);
}
