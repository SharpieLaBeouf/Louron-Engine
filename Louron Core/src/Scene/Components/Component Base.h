#pragma once

// Louron Core Headers
#include "UUID.h"
#include "../Entity.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers

namespace Louron 
{
    struct ComponentBase 
    {

    public:

        virtual ~ComponentBase() = default;

        const Entity& GetEntity() const { return m_Entity; }
        void SetEntity(const Entity& entity) { m_Entity = entity; }

        template<typename T>
        T& GetComponentInParent() { return (m_Entity) ? m_Entity.GetComponentInParent<T>() : Entity::GetBlankComponent<T>(); }

        template<typename T>
        T& GetComponentInChild() { return (m_Entity) ? m_Entity.GetComponentInChild<T>() : Entity::GetBlankComponent<T>(); }

        template<typename T>
        std::vector<Entity> GetComponentsInParents() { return (m_Entity) ? m_Entity.GetComponentsInParents<T>() : {}; }

        template<typename T>
        std::vector<Entity> GetComponentsInChildren() { return (m_Entity) ? m_Entity.GetComponentsInChildren<T>() : {}; }

    private:

        Entity m_Entity{};

    };
}