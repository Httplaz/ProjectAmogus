#pragma once
#include <entt.hpp>
#include "DefaultComponents.hpp"

class SystemIface {
public:
    virtual ~SystemIface() = default;
    virtual void Run() = 0;
};

class Scene;

// Template system class that inherits from ISystem
template<typename... Profile>
class System : public SystemIface {
protected:
    static entt::registry dummy_registry;
    using registryView = decltype(dummy_registry.template view<Profile...>());

public:
    System(Scene& scene, entt::registry& registry) : scene(scene), registry(registry) {}

    // Override the pure virtual function from ISystem
    void Run() override {
        auto view = registry.view<Profile...>();
        Process(view);
    }

    // Pure virtual function to be implemented by derived classes
    virtual void Process(registryView view) = 0;

protected:
    entt::registry& registry;
    Scene& scene;
};