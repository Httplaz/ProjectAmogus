#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>


class Shader {
public:
	unsigned int ID;
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;

		vShaderFile.open(vertexPath);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		vShaderFile.close();
		vertexCode = removeGLSLComments(vShaderStream.str());
		//std::cout << vertexCode;

		fShaderFile.open(fragmentPath);
		fShaderStream << fShaderFile.rdbuf();
		fShaderFile.close();
		fragmentCode = removeGLSLComments(fShaderStream.str());
		//std::cout << "\n" << fragmentCode;
		unsigned int vertex, fragment;

		vertex = compileShader(GL_VERTEX_SHADER, vertexCode.c_str());
		fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

		unsigned int geometry;
		if (geometryPath != nullptr) {
			gShaderFile.open(geometryPath);
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
			geometry = compileShader(GL_GEOMETRY_SHADER, geometryCode.c_str());
		}

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(ID, geometry);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);
	}

	void use() {
		glUseProgram(ID);
	}

	void setBool(const std::string& name, bool value) const {
		glProgramUniform1i(ID, glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const std::string& name, int value) const {
		glProgramUniform1i(ID, glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const {
		glProgramUniform1f(ID, glGetUniformLocation(ID, name.c_str()), value);
	}

	void setVec2(const std::string& name, const glm::vec2& value) const {
		glProgramUniform2fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setIvec2(const std::string& name, const glm::ivec2& value) const {
		glProgramUniform2iv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const {
		glProgramUniform3fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setVec4(const std::string& name, const glm::vec4& value) const {
		glProgramUniform4fv(ID, glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setMat4(const std::string& name, const glm::mat4& mat) const {
		glProgramUniformMatrix4fv(ID, glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
	}


	// Другие типы униформ можно добавить по аналогии

private:

	std::string removeGLSLComments(const std::string& shaderCode) {
		std::regex commentPattern("//.*?\n");
		return std::regex_replace(shaderCode, commentPattern, "");
	}

	unsigned int compileShader(GLenum type, const char* source) {
		unsigned int shader;
		shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		checkCompileErrors(shader, "SHADER");
		return shader;
	}

	void checkCompileErrors(unsigned int shader, std::string type) {
		int success;
		char infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};