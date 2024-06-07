#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class RectangleMesh {
public:
	RectangleMesh(float x, float y, float width, float height) {
		float vertices[] = {
			x, y, 0.0f, // left bottom corner
			x + width, y, 0.0f, // right bottom corner
			x + width, y + height, 0.0f, // right top corner
			x, y + height, 0.0f // left top corner
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(VAO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	~RectangleMesh() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void draw() {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);
	}

private:
	unsigned int VAO, VBO;
};
