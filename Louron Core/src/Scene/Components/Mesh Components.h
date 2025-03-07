#pragma once

// Louron Core Headers
#include "Component Base.h"

#include "../Spatial Partitioning/Bounds.h"
#include "../../Asset/Asset.h"
#include "../../OpenGL/Material.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers

namespace Louron
{

    struct MeshFilterComponent : public ComponentBase 
    {
        AssetHandle StaticMeshHandle = NULL_UUID;

        Bounds_AABB TransformedAABB{};
        bool AABBNeedsUpdate = true;
        bool OctreeNeedsUpdate = true;

        void UpdateTransformedAABB();

        MeshFilterComponent() = default;
        ~MeshFilterComponent() = default;

        MeshFilterComponent(const MeshFilterComponent& other) = default;
        MeshFilterComponent(MeshFilterComponent&& other) noexcept = default;

        MeshFilterComponent& operator=(const MeshFilterComponent& other) = default;
        MeshFilterComponent& operator=(MeshFilterComponent&& other) = default;

        void Serialize(YAML::Emitter& out) const;
        bool Deserialize(const YAML::Node data);

        void SetShouldDisplayDebugLines(const bool& shouldDisplay) { m_DisplayDebugAABB = shouldDisplay; }
        bool GetShouldDisplayDebugLines() const { return m_DisplayDebugAABB; }

    private:

        bool m_DisplayDebugAABB = false;

    };

    class MaterialUniformBlock;
    struct MeshRendererComponent : public ComponentBase 
    {
        bool Active = true;
        std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> MaterialHandles;

        bool CastShadows = false;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);
    };

    struct LODMeshComponent : public ComponentBase
    {
        struct LODElement
        {
            /// <summary>
            /// Percentage of Distance from Camera or Max Distance E.g., 0 = near plane, 1 = far plane, if this is set to .5, this will pop to the next LOD when the distance is in the middle of the view frustum
            /// </summary>
            float DistanceThresholdNormalised;

            /// <summary>
            /// Vector of Entity UUID w/ MeshRenderers
            /// </summary>
            std::vector<Louron::UUID> MeshRendererEntities;
        };

        LODMeshComponent() = default;
        LODMeshComponent(const LODMeshComponent&) = default;
        LODMeshComponent(LODMeshComponent&&) = default;
        LODMeshComponent& operator=(const LODMeshComponent& other) = default;
        LODMeshComponent& operator=(LODMeshComponent&& other) = default;

        void Serialize(YAML::Emitter& out);
        bool Deserialize(const YAML::Node data);

        /// <summary>
        /// Use a custom max distance instead of the far plane.
        /// </summary>
        bool MaxDistanceOverFarPlane = false;

        /// <summary>
        /// Custom Max Distance if using over far plane of camera.
        /// </summary>
        float MaxDistance = 500.0f;

        std::vector<LODElement> LOD_Elements = {
            LODElement{ 0.25f, {} },
            LODElement{ 0.50f, {} },
            LODElement{ 0.75f, {} }
        };
    };

}