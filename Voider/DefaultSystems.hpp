#pragma once
#include "System.hpp"
#include "DefaultComponents.hpp"
#include "Camera.hpp"
#include "InstanceRenderer.hpp"


#include <box2d/box2d.h>
#include <cmath>

class CharacterController {
public:
	CharacterController(b2Body* body)
		: m_body(body), m_maxSpeed(30.0f), m_acceleration(20.0f), m_turnSpeed(15.0f) {}

	void Update(float deltaTime, const b2Vec2 lookInput, const b2Vec2 movementInput) {
		// Ensure the body is not null
		if (!m_body) return;

		// Handle rotation
		RotateTowards(lookInput, deltaTime);

		// Handle movement
		Move(movementInput, deltaTime);
	}

private:
	b2Body* m_body;
	float m_maxSpeed;
	float m_acceleration;
	float m_turnSpeed;

	void RotateTowards(const b2Vec2& target, float deltaTime) {
		if (target.LengthSquared() == 0) return; // Avoid division by zero

		// Calculate desired angle
		float desiredAngle = std::atan2(target.y, target.x);

		// Get current angle
		float currentAngle = m_body->GetAngle();

		// Calculate the shortest direction to rotate
		float angleDifference = desiredAngle - currentAngle;
		while (angleDifference < -b2_pi) angleDifference += 2 * b2_pi;
		while (angleDifference > b2_pi) angleDifference -= 2 * b2_pi;

		// Calculate and apply angular velocity to smoothly rotate towards the target
		float angularVelocity = angleDifference * m_turnSpeed;
		m_body->SetAngularVelocity(angularVelocity);
	}

	void Move(const b2Vec2& direction, float deltaTime) {
		//if (direction.LengthSquared() == 0) return; // No movement input

		// Normalize the direction
		b2Vec2 normalizedDirection = direction;
		normalizedDirection.Normalize();

		// Calculate desired velocity
		b2Vec2 desiredVelocity = normalizedDirection;
		desiredVelocity*=m_maxSpeed;

		// Calculate the change in velocity required to achieve desired velocity
		b2Vec2 velocityChange = desiredVelocity - m_body->GetLinearVelocity();
		b2Vec2 impulse = m_body->GetMass() * velocityChange; // Impulse = mass * velocityChange

		impulse *= deltaTime * m_acceleration;

		// Apply a scaled impulse based on acceleration and deltaTime to smoothly change velocity
		m_body->ApplyLinearImpulseToCenter(impulse, true);
	}
};


class InputSystem : public System<const MovementInputComponent, BodyComponent>
{
public:
	InputSystem(Scene& scene, entt::registry& registry, glm::vec2& movementInput, glm::vec2& lookInput) :
		movementInput(movementInput), lookInput(lookInput), System(scene, registry) {}
	virtual void Process(registryView view)
	{
		for (auto entity : view) {
			glm::vec2 movInputSum{ 0.f, 0.f };
			glm::vec2 lookInputSum{ 0.f, 0.f };
			auto clientInput = registry.try_get<ClientInputComponent>(entity);
			auto randomInput = registry.try_get<RandomInputComponent>(entity);
			if (clientInput)
			{
				movInputSum += movementInput;
				lookInputSum += lookInput;
			}
			if (randomInput)
			{
				movInputSum += glm::vec2{random.next<float>(-1.f,1.f), random.next<float>(-1.f,1.f) };
				lookInputSum += glm::vec2{ random.next<float>(-10.f,10.f), random.next<float>(-10.f,10.f) };
			}

			float timestep = 1.f / 60.f;

			auto& body = view.get<BodyComponent>(entity);

			CharacterController controller(body.body);

			movInputSum *= body.body->GetInertia();

			controller.Update(timestep, b2Vec2(lookInputSum.x, lookInputSum.y), b2Vec2(movInputSum.x, movInputSum.y));

			//movInputSum;// *= body.body->GetInertia();
			//body.body->ApplyLinearImpulseToCenter(b2Vec2(movInputSum.x, movInputSum.y), true);
			//float desiredAngle = glm::pi<float>() + glm::orientedAngle(glm::vec2(1.f, 0.f), lookInputSum);
			//float currentAngle = body.body->GetAngle();

			//// Normalize angles to range [-pi, pi]
			//desiredAngle = fmod(desiredAngle + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
			//currentAngle = fmod(currentAngle + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
			//float timeStep = 1.0f / 60.0f;
			//body.body->SetTransform(body.body->GetPosition(), desiredAngle);
			//body.body->SetAngularVelocity((desiredAngle - currentAngle));

		}
	}
private:
	glm::vec2& movementInput;
	glm::vec2& lookInput;
	SVRandom random;
};

class PhysicsSystem : public System<BodyComponent>
{
public:
	PhysicsSystem(Scene& scene, entt::registry& registry, b2World& world) :
		world(world), System(scene, registry) {}
	virtual void Process(registryView view)
	{
		float timeStep = 1.0f / 60.0f;
		int32 velocityIterations = 6;
		int32 positionIterations = 6;
		world.Step(timeStep, velocityIterations, positionIterations);
	}
private:
	b2World& world;
};

class RenderingSystem : public System<const BodyComponent, DrawableComponent>
{
public:
	RenderingSystem(Scene& scene, entt::registry& registry, InstanceRenderer& renderer, Camera& camera) :
		renderer(renderer), camera(camera), System(scene, registry){}
	virtual void Process(registryView view)
	{
		for (auto entity : view) {
			const auto& body = view.get<BodyComponent>(entity);
			auto& drawable = view.get<DrawableComponent>(entity);
			auto& inst = renderer.accessInstance(drawable.instanceId);
			inst.rotation = body.body->GetTransform().q.GetAngle();
			const b2Vec2 pos = body.body->GetTransform().p;
			inst.pos = glm::vec3(pos.x, pos.y, 0.f);
		}
	}
private:
	InstanceRenderer& renderer;
	Camera& camera;
};