#pragma once
#include "Scene.hpp"
#include <memory>
#include <map>
#include <vector>
#include "DefaultComponents.hpp"

namespace weapon {

    class WeaponEffect {
    public:
        WeaponEffect(glm::vec2& pos, float angle) : position(pos), angle(angle){}
        virtual ~WeaponEffect() = default;
        virtual void execute(Scene& scene) = 0;
    //protected:
        glm::vec2 position;
        float angle;
    };

    class WeaponEffectFactory {
    public:
        virtual std::unique_ptr<WeaponEffect> produce() = 0;
    };

    class Weapon {
    public:
        virtual ~Weapon() = default;
        virtual std::vector<std::unique_ptr<WeaponEffect>> shoot() = 0;
    //protected:
        glm::vec2 position;
        float angle;
    };

    class WeaponFactory {
    public:
        virtual std::unique_ptr<Weapon> produce() = 0;
    };

    std::map<size_t, std::unique_ptr<WeaponEffectFactory>> weaponEffectFactories;
    std::map<size_t, std::unique_ptr<WeaponFactory>> weaponFactories;

    void addWeaponFactory(size_t idx, std::unique_ptr<WeaponFactory> factory) {
        weaponFactories[idx] = std::move(factory);
    }

    void addWeaponEffectFactory(size_t idx, std::unique_ptr<WeaponEffectFactory> factory) {
        weaponEffectFactories[idx] = std::move(factory);
    }

    std::unique_ptr<Weapon> createWeapon(size_t id) {
        return weaponFactories[id]->produce();
    }

    std::unique_ptr<WeaponEffect> createEffect(size_t id) {
        return weaponEffectFactories[id]->produce();
    }

} // namespace weapon

struct WeaponEffectComponent {
    std::unique_ptr<weapon::WeaponEffect> effect;
};

struct WeaponComponent {
    std::unique_ptr<weapon::Weapon> weapon;
};

struct WeaponHolderComponent {
    std::vector<entt::registry::entity_type> weapons;
};

struct WeaponInputComponent {
    bool* shooting;
};
