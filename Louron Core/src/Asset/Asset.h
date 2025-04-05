#pragma once

// Louron Core Headers
#include "../Core/UUID.h"

// C++ Standard Library Headers
#include <string>
#include <filesystem>
#include <memory>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace Louron {

	using AssetHandle = UUID;

	enum class AssetType : uint8_t {

		Unknown = 0,

		Scene,
		Prefab,

		Texture2D,
		TextureCubeMap,

		Mesh,
		ModelImport,

		Skeleton,
		AnimationClip,

		Audio,

		Material_Standard,
		Material_Skybox,

		Compute_Shader,
		Shader

	};

	namespace AssetUtils {

		std::string AssetTypeToString(AssetType type);
		AssetType AssetTypeFromString(const std::string& assetType);
	}


	class Asset : public std::enable_shared_from_this<Asset> {

	public:
		AssetHandle Handle;

		virtual bool operator==(const Asset& other) const { return Handle == other.Handle; }
		virtual AssetType GetType() const = 0;
	};

	struct AssetImportConfig 
	{ 
		//TODO: bool GenerateThumbnails	= true;		// Generate a thumbnail for preview in the editor.
		//TODO: bool OptimizeForRuntime	= true;		// Optimise asset data for faster loading/rendering.
		//TODO: bool PreserveSourceData	= false;	// Keep a copy of the raw source file data.
		bool AutoReimport				= true;		// Automatically reimport when the source file changes.
	};

	struct ModelImportConfig : public AssetImportConfig
	{
		glm::vec3 ImportPosition = glm::vec3(0.0f);	// Import with overriden position for root entity.
		glm::vec3 ImportRotation = glm::vec3(0.0f);	// Import with overriden rotation for root entity.
		glm::vec3 ImportScale	 = glm::vec3(1.0f);	// Import with overriden scale for root entity.

		bool ImportSkeleton				= false;	// Import Skeleton Bone Structure.
		bool ImportAnimations			= false;	// Import Animations.
		bool ImportMaterials			= true;		// Import embedded materials.

		//TODO: bool MergeMeshes		= false;	// Combine multiple meshes of same material into a single mesh.
		//TODO: bool GenerateLODs		= false;	// Generate Levels of Detail (LODs).
		//TODO: bool FlipUVs			= false;	// Flip UV coordinates (useful for certain model formats).
	};

	struct AssetMetaData {

		/// <summary>
		/// The Asset Type.
		/// </summary>
		AssetType Type = AssetType::Unknown;

		/// <summary>
		/// Configuration of Asset Import
		/// </summary>
		std::shared_ptr<AssetImportConfig> ImportConfig = nullptr;

		/// <summary>
		/// The Relative file path of the Asset to the Project Asset Directory.
		/// </summary>
		std::filesystem::path FilePath = "";

		/// <summary>
		/// The name of the Asset.
		/// </summary>
		std::string AssetName = "";

		/// <summary>
		/// If the Asset is a child to a composite asset, e.g., this could be a material embedded in a Model File such as FBX.
		/// </summary>
		AssetHandle ParentAssetHandle = NULL_UUID;

		/// <summary>
		/// If the Asset has children dependencies, e.g., materials, textures, animations, etc.
		/// </summary>
		bool IsComposite = false;

		/// <summary>
		/// If the Asset is a Custom Asset that is added to the registry at runtime.
		/// </summary>
		bool IsCustomAsset = false;

		operator bool() const { return Type != AssetType::Unknown; }
	};

}