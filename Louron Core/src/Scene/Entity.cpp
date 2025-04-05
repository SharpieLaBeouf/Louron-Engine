#include "Entity.h"

// Louron Core Headers
#include "Scene.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

    Entity::Entity(entt::entity regHandle, Scene* scene) :
        m_EntityHandle(regHandle), 
        m_Scene(scene)
    {
    }

    void Entity::InitAllEntityScripts()
    {
		if (!m_Scene)
			return;

		// Validate and Update
		auto view = m_Scene->m_Registry.view<ScriptComponent>();
		for (auto entity_handle : view)
		{
			auto& script_component = view.get<ScriptComponent>(entity_handle);
			for (const auto& [script_name, script_active] : script_component.Scripts)
			{
				// Validate Script Instance
				if (script_active)
				{
					ScriptManager::Get()->OnCreateScript(GetUUID(), script_name);
				}
			}
		}

    }
}