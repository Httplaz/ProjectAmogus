#pragma once
#include <SOIL/SOIL.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

struct TextureData {
	int channels;
	int width;
	int height;
	std::vector<unsigned char> bytes;
};

class Texture {
public:
	unsigned int ID;
	TextureData data;

	static TextureData load(const std::string& path)
	{
		TextureData result;
		unsigned char* image = SOIL_load_image(path.c_str(), &result.width, &result.height, &result.channels, SOIL_LOAD_AUTO);
		std::cout << SOIL_last_result << "\n";
		std::vector<unsigned char> a(image, image + (result.width * result.height * result.channels));
		result.bytes = a;
		SOIL_free_image_data(image);
		return result;
	}

	static constexpr GLenum GetTextureInternalFormat(int channels)
	{
		if (channels == 1)
			return GL_RED;
		else if (channels == 2)
			return GL_RG;
		else if (channels == 3)
			return GL_RGB;
		else
			return GL_RGBA;
	}

	static constexpr GLenum GetTextureFormat(int channels)
	{
		if (channels == 1)
			return GL_RED;
		else if (channels == 2)
			return GL_RG;
		else if (channels == 3)
			return GL_RGB;
		else
			return GL_RGBA;
	}


	Texture(const std::string& path) {
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		unsigned char* data1 = SOIL_load_image(path.c_str(), &data.width, &data.height, &data.channels, SOIL_LOAD_RGBA);
		if (data1)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		//data = load(path);
		//std::cout << "\n\n\n" << data.bytes.size();
		//glTexImage2D(GL_TEXTURE_2D, 0, GetTextureInternalFormat(data.channels), data.width, data.height, 0, GetTextureFormat(data.channels), GL_UNSIGNED_BYTE, data.bytes.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(data1);
	}

	void use() {
		glBindTexture(GL_TEXTURE_2D, ID);
	}
};
