// Основные библиотеки
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <entt.hpp>
#include <Box2D/Box2D.h>

#include <glm/gtx/vector_angle.hpp>
// Вспомогательные библиотеки
#include <iostream>
#include <thread>
#include <chrono>

#include "SVRandom.hpp"

#include "Shader.hpp"
#include "WorldView.hpp"
#include "InstanceRenderer.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "DefaultSystems.hpp"

#include "Weapon.hpp"

#include "Scene.hpp"
#include "Weapons.hpp"

Camera camera;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
glm::vec3 clientInput{ 0.f };
glm::vec2 cursorPos{ 0.f };

glm::vec2 movementInput{ 0.f };
glm::vec2 lookInput{ 0.f };

bool shooting = false;

bool firstMouse;
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		cursorPos.x = xpos;
		cursorPos.y = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - cursorPos.x;
	float yoffset = cursorPos.y - ypos; // reversed since y-coordinates go from bottom to top

	cursorPos.x = xpos;
	cursorPos.y = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	const float zoomSpeed = 0.04f; // скорость масштабирования

	camera.scale(1.0f + zoomSpeed * yoffset);
}

namespace stats
{
	float averageFps = 0.f;
	float actualFps = 0.f;
	uint32_t entities = 0;
}

static int scrWidth, scrHeight;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	scrWidth = width;
	scrHeight = height;
	glViewport(0, 0, width, height);
	float aspectRatio = static_cast<float>(width) / height;
	float left = -aspectRatio;
	float right = aspectRatio;
	float bottom = -1.0f;
	float top = 1.0f;
	float near = -1.0f;
	float far = 1.0f;

	// Создаем ортогональную матрицу проекции
	glm::mat4 projection = glm::ortho(left, right, bottom, top, near, far);

	camera.setProjection(projection);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
			shooting = true;
		else if (action == GLFW_RELEASE)
			shooting = false;
	}
}

// Обработчик ошибок GLFW
void glfw_error_callback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << std::endl;
}

// Инициализация GLFW и GLAD
GLFWwindow* init() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return nullptr;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Создаем окно
	scrWidth = 800;
	scrHeight = 600;
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Mint Rectangle", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	// Инициализируем GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}
	glfwSwapInterval(1); // Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	io.FontGlobalScale = 2.0f;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 410");

	return window;
}

void renderGui()
{
	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("STATS", nullptr);
	ImGui::Text((std::string("fps: ") + std::to_string(stats::averageFps)).c_str());
	ImGui::Text((std::string("entities: ") + std::to_string(stats::entities)).c_str());
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern float ta;

// Главная функция
int main() {
	GLFWwindow* window = init();
	if (!window) return -1;

	Shader shader("default.vert", "default.frag");

	SVRandom random;

	//Rectangle rectangle;
	InstanceRenderer renderer(1, 6, 8192);
	auto meshIndex = renderer.addMesh({
		// positions   // texCoords
		InstanceRenderer::Vertex{glm::vec4{ 0.5f,  0.5f,  1.0f, 1.0f}}, // top right
		InstanceRenderer::Vertex{glm::vec4{0.5f, -0.5f,  1.0f, 0.0f} }, // bottom right
		InstanceRenderer::Vertex{glm::vec4{-0.5f,  0.5f,  0.0f, 1.0f }}, // bottom left
		InstanceRenderer::Vertex{glm::vec4{ -0.5f, -0.5f,  0.0f, 0.0f}}  // top left
		});

	WorldGenerator::init();

	WorldGenerator::setImpl(std::unique_ptr<ColorsGenerator>(new ColorsGenerator));


	Scene scene(renderer, camera);

	int playerEntity = random.next<int>(1024);

	for (int i = 0; i < 1024; i++) {
		auto instanceId = renderer.addInstance(meshIndex, { random.next<float>(0.5f, 2.f) });

		auto& instance = renderer.accessInstance(instanceId);

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(random.next<float>(100.f), random.next<float>(100.f));

		auto body = scene.getPhysWorld().CreateBody(&bodyDef);

		b2PolygonShape box;
		box.SetAsBox(instance.scale / 2.0f, instance.scale / 2.0f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &box;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.9f;
		body->CreateFixture(&fixtureDef);

		auto entity = scene.createEntity().add<DrawableComponent>(instanceId).
			add<BodyComponent>(body).add<MovementInputComponent>();
		if (i == playerEntity)
		{
			auto weaponEntity = scene.createEntity().add<WeaponComponent>(std::make_unique<Mashinegun>());
			entity.add<WeaponHolderComponent>(std::vector<entt::entity>{ weaponEntity.entity });
			entity.add<CameraTargetComponent>().add<ClientInputComponent>().add<WeaponInputComponent>().
				add<WeaponInputComponent>(&shooting);
		}
		else
		{
			entity.add<RandomInputComponent>();
		}
	}
	scene.addSystem<RenderingSystem>(renderer, camera);
	scene.addSystem<PhysicsSystem>(scene.getPhysWorld());
	scene.addSystem<InputSystem>(movementInput, lookInput);
	scene.addSystem<WeaponSystem>();
	WorldView worldView(scene.getTileWorld());

	Chunk a;
	std::fill(a.tiles, a.tiles + WorldParams::chunkSize * WorldParams::chunkSize, uint32_t(-1));
	worldView.setDefaultChunk(a);

	double lastTime = glfwGetTime(), currentTime;
	double targetFPS = 60.f;
	double targetFrameTime = 1.0 / targetFPS;

	while (!glfwWindowShouldClose(window)) {
		const float cameraSpeed = 0.05f / camera.scale1;

		movementInput = { clientInput.x, clientInput.y };

		float maxDimension = std::max(scrWidth, scrHeight);
		glm::vec2 cursorPosLocal = glm::normalize((cursorPos / glm::vec2(scrWidth, scrHeight) - glm::vec2(0.5f)) * maxDimension / glm::vec2(scrHeight, scrWidth));
		lookInput = cursorPosLocal * glm::vec2(1.f, -1.f);

		glClearColor(0.6f, 1.0f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		worldView.setViewMatrix(glm::inverse(glm::translate(camera.getMatrix(), glm::vec3(glm::vec2(2.), 0.))));


		//worldView.update();
		//worldView.render();
		//pool.render();

	
		scene.update();
		stats::entities = scene.getRegistry().storage<entt::entity>().size();
		renderer.shader.use();
		renderer.shader.setMat4("cameraMatrix", camera.getMatrix());
		renderer.shader.setMat4("projectionMatrix", camera.getProjection());
		renderer.Draw();

		renderGui();

		glfwSwapBuffers(window);
		glfwPollEvents();

		const float zoomSpeed = 0.04f;

		camera.scale(1.0f + zoomSpeed * clientInput.z);

		currentTime = glfwGetTime();
		double frameTime = currentTime - lastTime;
		while (frameTime < targetFrameTime) {
			currentTime = glfwGetTime();
			frameTime = currentTime - lastTime;
		}
		stats::averageFps = 1.f / frameTime;
		lastTime = glfwGetTime();
	}

	glfwTerminate();
	return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << origin1.z << "\n\n\n\n\n";
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_W)
			clientInput.y++;
		if (key == GLFW_KEY_A)
			clientInput.x--;
		if (key == GLFW_KEY_S)
			clientInput.y--;
		if (key == GLFW_KEY_D)
			clientInput.x++;
		if (key == GLFW_KEY_Q)
			clientInput.z++;
		if (key == GLFW_KEY_E)
			clientInput.z--;
	}

	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_W)
			clientInput.y--;
		if (key == GLFW_KEY_A)
			clientInput.x++;
		if (key == GLFW_KEY_S)
			clientInput.y++;
		if (key == GLFW_KEY_D)
			clientInput.x--;
		if (key == GLFW_KEY_Q)
			clientInput.z--;
		if (key == GLFW_KEY_E)
			clientInput.z++;
	}
}
