#pragma once
#include <entt.hpp>
#include <glm/glm.hpp>
#include <Box2D/Box2D.h>

struct NonEmptyComponent
{
	size_t filler; //msvc optimisations break entt when the component struct is empty
};

struct DrawableComponent
{
	uint16_t instanceId;
};

struct BodyComponent
{
	b2Body* body;
};

struct CameraTargetComponent : public NonEmptyComponent
{
};

struct MovementInputComponent : public NonEmptyComponent
{
};

struct RandomInputComponent : public NonEmptyComponent
{
};

struct ClientInputComponent : public NonEmptyComponent
{
};