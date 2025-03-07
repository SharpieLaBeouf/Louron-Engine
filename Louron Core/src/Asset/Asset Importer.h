#pragma once

// Louron Core Headers
#include "Asset.h"
#include "Asset Manager.h"

#include "../OpenGL/Shader.h"
#include "../OpenGL/Compute Shader Asset.h"
#include "../OpenGL/Material.h"
#include "../Scene/Components/Skybox Component.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <entt/entt.hpp>

namespace Louron {

	class Scene;
	class Prefab;
	class Texture2D;

	using AssetMap = std::map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetaData>;

	class AssetImporter {
	
	public:

		struct ImportParams
		{
			AssetMap* asset_map;
			AssetRegistry* asset_reg;
			const AssetHandle& asset_handle;
			const AssetMetaData& asset_meta_data;
			const std::filesystem::path& project_asset_directory;
		};

		static std::shared_ptr<Asset> ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory);

	};

	class SceneImporter {

	public:

		static std::shared_ptr<Scene> ImportScene(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<Scene> LoadScene(const AssetImporter::ImportParams& import_params, const std::filesystem::path& scene_file_path);

	};

	class PrefabImporter {

	public:

		static std::shared_ptr<Prefab> ImportPrefab(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<Prefab> LoadPrefab(const std::filesystem::path& path);

	};

	class TextureImporter {

	public:

		static std::shared_ptr<Texture2D> ImportTexture2D(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<Texture2D> LoadTexture2D(const std::filesystem::path& path);
	};

	struct BoneLayout;

	namespace AssimpHelpers { struct MeshInstanceKey; }
	class ModelImporter {

	public:

		static std::shared_ptr<Prefab> ImportModel(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<Prefab> LoadModel(const AssetImporter::ImportParams& import_params, const std::filesystem::path& model_file_path);

	private:

		static bool ImportSkeleton;
		static bool ImportAnimations;
		static bool ImportMaterials;
		static AssetHandle ImportedSkeletonHandle;

		// --- General ---

		static void ProcessMesh(const aiScene* ai_scene, aiMesh* ai_mesh, std::shared_ptr<StaticMesh> asset_mesh, BoneLayout* skeleton = nullptr);
		static void ProcessMaterial(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, const aiMesh* ai_mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<StaticMesh> asset_mesh, const std::filesystem::path& model_file_path);

		// --- Static Mesh ---

		static void ProcessStaticMeshNode(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, aiNode* ai_node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, const std::filesystem::path& model_file_path);

		// --- Skinned Mesh ---

		static entt::entity ProcessSkeleton(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, const aiNode* ai_node, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path);
		static void ProcessSkinnedMeshNode(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path);

		// -- Animation --

		static void ProcessAnimations(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path);

	};

	class AudioImporter {

		//static std::shared_ptr<Audio> ImportAudio(AssetHandle handle, const AssetMetaData& meta_data);
		//static std::shared_ptr<Audio> LoadAudio(const std::filesystem::path& path);


	};

	class MaterialImporter {

	public:

		static std::shared_ptr<Material> ImportMaterial(const AssetImporter::ImportParams& import_params);

		static std::shared_ptr<Material> LoadMaterialPBR(const std::filesystem::path& path);
		static std::shared_ptr<SkyboxMaterial> LoadMaterialSkybox(const std::filesystem::path& path);

	};

	class ShaderImporter
	{

	public:

		static std::shared_ptr<Shader> ImportShader(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<Shader> LoadShader(const std::filesystem::path& path);

		static std::shared_ptr<ComputeShaderAsset> ImportComputeShader(const AssetImporter::ImportParams& import_params);
		static std::shared_ptr<ComputeShaderAsset> LoadComputeShader(const std::filesystem::path& path);

	};

}