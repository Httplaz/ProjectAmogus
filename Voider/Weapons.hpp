#include "Weapon.hpp"

class CreateBulletEffect : public weapon::WeaponEffect
{
public:
	CreateBulletEffect(glm::vec2& pos, float angle, float force, float offset) :
		weapon::WeaponEffect(pos, angle), force(force), offset(offset)
	{

	}
	virtual void execute(Scene& scene) override
	{
		SVRandom random;
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(position.x, position.y);

		auto body = scene.getPhysWorld().CreateBody(&bodyDef);

		b2CircleShape shape;
		shape.m_radius = 0.5f;

		//b2PolygonShape box;
		//box.SetAsBox(0.5f, 0.5f);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 5.0f;
		fixtureDef.friction = 0.1f;

		fixtureDef.filter.categoryBits = 0;
		fixtureDef.filter.maskBits = 0;

		body->CreateFixture(&fixtureDef);


		body->SetTransform(body->GetPosition(), angle);

		b2Rot rot(body->GetAngle());
		b2Vec2 dir(-1.f, 0.f);

		dir = b2Mul(rot, dir);

		auto posOffset = dir;
		posOffset *= offset;

		body->SetTransform(body->GetPosition() + posOffset, angle);

		dir *= force;
		body->ApplyLinearImpulseToCenter(dir, true);

		//std::cout << "bang!\n";

		auto bulletEntity = scene.createEntity().add<DrawableComponent>(static_cast<uint16_t>( - 1)).add<BodyComponent>(body);
	}
	float force;
	float offset;
};

class Mashinegun : public weapon::Weapon
{
public:
	virtual std::vector<std::unique_ptr<weapon::WeaponEffect>> shoot() override
	{
		std::vector<std::unique_ptr<weapon::WeaponEffect>> a;
		if (framesActive % 1 == 0) {
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
			a.push_back(std::make_unique<CreateBulletEffect>(position, angle, 300.f, 1.f));
		}
		framesActive++;
		return a;
	}
protected:
	uint32_t framesActive = 0;
};

class WeaponSystem : public System<BodyComponent, WeaponHolderComponent>
{
public:
	WeaponSystem(Scene& scene, entt::registry& registry) :
		System(scene, registry) {}
	virtual void Process(registryView view)
	{
		for (auto entity : view) {
			auto input = registry.try_get<WeaponInputComponent>(entity);
			const auto& body = view.get<BodyComponent>(entity);
			auto& holder = view.get<WeaponHolderComponent>(entity);
			for (auto weaponEntity : holder.weapons)
			{
				auto weapon1 = registry.try_get<WeaponComponent>(weaponEntity);
				auto& weapon = registry.get<WeaponComponent>(weaponEntity);
				weapon.weapon->position = glm::vec2(body.body->GetPosition().x, body.body->GetPosition().y);
				weapon.weapon->angle = body.body->GetAngle() - glm::pi<float>();
				if (input)
				{
					if(*(input->shooting))
					{
						std::vector<std::unique_ptr<weapon::WeaponEffect>> effects(weapon.weapon->shoot());
						for (auto& effect:effects)
						{
							effect->position = glm::vec2(body.body->GetPosition().x, body.body->GetPosition().y);
							//std::cout << "bang!\n";
							effect->execute(scene);
						}
					}
				}
			}
		}
	}
};