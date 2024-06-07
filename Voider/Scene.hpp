#pragma once
#include "InstanceRenderer.hpp"
#include "SVRandom.hpp"
#include <entt.hpp>
#include <box2d/box2d.h>
#include "DefaultSystems.hpp"
#include "World.hpp"
#include "WorldView.hpp"

class Scene
{
public:
	Scene(InstanceRenderer& renderer, Camera& camera) : physWorld(b2Vec2{ 0.f, 0.f }), 
		camera(camera), renderer(renderer)
	{
		tileWorld.st();
		for (auto i = 0; i < WorldParams::gpuChunkMapSide; i++)
			for (auto j = 0; j < WorldParams::gpuChunkMapSide; j++)
				tileWorld.loadChunk({ j,i });
	}
	
	struct EntityWrp
	{
		Scene& scene;
		entt::entity entity;

		template<typename Component, typename... Args>
		EntityWrp& add(Args&&... args)
		{
			scene.getRegistry().emplace<Component>(entity, std::forward<Args>(args)...);
			return *this;
		}

		template<>
		EntityWrp& add<DrawableComponent, uint16_t>(uint16_t&& arg) {
			if (arg != static_cast<uint16_t>( - 1))
			{
				scene.getRegistry().emplace<DrawableComponent>(entity, std::forward<uint16_t>(arg));
			}
			else
			{
				auto instanceId = scene.getRenderer().addInstance(0, {1.f});
				scene.getRegistry().emplace<DrawableComponent>(entity, instanceId);
			}
			return *this;
		}
	};

	EntityWrp createEntity() {

		entt::entity entity = registry.create();
		return { *this, entity };
	}

	template<typename SysT, typename... Args>
	SysT& addSystem(Args&&... args) {
		auto system = std::make_unique<SysT>(*this, registry, std::forward<Args>(args)...);
		systems.push_back(std::move(system));
		return static_cast<SysT&>(*systems.back());
	}

	entt::registry& getRegistry()
	{
		return registry;
	}

	void update()
	{
		auto cameraTargetView = registry.view<const BodyComponent, CameraTargetComponent>();
		for (auto entity : cameraTargetView) {
			const auto& body = cameraTargetView.get<const BodyComponent>(entity);
			camera.setTranslation({ body.body->GetTransform().p.x,body.body->GetTransform().p.y });
		}

		for (auto& system: systems)
		{
			system->Run();
		}
	}

	b2World& getPhysWorld()
	{
		return physWorld;
	}

	SvWorld& getTileWorld()
	{
		return tileWorld;
	}

	InstanceRenderer& getRenderer()
	{
		return renderer;
	}

	~Scene()
	{

	}
private:
	b2World physWorld;
	SvWorld tileWorld;
	Camera& camera;
	InstanceRenderer& renderer;
	entt::registry registry;
	std::vector<std::unique_ptr<SystemIface>> systems;
};