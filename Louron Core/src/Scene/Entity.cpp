#include "Entity.h"

// Louron Core Headers

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

    Entity::Entity(entt::entity regHandle, Scene* scene) :
        m_EntityHandle(regHandle), 
        m_Scene(scene)
    {
    }
}