#include "CollisionCallback.h"

// Louron Core Headers
#include "../Scene/Scene.h"
#include "../Scene/Entity.h"

#include "../Scripting/Script Manager.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	void CollisionCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) 
	{
		if (auto scene_ref = m_Scene.lock(); scene_ref && scene_ref->IsRunning()) 
		{
			for (PxU32 i = 0; i < nbPairs; i++) 
			{
				const PxContactPair& cp = pairs[i];

				// Retrieve the entities associated with the shapes
				if (!cp.shapes[0]->userData || !cp.shapes[1]->userData) {
					L_CORE_ERROR("Collision Callback - Collider Shapes Have Invalid UserData.");
					continue;
				}
						
				std::array<Entity, 2> entities{
					scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[0]->userData))),
					scene_ref->FindEntityByUUID(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cp.shapes[1]->userData)))
				};

				if (entities[0] && entities[1]) 
				{
					for (int i = 0; i < entities.size(); i++) {
						int index_this = (i == 0) ? 0 : 1;
						int index_other = (i == 0) ? 1 : 0;

						if (entities[i].HasComponent<ScriptComponent>()) 
						{
							auto& script_component = entities[i].GetComponent<ScriptComponent>();
							for (const auto& [script_name, script_active] : script_component.Scripts)
							{
								// Validate Script Instance
								if (!script_active)
								{
									continue;
								}
								else if (!ScriptManager::Get()->GetScriptClassInstance(entities[i].GetUUID(), script_name))
								{
									// Script Active in Component, but Script Instance Not Created Yet
									ScriptManager::Get()->OnCreateScript(entities[i].GetUUID(), script_name);
								}

								if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
								{
									ScriptManager::Get()->OnCollideScript(entities[index_this].GetUUID(), entities[index_other].GetUUID(), script_name, Script_Collision_Type::CollideEnter);
								}
								if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) 
								{
									ScriptManager::Get()->OnCollideScript(entities[index_this].GetUUID(), entities[index_other].GetUUID(), script_name, Script_Collision_Type::CollideStay);
								}
								if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) 
								{
									ScriptManager::Get()->OnCollideScript(entities[index_this].GetUUID(), entities[index_other].GetUUID(), script_name, Script_Collision_Type::CollideLeave);
								}
							}
						}
					}
				}
				else 
				{
					L_CORE_ERROR("Collision Callback - Collision Detected With Entity Not Found In Scene.");
				}
				
			}
		}
	}

	void CollisionCallback::onTrigger(PxTriggerPair* pairs, PxU32 count) {

		if (auto scene_ref = m_Scene.lock(); scene_ref && scene_ref->IsRunning()) 
		{
			for (PxU32 i = 0; i < count; i++) 
			{
				const PxTriggerPair& tp = pairs[i];

				// Retrieve the entities associated with the trigger and actor shapes
				if (!tp.triggerShape->userData || !tp.otherShape->userData) 
				{
					L_CORE_ERROR("Trigger Callback - Collider Shapes Have Invalid UserData.");
					continue;
				}

				uint32_t trigger_uuid = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tp.triggerShape->userData));
				uint32_t other_uuid = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tp.otherShape->userData));

				Entity trigger_entity = scene_ref->FindEntityByUUID(trigger_uuid);
				Entity other_entity = scene_ref->FindEntityByUUID(other_uuid);

				if (trigger_entity && other_entity) 
				{
					std::pair<uint32_t, uint32_t> trigger_pair = { trigger_uuid, other_uuid };

					if (trigger_entity.HasComponent<ScriptComponent>()) 
					{
						auto& script_component = trigger_entity.GetComponent<ScriptComponent>();
						for (const auto& [script_name, script_active] : script_component.Scripts)
						{
							// Validate Script Instance
							if (!script_active)
							{
								continue;
							}
							else if (!ScriptManager::Get()->GetScriptClassInstance(trigger_uuid, script_name))
							{
								// Script Active in Component, but Script Instance Not Created Yet
								ScriptManager::Get()->OnCreateScript(trigger_uuid, script_name);
							}

							if (tp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
							{
								ScriptManager::Get()->OnCollideScript(trigger_uuid, other_uuid, script_name, Script_Collision_Type::TriggerEnter);
								s_ActiveTriggers.insert(trigger_pair);
							}

							if (tp.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
							{
								ScriptManager::Get()->OnCollideScript(trigger_uuid, other_uuid, script_name, Script_Collision_Type::TriggerLeave);
								s_ActiveTriggers.erase(trigger_pair);
							}
						}
					}
				}
				else 
				{
					L_CORE_ERROR("Collision Callback - Collision Detected With Entity Not Found In Scene.");
				}

			}
		}
	}

	void CollisionCallback::onConstraintBreak(PxConstraintInfo*, PxU32)
	{
	}

	void CollisionCallback::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
	{
	}

	void CollisionCallback::onWake(PxActor**, PxU32)
	{
	}

	void CollisionCallback::onSleep(PxActor**, PxU32)
	{
	}

}