#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>

#include "Engine Callbacks.h"

#include "Defines/LAssets.h"
#include "Defines/LBounds.h"
#include "Defines/LMath.h"
#include "Defines/LTypes.h"
#include "Defines/LVectors.h"
#include "Defines/LMatrices.h"

#include "ECS/LEntity.h"
#include "ECS/LComponent.h"

#include "Engine Util/LDebug.h"
#include "Engine Util/LInput.h"
#include "Engine Util/LTime.h"

namespace Louron
{
    enum class ColliderType : uint8_t
    {
        Unknown = 0,
        Box_Collider,
        Sphere_Collider,
        Capsule_Collider,
        Mesh_Collider
    };

    /**
	* @brief Structure that contains information about a collision.
    * 
	* This is passed to the OnCollide and OnTrigger function's when a collision occurs.
    */
    struct Collider
    {
        /**
         * @brief The type of collider of the other entity.
         *
         * This can be Box, Sphere, Capsule, etc.
         */
        ColliderType type;

        /**
         * @brief The Entity that this collision event was had with.
         *
         * Use this to access components or tags from the other object involved in the collision.
         */
        Entity entity;
    };

	/**
	* @brief Interface for all scripts.
    * 
	* Every script must inherit from this interface, in order to be recognised by the engine.
    * 
	* Each script has a range of default functions the engine will invoke at different points in the game loop.
    * 
	* Each script MUST be a class, and must inherit from this interface for this to work. If you use a struct , it will not work.
    * 
    * @code
    * 
    * // Will Not Work
	* struct MyScript : public IScript {};
    * 
    * // Will Work
	* class MyScript : public IScript {};
    * 
    * @endcode
    */
    class IScript : public Louron::Entity
    {

    public:

        /**
        * @brief Function that is called as the script is instantiated.
        *
        * When the script is made active on a ScriptComponent, or when an entity is instantiated with an active script, this function is called.
        *
        * @code
        * void OnCreate() override
        * {
        *     Debug::Log("Script created!");
        * }
        * @endcode
        */
        virtual void OnCreate() {};

        /**
        * @brief Function that is called when the script is destroyed.
        *
        * When the script is destroyed, or when the entity it is attached to is destroyed, this function is called.
        *
        * @code
        * void OnDestroy() override
        * {
        *     Debug::Log("Script destroyed!");
        * }
        * @endcode
        */
        virtual void OnDestroy() {};

        /**
        * @brief Function that is called every frame.
        *
        * This is the main function that is used for most of the script logic.
        *
        * This is called once per frame during the update phase.
        *
        * @code
        * void OnUpdate() override
        * {
		*     // Can just call GetPosition() and SetPosition() directly as the IScript inherits from Entity.
		*     SetPosition(GetPosition() + Vector3(0.0f, 1.0f, 0.0f) * Time::GetDeltaTime());
        * }
        * @endcode
        */
        virtual void OnUpdate() {};

        /**
        * @brief Function that is called every frame, after the update phase.
        *
        * Once all OnUpdates have been called, this function is called on all scripts.
        *
        * This is useful for scenarios where you need to do something after all other scripts have updated, for instance update the transform of a camera that is tracking an Entity.
        *
        * @code
		* void CustomEntityFollow(Entity& target)
        * {
		*     // ... Custom look at logic per requirements, maybe using a smooth interpolation, or just direct lookat.
        * }
        * 
        * void OnLateUpdate() override
        * {
        *     CustomEntityFollow(Entity::FindByName("Player"));
        * }
        * @endcode
        */
        virtual void OnLateUpdate() {};

        /**
		* @brief Function that is called every fixed frame.
        * 
		* This is called every fixed tick of game engine. The engine has its main update pass which is called once per frame, however, for systems like physics, you need a fixed update. 
        *
		* This is set to 60hz per default, but can be changed using Time::SetFixedDeltaTime(float fixed_delta_time).
        * 
		* Use FixedUpdate when using RigidbodyComponent. Set a force to a RigidbodyComponent and it applies each fixed frame. If however, you applied a force every frame, it would be inconsistent and not work as intended. 
        * 
        * Imagine having a computer that only could run at 30 frames per second, you would apply 30 forces per second, but if another computer were to run the game at 60 frames per second, it would apply 60 forces per second. This would be inconsistent and not work as intended.
        * 
		* Having a fixed step basically means that you can apply a force every fixed frame, and it will be consistent across all computers, e.g., for a computer running at 30 frames per second, it would call the fixed update twice per frame, to ensure the consistency, and for a computer running at 60 frames per second, it would call the fixed update once per frame. To go even further, if you had a computer running at 120 frames per second, it would call the fixed update every other frame.
        * 
        * @code
        * void OnFixedUpdate() override
        * {
        *     if (Input::GetKey(KeyCode::W))
        *         GetComponent<RigidbodyComponent>.AddForce(this->GetFront() * 10.0f);
        * }
        * @endcode
        */
        virtual void OnFixedUpdate() {};

        /**
		* @brief Function that is called when a Collider hits any collider attached to this Entity.
        * 
		* When this Entity collides with another, a collision event is triggered to any scripts on both entities that have a OnCollideEnter function.
        * 
		* @param other Information about the collision include the type of Collider, and the Entity collided with.
        * 
        * @code
		* uint8_t player_health = 100;
		* void OnCollideEnter(const Collider& other) override
        * {
		*     if (other.entity.GetTag().find("Bullet") != std::string::npos)
        *     {
		*         player_health -= 1;
        *     }
        * 
		*     if(other.collider_type == ColliderType::Box_Collider)
        *     {
		*         Debug::Log("Collided with a Box Collider");
        *     }
        * }
		* @endcode
        */
        virtual void OnCollideEnter(const Louron::Collider& other) {};

        /**
		* @brief Function that is called when a Collider is still colliding with any collider attached to this Entity.
        * 
		* When this Entity is still colliding with another, a collision event is triggered to any scripts on both entities that have a OnCollideStay function.
        * 
		* @param other Information about the collision include the type of Collider, and the Entity collided with.
        * 
        * @code
        * struct BabyMamma
        * {
        *     float time_spent_sexing = 0.0f;
        *     std::vector<Entity> babies;
        * 
        *     void MakeABaby(const Prefab& prefab, const Transform& transform)
        *     {
        *         babies.push_back(Entity::Instantiate(prefab, transform)); // The special magic happens here
        *     }
        * };
        * 
        * // Key = BabyMamma, Value = The Sexing and Babies :3
        * std::unordered_map<Entity, BabyMamma> baby_mammas;
        * [[ExposedInEditor]] Prefab baby_prefab; // Set in editor
        * float time_required_to_make_a_baby = 5.0f;
        * 
        * void OnCollideStay(const Collider& other) override
        * {
        *     if (other.entity.GetTag().find("Mamma") != std::string::npos) // If she a mamma
        *     {
        *         BabyMamma& baby_mamma = baby_mammas[other.entity];
        *         if (baby_mamma.time_spent_sexing < time_required_to_make_a_baby)
        *         {
        *             baby_mamma.time_spent_sexing += Time::GetFixedDeltaTime(); // Fixed because Collision functions are called in FixedUpdate stage of engine.
        *         }
        *         else
        *         {
        *             baby_mamma.time_spent_sexing = 0.0f;
        *             baby_mamma.MakeABaby(baby_prefab, other.entity.GetTransform());
        *         }
        *     }
        * }
		* @endcode
        */
        virtual void OnCollideStay(const Louron::Collider& other) {};

        /**
        * @brief Function that is called when a Collider leaves any collider that had collided with this Entity.
        *
        * When the collider leaves the other collider, a collision event is triggered to any scripts on both entities that have an OnCollideLeave() function.
        *
        * @param other Information about the collision include the type of Collider, and the Entity collided with.
        *
        * @code
        * uint8_t player_health = 100;
        * void OnCollideLeave(const Collider& other) override
        * {
        *     if (other.entity.GetTag().find("SafeZone") != std::string::npos)
        *     {
        *         player_health -= 1;
        *         Debug::Log("Left the Safe Zone, health decreased by 1");
        *     }
        * }
        * @endcode
        */
        virtual void OnCollideLeave(const Louron::Collider& other) {};

        /**
        * @brief Function that is called when a Trigger collider enters any trigger attached to this Entity.
        *
        * Trigger colliders do not physically block movement but instead raise trigger events.
        *
        * @param other Information about the trigger, including the type and the entity that entered.
        *
        * @code
        * void OnTriggerEnter(const Collider& other) override
        * {
        *     if (other.entity.GetTag() == "Player")
        *     {
        *         Debug::Log("Player entered the trigger zone.");
        *     }
        * }
        * @endcode
        */
        virtual void OnTriggerEnter(const Louron::Collider& other) {};

        /**
        * @brief Function that is called when a Trigger collider stays inside any trigger attached to this Entity.
        *
        * Trigger colliders do not physically block movement but instead raise trigger events.
        *
        * @param other Information about the trigger, including the type and the entity that is still inside.
        *
        * @code
        * void OnTriggerStay(const Collider& other) override
        * {
        *     if (other.entity.GetTag() == "Player")
        *     {
        *         Debug::Log("Player is still inside the trigger.");
        *     }
        * }
        * @endcode
        */
        virtual void OnTriggerStay(const Louron::Collider& other) {};

        /**
        * @brief Function that is called when a Trigger collider leaves any trigger attached to this Entity.
        *
        * Trigger colliders do not physically block movement but instead raise trigger events.
        *
        * @param other Information about the trigger, including the type and the entity that exited.
        *
        * @code
        * void OnTriggerLeave(const Collider& other) override
        * {
        *     if (other.entity.GetTag() == "Player")
        *     {
        *         Debug::Log("Player exited the trigger zone.");
        *     }
        * }
        * @endcode
        */
        virtual void OnTriggerLeave(const Louron::Collider& other) {};

        const uint32_t& GetEntityID() const { return m_EntityID; }

    };
}

namespace BackEndAPI
{
    // --- DATA STRUCTURES ---
    struct FieldInfo
    {
        const char* name = "";
        size_t offset = 0;
        BackEndAPI::FieldType type = BackEndAPI::FieldType::Unknown;
    };

    struct ScriptTypeInfo
    {
        const char* name;
        int field_count;
        const FieldInfo* fields;
        void* (*factory_function)(uint32_t);
        void(*release_function)(uint32_t);
    };
}

// --- EXPORTED FUNCTIONS FROM DLL ---
SCRIPT_API void LoadScripts();

SCRIPT_API const BackEndAPI::ScriptTypeInfo* GetScriptTypes(size_t* count);
SCRIPT_API const char** GetScriptNames(size_t* count);

inline std::unordered_map<std::string, std::unique_ptr<Louron::IScript>> s_ScriptInstanceMap;