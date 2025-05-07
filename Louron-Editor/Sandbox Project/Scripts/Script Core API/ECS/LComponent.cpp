#include "LComponent.h"

#include "LEntity.h"
#include "../ScriptAPI.h"

namespace Louron
{
	namespace Components
	{

		IScript* ScriptComponent::GetScriptInstance(const std::string& script_name) const
		{
			auto found_instance = s_ScriptInstanceMap.find(std::to_string(m_EntityID) + script_name);
			if (found_instance != s_ScriptInstanceMap.end())
				return found_instance->second.get();

			return nullptr;
		}

		std::vector<LODMeshComponent::LODElement> LODMeshComponent::GetLODElements() const
		{
			std::vector<LODElement> elements;

			size_t element_count = ENGINE_SAFE_CALL_RET(size_t, size_t(*)(uint32_t), LODMeshComponent_GetElementCount, m_EntityID);

			if (element_count == 0)
				return elements;

			elements.reserve(element_count);

			float* distances = new float[element_count];
			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, float*, size_t), LODMeshComponent_GetDistances, m_EntityID, distances, element_count);

			for (size_t i = 0; i < element_count; ++i)
			{
				LODElement element;
				element.DistanceThresholdNormalised = distances[i];

				size_t lod_entity_count = ENGINE_SAFE_CALL_RET(size_t, size_t(*)(uint32_t, size_t), LODMeshComponent_GetEntityCount, m_EntityID, i);
				if (lod_entity_count == 0)
				{
					elements.emplace_back(std::move(element));
					continue;
				}
				element.MeshRendererEntities.reserve(lod_entity_count);
				uint32_t* lod_entity_array = new uint32_t[lod_entity_count];
				std::fill(lod_entity_array, lod_entity_array + lod_entity_count, NULL_UUID);

				ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t*, size_t, size_t), LODMeshComponent_GetEntityArray, m_EntityID, lod_entity_array, lod_entity_count, i);

				for (size_t j = 0; j < lod_entity_count; ++j)
				{
					element.MeshRendererEntities.emplace_back(Entity(lod_entity_array[j]));
				}
				delete[] lod_entity_array;

				elements.emplace_back(std::move(element));
			}
			delete[] distances;

			return elements;
		}

		void LODMeshComponent::SetLODElements(const std::vector<LODElement>& lod_elements) const
		{
			size_t element_count = lod_elements.size();

			float* distances = new float[element_count];
			size_t* entity_counts = new size_t[element_count];

			size_t total_entity_count = 0;
			for (size_t i = 0; i < element_count; ++i)
			{
				distances[i] = lod_elements[i].DistanceThresholdNormalised;
				entity_counts[i] = lod_elements[i].MeshRendererEntities.size();
				total_entity_count += entity_counts[i];
			}

			uint32_t* entity_array = new uint32_t[total_entity_count];

			size_t offset = 0;
			for (size_t i = 0; i < element_count; ++i)
			{
				const auto& entities = lod_elements[i].MeshRendererEntities;
				for (size_t j = 0; j < entities.size(); ++j)
				{
					entity_array[offset++] = static_cast<uint32_t>(entities[j]); // Assuming Entity -> uint32_t
				}
			}

			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, const size_t*, const uint32_t*, size_t),
				LODMeshComponent_SetLODElements,
				m_EntityID,
				distances,
				entity_counts,
				entity_array,
				element_count);

			delete[] distances;
			delete[] entity_counts;
			delete[] entity_array;
		}

		std::unordered_map<uint32_t, Entity> SkinnedMeshComponent::GetBoneMapping() const
		{
			std::unordered_map<uint32_t, Entity> bone_map{};

			size_t bone_count = 0;
			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, size_t*), SkinnedMeshComponent_GetBoneMapCount, m_EntityID, &bone_count);

			if (bone_count == 0)
				return bone_map;

			bone_map.reserve(bone_count);

			uint32_t* key_array = new uint32_t[bone_count];
			uint32_t* entity_array = new uint32_t[bone_count];

			std::fill(key_array, key_array + bone_count, NULL_UUID);
			std::fill(entity_array, entity_array + bone_count, NULL_UUID);

			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t*, uint32_t*, size_t), SkinnedMeshComponent_GetBoneMapEntities, m_EntityID, key_array, entity_array, bone_count);

			for (size_t i = 0; i < bone_count; ++i)
				bone_map[key_array[i]] = Entity(entity_array[i]);

			return bone_map;
		}

		void SkinnedMeshComponent::SetBoneMapping(const std::unordered_map<uint32_t, Entity>& bone_map) const
		{
			size_t bone_count = bone_map.size();

			if (bone_count == 0)
				return;

			uint32_t* key_array = new uint32_t[bone_count];
			uint32_t* entity_array = new uint32_t[bone_count];

			std::fill(key_array, key_array + bone_count, NULL_UUID);
			std::fill(entity_array, entity_array + bone_count, NULL_UUID);

			size_t i = 0;
			for (const auto& pair : bone_map)
			{
				if (i >= bone_count)
					break;

				key_array[i] = pair.first;
				entity_array[i] = pair.second.m_EntityID;
				++i;
			}

			ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t*, const uint32_t*, size_t),
				SkinnedMeshComponent_SetBoneMapEntities,
				m_EntityID,
				key_array,
				entity_array,
				bone_count);

			delete[] key_array;
			delete[] entity_array;
		}

	}

}
