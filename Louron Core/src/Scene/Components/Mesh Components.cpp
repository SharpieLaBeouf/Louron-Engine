#include "Mesh Components.h"

// Louron Core Headers
#include "../Entity.h"
#include "../Spatial Partitioning/Bounds.h"
#include "../../OpenGL/Mesh.h"

#include "Core Components.h"

// C++ Standard Library Headers
#include <iomanip>

// External Vendor Library Headers

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron
{

#pragma region Mesh Renderer Component

	void MeshRendererComponent::Serialize(YAML::Emitter& out) {
		out << YAML::Key << "MeshRendererComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshActive" << YAML::Value << Active;
		out << YAML::Key << "CastShadows" << YAML::Value << CastShadows;

		{
			out << YAML::Key << "MaterialAssetCount" << YAML::Value << (uint32_t)MeshRendererMaterialHandles.size();
			out << YAML::Key << "MaterialAssetHandles" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, uniform_block] : MeshRendererMaterialHandles) {
				out << (uint32_t)handle;
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
	}

	bool MeshRendererComponent::Deserialize(const YAML::Node data)
	{
		YAML::Node component = data;

		if (component["MeshActive"]) {
			Active = component["MeshActive"].as<bool>();
		}

		if (component["CastShadows"]) {
			CastShadows = component["CastShadows"].as<bool>();
		}

		if (component["MaterialAssetCount"] && component["MaterialAssetHandles"]) {
			uint32_t count = component["MaterialAssetCount"].as<uint32_t>();
			YAML::Node handles = component["MaterialAssetHandles"];

			if (handles.size() != count) {
				return false;
			}

			MeshRendererMaterialHandles.clear();
			for (const auto& handle : handles) {
				MeshRendererMaterialHandles.push_back({ handle.as<uint32_t>() , nullptr });
			}
		}
		else {
			return false;
		}

		return true;
	}

#pragma endregion 

#pragma region Mesh Filter Component

	void MeshFilterComponent::UpdateTransformedAABB() {

		if (!GetEntity()) {
			L_CORE_ERROR("Cannot UpdateTransformedAABB - Current Entity Is Invalid!");
			return;
		}

		Entity entity = *GetEntity();

		if (!entity || !entity.GetScene())
			return;

		if (MeshFilterAssetHandle == NULL_UUID)
			return;

		if (auto mesh_asset = AssetManager::GetAsset<StaticMesh>(MeshFilterAssetHandle); mesh_asset) {

			// Define the 8 corner points of the AABB
			std::array<glm::vec3, 8> corners = {
				mesh_asset->MeshBounds.BoundsMin,
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMin.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMin.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMax.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z),
				glm::vec3(mesh_asset->MeshBounds.BoundsMin.x, mesh_asset->MeshBounds.BoundsMax.y, mesh_asset->MeshBounds.BoundsMax.z)
			};

			// Transform the corner points
			glm::mat4 global_transform = GetComponent<TransformComponent>().GetGlobalTransform();
			for (int i = 0; i < 8; ++i) {
				glm::vec4 transformed_corner = global_transform * glm::vec4(corners[i], 1.0f);
				corners[i] = glm::vec3(transformed_corner);
			}

			// Find the new BoundsMin and BoundsMax
			glm::vec3 newMin = corners[0];
			glm::vec3 newMax = corners[0];
			for (int i = 1; i < 8; ++i) {
				newMin = glm::min(newMin, corners[i]);
				newMax = glm::max(newMax, corners[i]);
			}
			TransformedAABB.BoundsMin = newMin;
			TransformedAABB.BoundsMax = newMax;

			AABBNeedsUpdate = false;
		}
	}

	void MeshFilterComponent::Serialize(YAML::Emitter& out) const {

		out << YAML::Key << "MeshFilterComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "MeshAssetHandle" << YAML::Value << (uint32_t)MeshFilterAssetHandle;

		out << YAML::EndMap;
	}

	bool MeshFilterComponent::Deserialize(const YAML::Node data) {

		YAML::Node component = data;

		if (component["MeshAssetHandle"]) {
			MeshFilterAssetHandle = component["MeshAssetHandle"].as<uint32_t>();
		}
		else {
			return false;
		}

		return true;
	}

#pragma endregion 

#pragma region LOD Component

    void LODMeshComponent::Serialize(YAML::Emitter& out)
    {
        out << YAML::Key << "LODMeshComponent";
        out << YAML::BeginMap;

        if (LOD_Elements.empty())
        {
            out << YAML::EndMap;
            return;
        }

        out << YAML::Key << "MaxDistanceOverFarPlane" << MaxDistanceOverFarPlane;
        out << YAML::Key << "MaxDistance" << MaxDistance;

        out << YAML::Key << "LOD Elements" << YAML::BeginMap;
        for (int i = 0; i < LOD_Elements.size(); i++)
        {
            out << YAML::Key << "Element " + std::to_string(i) << YAML::Value;
            out << YAML::BeginMap;
            {
                out << YAML::Key << "Distance Threshold" << YAML::Value << LOD_Elements[i].DistanceThresholdNormalised;

                out << YAML::Key << "Entities" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& entity_uuid : LOD_Elements[i].MeshRendererEntities) {
                    out << (uint32_t)entity_uuid;
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        out << YAML::EndMap;
    }

    bool LODMeshComponent::Deserialize(const YAML::Node data)
    {
        if (data["MaxDistanceOverFarPlane"])
            MaxDistanceOverFarPlane = data["MaxDistanceOverFarPlane"].as<bool>();

        if (data["MaxDistance"])
            MaxDistance = data["MaxDistance"].as<float>();

        if (YAML::Node elements = data["LOD Elements"]; elements && elements.size() > 0)
        {
            LOD_Elements.clear();

            for (auto it = elements.begin(); it != elements.end();)
            {
                LOD_Elements.push_back({});

                LOD_Elements.back().DistanceThresholdNormalised = it->second["Distance Threshold"].as<float>();

                auto entitiesSeq = it->second["Entities"];
                if (entitiesSeq.IsSequence() && entitiesSeq.size() > 0) {

                    for (int i = 0; i < entitiesSeq.size(); i++)
                    {
                        LOD_Elements.back().MeshRendererEntities.push_back(entitiesSeq[i].as<uint32_t>());
                    }
                }

                ++it;
            }
        }

        return true;
    }

#pragma endregion 

}