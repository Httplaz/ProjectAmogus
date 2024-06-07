#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::mat4 projection = glm::mat4(1.f);
	float scale1 = 1.0f;

	// Сеттеры и методы преобразования
	void setScale(float newScale) {
		scale1 = newScale;
	}

	void scale(float scaleFactor) {
		scale1 *= scaleFactor;
	}

	void setProjection(glm::mat4 proj)
	{
		projection = proj;
	}

	glm::mat4 getProjection() const
	{
		return glm::mat4(1.0f);// projection;
	}


	void setTranslation(const glm::vec2& newPosition) {
		position = newPosition;
	}

	void translate(const glm::vec2& displacement) {
		position += displacement;
	}

	glm::mat4 getMatrix() const {
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
		glm::mat4 scaling = glm::scale(glm::mat4(1.0f), glm::vec3(scale1, scale1, 1.0f));
		return projection * scaling * translation;
	}
};