#include "Asset Importer.h"

// Louron Core Headers
#include "Asset Manager API.h"

#include "../Core/FileSystem Utilities.h"

#include "../Debug/Profiler.h"
#include "../Project/Project.h"

#include "../Scene/Scene.h"
#include "../Scene/Prefab.h"

#include "../Scene/Components/Animator Component.h"
#include "../Scene/Components/SkinnedMeshComponent.h"

#include "../Animation/Animations.h"
#include "../Animation/Skeleton.h"
#include "../Animation/Humanoid.h"
#include "../Animation/Animation State Machine.h"
#include "../OpenGL/Mesh.h"

// C++ Standard Library Headers
#include <map>
#include <memory>
#include <functional>

// External Vendor Library Headers
#include <glm/gtx/string_cast.hpp>

#include <yaml-cpp/yaml.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron 
{

#pragma region Asset Import

	//						const AssetImporter::ImportParams& import_params
	using AssetImportFunction = std::function<std::shared_ptr<Asset>(const AssetImporter::ImportParams&)>;
	
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {

		//{ AssetType::Audio, AudioImporter::ImportAudio },
		{ AssetType::Prefab,					PrefabImporter::ImportPrefab },
		{ AssetType::Scene,						SceneImporter::ImportScene },

		{ AssetType::Texture2D,					TextureImporter::ImportTexture2D },

		{ AssetType::Material_Standard,			MaterialImporter::ImportMaterial },
		{ AssetType::Material_Skybox,			MaterialImporter::ImportMaterial },

		{ AssetType::ModelImport,				ModelImporter::ImportModel },

		{ AssetType::Shader,					ShaderImporter::ImportShader },
		{ AssetType::Compute_Shader,			ShaderImporter::ImportComputeShader },
		
		{ AssetType::AnimationStateMachine,		AnimationStateMachineImporter::ImportStateMachine },

		{ AssetType::Humanoid,					HumanoidImporter::ImportHumanoid },
		{ AssetType::HumanoidMask,				HumanoidImporter::ImportHumanoidMask }
	};

	std::shared_ptr<Asset> AssetImporter::ImportAsset(AssetMap* asset_map, AssetRegistry* asset_reg, AssetHandle handle, const AssetMetaData& metadata, const std::filesystem::path& project_asset_directory)
	{
		if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
		{
			L_CORE_ERROR("No importer available for asset type: {0}", AssetUtils::AssetTypeToString(metadata.Type));
			return nullptr;
		}

		ImportParams parameters{ asset_map, asset_reg, handle, metadata, project_asset_directory };

		return s_AssetImportFunctions.at(metadata.Type)(parameters);
	}

#pragma endregion

#pragma region Scene Import


	std::shared_ptr<Scene> SceneImporter::ImportScene(const AssetImporter::ImportParams& import_params) 
	{
		auto project = Project::GetActiveProject();
		return LoadScene(import_params, project->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	std::shared_ptr<Scene> SceneImporter::LoadScene(const AssetImporter::ImportParams& import_params, const std::filesystem::path& scene_file_path)
	{

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneSerializer serializer(scene);
		serializer.Deserialize(scene_file_path);
		return scene;
	}

#pragma endregion

#pragma region Prefab File Import

	std::shared_ptr<Prefab> PrefabImporter::ImportPrefab(const AssetImporter::ImportParams& import_params)
	{
		return LoadPrefab(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	std::shared_ptr<Prefab> PrefabImporter::LoadPrefab(const std::filesystem::path& path)
	{
		std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>();
		prefab->SetMutable(false);
		if (prefab->Deserialize(path)) 
		{
			return prefab;
		}

		return nullptr;
	}

#pragma endregion

#pragma region Texture2D Import

	std::shared_ptr<Texture2D> TextureImporter::ImportTexture2D(const AssetImporter::ImportParams& import_params) 
	{
		return LoadTexture2D(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	std::shared_ptr<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& path) {
		return std::make_shared<Texture2D>(path);
	}


#pragma endregion

#pragma region Material Import

	std::shared_ptr<Material> MaterialImporter::ImportMaterial(const AssetImporter::ImportParams& import_params) {

		if (import_params.asset_meta_data.FilePath.extension() != ".lmat" && import_params.asset_meta_data.FilePath.extension() != ".lmaterial" && import_params.asset_meta_data.FilePath.extension() != ".lskybox") 
		{
			L_CORE_WARN("Incompatible Material File Extension");
			L_CORE_WARN("Extension Used: {0}", import_params.asset_meta_data.FilePath.extension().string());

			return nullptr;
		}

		YAML::Node data;

		auto project = Project::GetActiveProject();
		std::filesystem::path file_path = project->GetAssetDirectory() / import_params.asset_meta_data.FilePath;

		if (!std::filesystem::exists(file_path))
			return nullptr;

		try 
		{
			data = YAML::LoadFile(file_path.string());
		}
		catch (YAML::ParserException e) 
		{
			L_CORE_ERROR("YAML-CPP Failed to Load Scene File: '{0}', {1}", import_params.asset_meta_data.FilePath.string(), e.what());
			return nullptr;
		}

		if (!data["Material Asset Type"]) 
		{
			L_CORE_ERROR("Material Type Node is Not Specified in File: '{0}'", import_params.asset_meta_data.FilePath.string());
			return nullptr;
		}

		if (data["Material Asset Type"].as<std::string>() == AssetUtils::AssetTypeToString(AssetType::Material_Skybox)) 
		{
			return LoadMaterialSkybox(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
		}

		if (data["Material Asset Type"].as<std::string>() == AssetUtils::AssetTypeToString(AssetType::Material_Standard)) 
		{
			return LoadMaterialPBR(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
		}

		return nullptr;
	}

	std::shared_ptr<Material> MaterialImporter::LoadMaterialPBR(const std::filesystem::path& path) {

		std::shared_ptr<Material> material = std::make_shared<Material>();

		if (material->Deserialize(path))
			return material;

		return nullptr;
	}

	std::shared_ptr<SkyboxMaterial> MaterialImporter::LoadMaterialSkybox(const std::filesystem::path& path) {

		std::shared_ptr<SkyboxMaterial> material = std::make_shared<SkyboxMaterial>();

		if (material->Deserialize(path))
			return material;
		
		return nullptr;
	}

#pragma endregion

#pragma region Model Import

	namespace AssimpHelpers
	{
		static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
		{
			glm::mat4 to{};
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}
		static inline glm::vec3 GetGLMVec(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
		static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation) { return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z); }

		// Used to compare nodes within an aiScene to identify Linked Duplicates
		struct MeshInstanceKey
		{
			std::vector<uint32_t> mesh_references;
			bool operator==(const MeshInstanceKey& other) const { return mesh_references == other.mesh_references; }
			void normalize() { std::sort(mesh_references.begin(), mesh_references.end()); }

			void GenerateKey(const aiNode* ai_node)
			{
				mesh_references.reserve(ai_node->mNumMeshes);
				for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
					mesh_references.emplace_back(ai_node->mMeshes[i]);
				normalize();
			}
		};

		// Used to compare nodes within an aiScene to identify Linked Duplicates
		struct MeshInstanceKeyHash
		{
			std::size_t operator()(const MeshInstanceKey& node) const
			{
				std::size_t seed = node.mesh_references.size();
				for (uint32_t mesh : node.mesh_references) seed ^= std::hash<uint32_t>{}(mesh)+0x9e3779b9 + (seed << 6) + (seed >> 2);
				return seed;
			}
		};

		// Used to compare nodes within an aiScene to identify Linked Duplicates
		static std::unordered_map<MeshInstanceKey,
			std::pair<AssetHandle, // AssetMesh Reference
			std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>>>, // Material Vector
			MeshInstanceKeyHash> s_LoadedNodes = {};

		static std::filesystem::path ResolveAssimpTexturePath(const std::filesystem::path& asset_file_path, const aiString& assimp_texture_string)
		{
			std::filesystem::path absolute_texture_path = assimp_texture_string.C_Str(); // Assume is absolute

			if (absolute_texture_path.is_relative()) // Check if relative
			{
				absolute_texture_path = std::filesystem::absolute(asset_file_path.parent_path() / assimp_texture_string.C_Str());
			}
			else if (absolute_texture_path.filename() == absolute_texture_path) // Check if path stripped and is just in the same directory as model file
			{
				absolute_texture_path = std::filesystem::absolute(asset_file_path.parent_path() / absolute_texture_path);
			}
			else // Just format it to an absolute path if neither of the above work
			{
				absolute_texture_path = std::filesystem::absolute(absolute_texture_path);
			}

			return Utils::NormalisePath(absolute_texture_path);
		}

	}

	std::shared_ptr<Prefab> ModelImporter::ImportModel(const AssetImporter::ImportParams& import_params)
	{
		return LoadModel(import_params, import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	bool ModelImporter::ImportSkeleton = false;
	bool ModelImporter::ImportAnimations = false;
	bool ModelImporter::ImportMaterials = true;
	AssetHandle ModelImporter::ImportedSkeletonHandle = NULL_UUID;

	std::shared_ptr<Prefab> ModelImporter::LoadModel(const AssetImporter::ImportParams& import_params, const std::filesystem::path& model_file_path)
	{

		if (!import_params.asset_map)
		{
			L_CORE_ERROR("Cannot Import Model - Asset Map Invalid.");
			return nullptr;
		}

		if (!import_params.asset_reg)
		{
			L_CORE_ERROR("Cannot Import Model - Asset Registry Invalid.");
			return nullptr;
		}

		if (!std::filesystem::exists(model_file_path)) 
		{
			L_CORE_ERROR("Cannot Import Model - File Path Does Not Exist.");
			return nullptr;
		}

		std::string model_name = model_file_path.filename().replace_extension().string();


		static Assimp::Importer ai_importer = {};

		const aiScene* ai_scene = ai_importer.ReadFile(model_file_path.string(),

			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_SplitLargeMeshes |
			aiProcess_ImproveCacheLocality |
			aiProcess_JoinIdenticalVertices |
			aiProcess_RemoveRedundantMaterials |

			aiProcess_GenUVCoords |
			aiProcess_GenSmoothNormals |
			aiProcess_GenBoundingBoxes |

			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_PopulateArmatureData |

			aiProcess_FindInstances |
			aiProcess_FindDegenerates |
			aiProcess_FindInvalidData |
			0

		);

		if (!ai_scene) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Invalid: {0}", ai_importer.GetErrorString());
			ai_importer.FreeScene();
			return nullptr;
		}

		if (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Data Structure Incomplete. Potentially Corrupted File.");
			ai_importer.FreeScene();
			return nullptr;
		}

		if (!ai_scene->mRootNode) {
			L_CORE_ERROR("Cannot Import Model - Assimp Scene Has No Root Node");
			ai_importer.FreeScene();
			return nullptr;
		}

		ImportSkeleton = false;
		ImportAnimations = false;
		ImportMaterials = true;
		ImportedSkeletonHandle = NULL_UUID;

		if (import_params.asset_meta_data.ImportConfig)
		{
			auto import_config = static_pointer_cast<ModelImportConfig>(import_params.asset_meta_data.ImportConfig);
			if (import_config)
			{
				ImportSkeleton = import_config->ImportSkeleton;
				ImportAnimations = import_config->ImportAnimations;
				ImportMaterials = import_config->ImportMaterials;
			}
		}

		std::shared_ptr<Prefab> model_prefab = std::make_shared<Prefab>();
		model_prefab->SetMutable(false);

		if (ImportSkeleton)
		{
			auto& skinned_mesh_component = model_prefab->AddComponent<SkinnedMeshComponent>(model_prefab->GetRootEntity());
			entt::entity first_bone_entity = ProcessSkeleton(import_params, ai_scene, ai_scene->mRootNode, model_prefab, model_file_path);
			ProcessSkinnedMeshNode(import_params, ai_scene, model_prefab, model_file_path);

			skinned_mesh_component.ComputeFinalBoneTransformations(model_prefab);
		}
		else
		{
			ProcessStaticMeshNode(import_params, ai_scene, ai_scene->mRootNode, model_prefab, entt::null, model_file_path);
		}
		
		if (ImportAnimations)
		{
			ProcessAnimations(import_params, ai_scene, model_prefab, model_file_path);
		}

		AssimpHelpers::s_LoadedNodes.clear();
		ai_importer.FreeScene();

		return model_prefab;
	}

	void ModelImporter::ProcessMesh(const aiScene* ai_scene, aiMesh* ai_mesh, std::shared_ptr<StaticMesh> asset_mesh, BoneLayout* skeleton)
	{
		// 1. Process Vertices
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<glm::ivec4> bone_ids;
		std::vector<glm::vec4> bone_weights;

		// Extract Vertice Data
		for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {

			bone_ids.push_back({ -1, -1, -1, -1 });
			bone_weights.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });

			vertices.push_back(glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z));
			normals.push_back(glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z));

			if (ai_mesh->mTextureCoords[0]) texcoords.push_back(glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y));
			if (ai_mesh->mTangents) tangents.push_back(glm::vec3(ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z));
			if (ai_mesh->mBitangents) bitangents.push_back(glm::vec3(ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z));
		}

		// Extract Bone Data
		if (ImportSkeleton && skeleton)
		{
			for (unsigned int i = 0; i < ai_mesh->mNumBones; i++)
			{
				aiBone* ai_bone = ai_mesh->mBones[i];

				BoneLayout* skeleton_bone = skeleton->find(ai_bone->mName.C_Str());

				if (!skeleton_bone)
					continue;

				skeleton_bone->BoneOffsetMatrix = AssimpHelpers::ConvertMatrixToGLMFormat(ai_bone->mOffsetMatrix);

				// Loop throughy vertices affected by this bone
				for (unsigned int j = 0; j < ai_bone->mNumWeights; j++)
				{
					aiVertexWeight weight = ai_bone->mWeights[j];

					// Get index to vertex in vertices vector
					int vertex_id = weight.mVertexId;

					// Get weight of bone on vertice
					float vertex_weight = weight.mWeight;

					// Find the weakest bone influence
					int min_index = -1;
					float min_weight = 1.0f;
					for (int k = 0; k < 4; k++)
					{
						if (bone_ids[vertex_id][k] == -1)
						{
							// Prefer an unassigned slot
							min_index = k;
							break;
						}
						if (bone_weights[vertex_id][k] < min_weight)
						{
							min_weight = bone_weights[vertex_id][k];
							min_index = k;
						}
					}

					// If a weaker or empty slot is found, replace it
					if (min_index != -1)
					{
						bone_ids[vertex_id][min_index] = static_cast<int>(skeleton_bone->BoneID);
						bone_weights[vertex_id][min_index] = vertex_weight;
					}
				}
			}
		}

		std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();

		sub_mesh->SetVAO(std::make_unique<VertexArray>());

		// Separate VBOs so that vertex data is separate for runtime modification
		VertexBuffer* vbo_verts = new VertexBuffer(&vertices[0][0], (GLuint)vertices.size() * 3);
		vbo_verts->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aPos"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_verts);

		VertexBuffer* vbo_norms = new VertexBuffer(&normals[0][0], (GLuint)normals.size() * 3);
		vbo_norms->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aNormal"} });
		sub_mesh->GetVAO()->AddVertexBuffer(vbo_norms);

		if (!texcoords.empty()) {
			VertexBuffer* vbo_texcoords = new VertexBuffer(&texcoords[0][0], (GLuint)texcoords.size() * 2);
			vbo_texcoords->SetLayout(BufferLayout{ {ShaderDataType::Float2, "aTexCoord"} });
			sub_mesh->GetVAO()->AddVertexBuffer(vbo_texcoords);
		}
		if (!tangents.empty()) {
			VertexBuffer* vbo_tangents = new VertexBuffer(&tangents[0][0], (GLuint)tangents.size() * 3);
			vbo_tangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aTangent"} });
			sub_mesh->GetVAO()->AddVertexBuffer(vbo_tangents);
		}
		if (!bitangents.empty()) {
			VertexBuffer* vbo_bitangents = new VertexBuffer(&bitangents[0][0], (GLuint)bitangents.size() * 3);
			vbo_bitangents->SetLayout(BufferLayout{ {ShaderDataType::Float3, "aBitangent"} });
			sub_mesh->GetVAO()->AddVertexBuffer(vbo_bitangents);
		}
		if (!bone_ids.empty()) {
			VertexBuffer* vbo_bone_ids = new VertexBuffer(&bone_ids[0][0], (GLuint)bone_ids.size() * 4);
			vbo_bone_ids->SetLayout(BufferLayout{ {ShaderDataType::Int4, "aBoneIDs"} });
			sub_mesh->GetVAO()->AddVertexBuffer(vbo_bone_ids);
		}
		if (!bone_weights.empty()) {
			VertexBuffer* vbo_bone_weights = new VertexBuffer(&bone_weights[0][0], (GLuint)bone_weights.size() * 4);
			vbo_bone_weights->SetLayout(BufferLayout{ {ShaderDataType::Float4, "aBoneWeights"} });
			sub_mesh->GetVAO()->AddVertexBuffer(vbo_bone_weights);
		}

		// 2. Process Indices
		std::vector<GLuint> mesh_indices;
		for (unsigned int i = 0; i < ai_mesh->mNumFaces; i++) 
		{
			aiFace face = ai_mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				mesh_indices.push_back(face.mIndices[j]);
		}

		IndexBuffer* ebo = new IndexBuffer(mesh_indices, (GLuint)mesh_indices.size());
		sub_mesh->GetVAO()->SetIndexBuffer(ebo);

		// 3. Push SubMesh to Mesh Asset vector
		asset_mesh->SubMeshes.push_back(std::move(sub_mesh));
	}

	void ModelImporter::ProcessMaterial(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, const aiMesh* ai_mesh, std::shared_ptr<Prefab> model_prefab, entt::entity current_entity_handle, std::shared_ptr<StaticMesh> asset_mesh, const std::filesystem::path& model_file_path)
	{
		// 4. Create Material Asset
		aiMaterial* material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);

		auto project = Project::GetActiveProject();
		AssetHandle asset_material_handle;

		std::string material_suffix;
		if (materialName == aiString("DefaultMaterial") || !ImportMaterials)
		{
			material_suffix = "InBuiltAssetDefault_Material";  // Default Material
		}
		else
		{
			material_suffix = model_file_path.filename().string() + materialName.C_Str();  // Unique Material
		}

		// Generate the material handle
		asset_material_handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(AssetType::Material_Standard) + 
			material_suffix
		);

		// If the Material has not already been loaded into the asset map, we 
		// want to create a new material. Once this is done, we won't need to do this again 
		// for this material.
		if (import_params.asset_map->count(asset_material_handle) == 0)
		{
			AssetMetaData material_metadata;
			material_metadata.FilePath = std::filesystem::relative(model_file_path, Project::GetActiveProject()->GetAssetDirectory());
			material_metadata.Type = AssetType::Material_Standard;
			material_metadata.AssetName = materialName.C_Str();
			material_metadata.ParentAssetHandle = import_params.asset_handle;
			material_metadata.IsCustomAsset = import_params.asset_meta_data.IsCustomAsset;

			std::shared_ptr<Material> asset_material = std::make_shared<Material>();

			asset_material->SetName(materialName.C_Str());

			// Load Material Values

			ai_real temp{};
			aiColor4D colour{};

			if (material->Get(AI_MATKEY_BASE_COLOR, colour) == aiReturn_SUCCESS)
				asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });
			else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == aiReturn_SUCCESS)
				asset_material->SetAlbedoTintColour({ colour.r, colour.g, colour.b, colour.a });

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, temp) == aiReturn_SUCCESS)
				asset_material->SetMetallic(temp);

			if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, temp) == aiReturn_SUCCESS)
				asset_material->SetRoughness(temp);

			if (material->Get(AI_MATKEY_TRANSPARENCYFACTOR, temp) == aiReturn_SUCCESS && temp > 0.0f)
			{
				glm::vec4 temp_colour = asset_material->GetAlbedoTintColour();
				temp_colour.a = 1.0f - temp;
				asset_material->SetAlbedoTintColour(temp_colour);

				asset_material->SetRenderType(RenderType::L_MATERIAL_TRANSPARENT);
			}

			// Load Material Textures

			auto load_texture = [&](aiTextureType texture_type) -> void
				{
					if (texture_type == aiTextureType_NONE || material->GetTextureCount(texture_type) == 0)
						return;

					aiString assimp_texture_string;
					material->GetTexture(texture_type, 0, &assimp_texture_string);

					std::filesystem::path absolute_texture_path = AssimpHelpers::ResolveAssimpTexturePath(model_file_path, assimp_texture_string);

					AssetHandle texture_handle;
					AssetMetaData texture_meta_data;
					std::shared_ptr<Texture2D> texture_asset = nullptr;

					if (std::filesystem::exists(absolute_texture_path) && std::filesystem::is_regular_file(absolute_texture_path))
					{
						std::filesystem::path relative_texture_path = Utils::NormalisePath(std::filesystem::relative(absolute_texture_path, Project::GetActiveProject()->GetAssetDirectory()));
						bool is_relative_to_project = !relative_texture_path.string().starts_with("..");

						texture_meta_data.AssetName = relative_texture_path.stem().string();
						texture_meta_data.FilePath = is_relative_to_project ? relative_texture_path.string() : absolute_texture_path.string();
						texture_meta_data.Type = AssetType::Texture2D;

						// As this is not an embedded texture, this meta data will not have a parent asset 
						// as the texture is standalone and can be loaded independent of the model being loaded
						texture_meta_data.ParentAssetHandle = NULL_UUID;

						texture_handle = Utils::fnv1a_hash(
							AssetUtils::AssetTypeToString(texture_meta_data.Type) + 
							Utils::NormalisePath(texture_meta_data.FilePath).string()
						);

						// Check if texture file already loaded.
						if (import_params.asset_map->count(texture_handle) == 0)
							texture_asset = std::make_shared<Texture2D>(absolute_texture_path);
						else
							texture_asset = static_pointer_cast<Texture2D>(import_params.asset_map->at(texture_handle));
					}
					else if (auto assimp_texture_ref = ai_scene->GetEmbeddedTexture(assimp_texture_string.C_Str()))
					{
						texture_meta_data.AssetName = assimp_texture_string.C_Str();
						texture_meta_data.FilePath = Utils::NormalisePath(std::filesystem::relative(model_file_path, Project::GetActiveProject()->GetAssetDirectory()));
						texture_meta_data.Type = AssetType::Texture2D;

						// This is an embedded texture which requires the model 
						// to be loaded so we can access this texture
						texture_meta_data.ParentAssetHandle = import_params.asset_handle;

						texture_handle = Utils::fnv1a_hash(
							AssetUtils::AssetTypeToString(texture_meta_data.Type) + 
							model_file_path.filename().string() + 
							texture_meta_data.AssetName
						);

						glm::ivec2 texture_size = { assimp_texture_ref->mWidth, assimp_texture_ref->mHeight };
						unsigned char* texture_data = reinterpret_cast<unsigned char*>(assimp_texture_ref->pcData);

						// Check if texture file already loaded.
						if (import_params.asset_map->count(texture_handle) == 0)
							texture_asset = std::make_shared<Texture2D>(texture_data, texture_size.x, texture_size.y, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8, Texture2D::TextureFormat::BLUE_GREEN_RED_ALPHA_8);
						else
							texture_asset = static_pointer_cast<Texture2D>(import_params.asset_map->at(texture_handle));
					}
					else if (!absolute_texture_path.empty())
					{
						L_CORE_ERROR("ModelImporter::ProcessMaterial: Could Not Find Texture. Path:'{}', Assimp Texture String:'{}'.", absolute_texture_path.string(), assimp_texture_string.C_Str());
						texture_asset = nullptr;
					}

					if (texture_asset)
					{

						switch (texture_type)
						{
						case aiTextureType_DIFFUSE:
						case aiTextureType_BASE_COLOR:
						{
							asset_material->SetAlbedoTexture(texture_handle);
							break;
						}

						case aiTextureType_METALNESS:
						{
							asset_material->SetMetallicTexture(texture_handle);
							break;
						}

						case aiTextureType_NORMALS:
						{
							asset_material->SetNormalTexture(texture_handle);
							break;
						}
						}

						texture_asset->Handle = texture_handle;

						import_params.asset_map->operator[](texture_handle) = texture_asset;
						import_params.asset_reg->operator[](texture_handle) = texture_meta_data;
					}

				};

			aiTextureType texture_base_colour_type = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ? aiTextureType_DIFFUSE : aiTextureType_NONE;

			load_texture(texture_base_colour_type);
			load_texture(aiTextureType_METALNESS);
			load_texture(aiTextureType_NORMALS);

			if (asset_material) {

				if (ImportSkeleton)
				{
					auto& component = model_prefab->GetComponent<SkinnedMeshComponent>(current_entity_handle);
					component.MaterialHandles.push_back({ asset_material_handle, nullptr });
				}
				else
				{
					auto& component = model_prefab->GetComponent<MeshRendererComponent>(current_entity_handle);
					component.MaterialHandles.push_back({ asset_material_handle, nullptr });
				}

				asset_material->Handle = asset_material_handle;
				import_params.asset_map->operator[](asset_material_handle) = asset_material;
				import_params.asset_reg->operator[](asset_material_handle) = material_metadata;
			}

		}
		else
		{
			if (ImportSkeleton)
			{
				model_prefab->GetComponent<SkinnedMeshComponent>(current_entity_handle).MaterialHandles.push_back({ asset_material_handle, nullptr });
			}
			else
			{
				model_prefab->GetComponent<MeshRendererComponent>(current_entity_handle).MaterialHandles.push_back({ asset_material_handle, nullptr });
			}
		}
	}

	void ModelImporter::ProcessStaticMeshNode(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, aiNode* ai_node, std::shared_ptr<Prefab> model_prefab, entt::entity parent_entity_handle, const std::filesystem::path& model_file_path)
	{
		entt::entity current_entity_handle = entt::null;

		// Check if the current node is the root node and has children,
		// if the root node has children, we want to have a root node 
		// that all children attach to which will be the root node.
		if (ai_node == ai_scene->mRootNode && ai_node->mNumChildren > 1)
		{
			current_entity_handle = model_prefab->GetRootEntity();
			model_prefab->SetPrefabName(model_file_path.stem().string());
		}

		if (ai_node->mNumMeshes > 0)
		{
			// If the Current Entity Handle has not been set
			if (current_entity_handle == entt::null)
			{

				if (parent_entity_handle == entt::null) // If there is no parent, get the Root Entity of the Prefab
				{
					current_entity_handle = model_prefab->GetRootEntity();
					model_prefab->SetPrefabName(model_file_path.stem().string());
				}
				else // If there is a parent, we want to create a new sub entity and attach it to that parent in the Prefab registry
				{
					current_entity_handle = model_prefab->CreateEntity(ai_node->mName.C_Str());
					model_prefab->GetComponent<TransformComponent>(current_entity_handle).SetTransform(AssimpHelpers::ConvertMatrixToGLMFormat(ai_node->mTransformation));
					model_prefab->GetComponent<HierarchyComponent>(current_entity_handle).m_Parent = (uint32_t)parent_entity_handle;
					model_prefab->GetComponent<HierarchyComponent>(parent_entity_handle).m_Children.push_back((uint32_t)current_entity_handle);
				}

			}

			// Compare the Node to any Instances Already Loaded
			// This merely compares the meshes contained in the node
			AssimpHelpers::MeshInstanceKey node_key{};
			node_key.GenerateKey(ai_node);

			// We have already loaded an instance of this node
			if (AssimpHelpers::s_LoadedNodes.count(node_key) != 0)
			{
				model_prefab->GetComponent<TransformComponent>(current_entity_handle).SetTransform(AssimpHelpers::ConvertMatrixToGLMFormat(ai_node->mTransformation));

				auto& mesh_filter_component = model_prefab->AddComponent<MeshFilterComponent>(current_entity_handle);
				auto& mesh_renderer_component = model_prefab->AddComponent<MeshRendererComponent>(current_entity_handle);

				mesh_filter_component.StaticMeshHandle = AssimpHelpers::s_LoadedNodes[node_key].first;
				mesh_renderer_component.MaterialHandles = AssimpHelpers::s_LoadedNodes[node_key].second;
			}
			else
			{
				// Generate AssetMesh Handle
				AssetHandle handle = Utils::fnv1a_hash(
					AssetUtils::AssetTypeToString(AssetType::Mesh) + 
					model_file_path.filename().string() + 
					ai_node->mName.C_Str()
				);

				std::shared_ptr<StaticMesh> asset_mesh = std::make_shared<StaticMesh>();
				asset_mesh->Handle = handle;

				AssetMetaData metadata;
				metadata.Type = AssetType::Mesh;
				metadata.AssetName = ai_node->mName.C_Str();
				metadata.ParentAssetHandle = import_params.asset_handle;
				metadata.FilePath = import_params.asset_meta_data.FilePath;
				metadata.IsCustomAsset = import_params.asset_meta_data.IsCustomAsset;

				// Update Prefab Transform and Add Required Components
				model_prefab->AddComponent<MeshFilterComponent>(current_entity_handle).StaticMeshHandle = handle;
				model_prefab->AddComponent<MeshRendererComponent>(current_entity_handle);

				// Process Meshes of the Node
				for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
				{
					aiMesh* ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
					ProcessMesh(ai_scene, ai_mesh, asset_mesh);
					ProcessMaterial(import_params, ai_scene, ai_mesh, model_prefab, current_entity_handle, asset_mesh, model_file_path);

					// Calculate the the AABB of the mesh including any sub meshes
					// Update bounds to include the current submesh
					asset_mesh->MeshBounds.BoundsMax = glm::max(asset_mesh->MeshBounds.BoundsMax,
						glm::vec3{
							ai_mesh->mAABB.mMax.x,
							ai_mesh->mAABB.mMax.y,
							ai_mesh->mAABB.mMax.z,
						});

					asset_mesh->MeshBounds.BoundsMin = glm::min(asset_mesh->MeshBounds.BoundsMin,
						glm::vec3{
							ai_mesh->mAABB.mMin.x,
							ai_mesh->mAABB.mMin.y,
							ai_mesh->mAABB.mMin.z,
						});
				}

				if (asset_mesh)
				{
					// Store instance of node into the loaded node map
					auto& pair = AssimpHelpers::s_LoadedNodes[node_key];
					pair.first = model_prefab->GetComponent<MeshFilterComponent>(current_entity_handle).StaticMeshHandle;
					pair.second = model_prefab->GetComponent<MeshRendererComponent>(current_entity_handle).MaterialHandles;

					// Store instance of AssetMesh in AssetManager registry
					import_params.asset_map->operator[](handle) = asset_mesh;
					import_params.asset_reg->operator[](handle) = metadata;
				}
				else
				{
					model_prefab->RemoveComponent<MeshFilterComponent>(current_entity_handle);
					model_prefab->RemoveComponent<MeshRendererComponent>(current_entity_handle);
				}
			}
		}
		else if (ai_node != ai_scene->mRootNode) // For empty nodes that act as groups, e.g., Empty Axis in blender with Children
		{
			if (parent_entity_handle == entt::null) // If there is no parent, get the Root Entity of the Prefab
			{
				current_entity_handle = model_prefab->GetRootEntity();
				model_prefab->SetPrefabName(model_file_path.stem().string());
			}
			else // If there is a parent, we want to create a new sub entity and attach it to that parent in the Prefab registry
			{
				current_entity_handle = model_prefab->CreateEntity(ai_node->mName.C_Str());
				model_prefab->GetComponent<TransformComponent>(current_entity_handle).SetTransform(AssimpHelpers::ConvertMatrixToGLMFormat(ai_node->mTransformation));
				model_prefab->GetComponent<HierarchyComponent>(current_entity_handle).m_Parent = (uint32_t)parent_entity_handle;
				model_prefab->GetComponent<HierarchyComponent>(parent_entity_handle).m_Children.push_back((uint32_t)current_entity_handle);
			}
		}

		// Process Any Children Nodes
		for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
		{
			// Check if child node has any meshes for import
			if (ai_node == ai_scene->mRootNode)
			{
				std::function<bool(aiNode*)> has_meshes = [&](aiNode* current_node) -> bool
					{
						if (current_node->mNumMeshes > 0)
							return true;

						for (unsigned int j = 0; j < current_node->mNumChildren; j++)
						{
							if (has_meshes(current_node->mChildren[j]))
								return true;
						}
						return false;
					};

				if (!has_meshes(ai_node->mChildren[i]))
					continue;
			}

			ProcessStaticMeshNode(import_params, ai_scene, ai_node->mChildren[i], model_prefab, current_entity_handle, model_file_path);
		}
	}

	entt::entity ModelImporter::ProcessSkeleton(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, const aiNode* ai_node, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path)
	{
		std::unordered_set<std::string> bone_names;

		// Step 1: Collect all bone names from the meshes
		for (unsigned int i = 0; i < ai_scene->mNumMeshes; i++)
		{
			aiMesh* mesh = ai_scene->mMeshes[i];
			for (unsigned int j = 0; j < mesh->mNumBones; j++)
			{
				bone_names.insert(mesh->mBones[j]->mName.C_Str());
			}
		}

		// Step 2: Identify all bone hierarchy root nodes
		std::vector<aiNode*> bone_hierarchy_root_nodes;
		std::function<aiNode* (aiNode*)> find_bone_recursive = [&](aiNode* node) -> aiNode*
		{
			if (bone_names.find(node->mName.C_Str()) != bone_names.end())
			{
				return node; // Found the first bone in the hierarchy
			}

			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				aiNode* found = find_bone_recursive(node->mChildren[i]);
				if (found) return found;
			}

			return nullptr;
		};

		aiNode* first_bone_node = find_bone_recursive(ai_scene->mRootNode);
		if (!first_bone_node)
			return entt::null; // No skeleton found

		// Step 3: If this node has a single parent (armature), return the parent
		if (first_bone_node->mParent && first_bone_node->mParent != ai_scene->mRootNode)
		{
			first_bone_node = first_bone_node->mParent; // Likely an armature node
		}

		// Step 4: Recursive function to create bone hierarchy
		entt::entity first_bone_entity = entt::null;

		std::function<void(entt::entity, aiNode*, BoneLayout&)> create_bones_recursive;
		uint32_t bone_counter = 0;
		create_bones_recursive = [&](entt::entity parent_entity, aiNode* current_node, BoneLayout& current_bone)
		{
			// Create entity for the current bone
			entt::entity bone_entity = model_prefab->CreateEntity(current_node->mName.C_Str());

			current_bone.BoneID = bone_counter;
			current_bone.BoneName = current_node->mName.C_Str();
			bone_counter++;

			auto& skinned_mesh_component = model_prefab->GetComponent<SkinnedMeshComponent>(model_prefab->GetRootEntity());
			skinned_mesh_component.SkeletonBoneMapping[current_bone.BoneID] = (uint32_t)bone_entity;

			// Step 4.1: Set Transform Component using Assimp matrix conversion
			aiMatrix4x4 transform = current_node->mTransformation;
			model_prefab->GetComponent<TransformComponent>(bone_entity).SetTransform(AssimpHelpers::ConvertMatrixToGLMFormat(transform));

			// Step 4.2: Setup hierarchy relationship
			if (parent_entity == entt::null)
			{
				first_bone_entity = bone_entity;
				model_prefab->GetComponent<TagComponent>(bone_entity).Tag = "Skeleton Root Bone";
				model_prefab->GetComponent<HierarchyComponent>(bone_entity).m_Parent = static_cast<uint32_t>(model_prefab->GetRootEntity());
				model_prefab->GetComponent<HierarchyComponent>(model_prefab->GetRootEntity()).m_Children.push_back(static_cast<uint32_t>(bone_entity));
			}
			else
			{
				model_prefab->GetComponent<HierarchyComponent>(bone_entity).m_Parent = static_cast<uint32_t>(parent_entity);
				model_prefab->GetComponent<HierarchyComponent>(parent_entity).m_Children.push_back(static_cast<uint32_t>(bone_entity));
			}

			// Step 4.3: Recursively process child bones
			for (unsigned int i = 0; i < current_node->mNumChildren; i++)
			{
				current_bone.BoneChildren.push_back({});
				create_bones_recursive(bone_entity, current_node->mChildren[i], current_bone.BoneChildren.back());
			}
		};

		// Step 5: Create Skeleton Asset
		auto& skinned_mesh_component = model_prefab->GetComponent<SkinnedMeshComponent>(model_prefab->GetRootEntity());
		skinned_mesh_component.SkeletonHandle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(AssetType::Skeleton) + 
			Utils::NormalisePath(import_params.asset_meta_data.FilePath).string() + 
			first_bone_node->mName.C_Str()
		);

		std::shared_ptr<Skeleton> asset_skeleton = std::make_shared<Skeleton>();
		asset_skeleton->Handle = skinned_mesh_component.SkeletonHandle;

		AssetMetaData metadata;
		metadata.Type = AssetType::Skeleton;
		metadata.AssetName = first_bone_node->mName.C_Str();
		metadata.ParentAssetHandle = import_params.asset_handle;
		metadata.FilePath = import_params.asset_meta_data.FilePath;
		metadata.IsCustomAsset = import_params.asset_meta_data.IsCustomAsset;

		// Add to Asset Registry
		import_params.asset_map->operator[](asset_skeleton->Handle) = asset_skeleton;
		import_params.asset_reg->operator[](asset_skeleton->Handle) = metadata;

		ImportedSkeletonHandle = asset_skeleton->Handle;

		// Step 6: Start recursive creation from the skeleton root
		create_bones_recursive(entt::null, first_bone_node, asset_skeleton->SkeletonLayout);

		return first_bone_entity;
	}

	void ModelImporter::ProcessSkinnedMeshNode(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path)
	{
		entt::entity root_entity_handle = model_prefab->GetRootEntity();
		model_prefab->SetPrefabName(model_file_path.stem().string());

		auto& skinned_mesh_component = model_prefab->GetComponent<SkinnedMeshComponent>(root_entity_handle);

		// Generate AssetMesh Handle
		skinned_mesh_component.StaticMeshHandle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(AssetType::Mesh) + 
			model_file_path.filename().string() + 
			model_file_path.stem().string()
		);

		std::shared_ptr<StaticMesh> asset_mesh = std::make_shared<StaticMesh>();
		asset_mesh->Handle = skinned_mesh_component.StaticMeshHandle;

		AssetMetaData metadata;
		metadata.Type = AssetType::Mesh;
		metadata.AssetName = model_file_path.stem().string();
		metadata.ParentAssetHandle = import_params.asset_handle;
		metadata.FilePath = import_params.asset_meta_data.FilePath;
		metadata.IsCustomAsset = import_params.asset_meta_data.IsCustomAsset;

		std::function<void(aiNode*)> process_skin_mesh_nodes;

		std::shared_ptr<Skeleton> asset_skeleton = nullptr;
		
		if (AssetManager::IsAssetLoaded(skinned_mesh_component.SkeletonHandle))
			asset_skeleton = AssetManager::GetAsset<Skeleton>(skinned_mesh_component.SkeletonHandle);

		process_skin_mesh_nodes = [&](aiNode* current_node)
		{
			// Process Meshes
			if (current_node->mNumMeshes > 0)
			{
				// Process Meshes of the Node
				for (unsigned int i = 0; i < current_node->mNumMeshes; i++)
				{
					aiMesh* ai_mesh = ai_scene->mMeshes[current_node->mMeshes[i]];
					ProcessMesh(ai_scene, ai_mesh, asset_mesh, asset_skeleton ? &asset_skeleton->SkeletonLayout : nullptr);
					ProcessMaterial(import_params, ai_scene, ai_mesh, model_prefab, root_entity_handle, asset_mesh, model_file_path);

					// Calculate the the AABB of the mesh including any sub meshes
					// Update bounds to include the current submesh
					asset_mesh->MeshBounds.BoundsMax = glm::max(asset_mesh->MeshBounds.BoundsMax,
						glm::vec3{
							ai_mesh->mAABB.mMax.x,
							ai_mesh->mAABB.mMax.y,
							ai_mesh->mAABB.mMax.z,
						});

					asset_mesh->MeshBounds.BoundsMin = glm::min(asset_mesh->MeshBounds.BoundsMin,
						glm::vec3{
							ai_mesh->mAABB.mMin.x,
							ai_mesh->mAABB.mMin.y,
							ai_mesh->mAABB.mMin.z,
						});
				}
			}

			// Process Child Nodes
			for (unsigned int i = 0; i < current_node->mNumChildren; i++)
			{
				process_skin_mesh_nodes(current_node->mChildren[i]);
			}
		};

		process_skin_mesh_nodes(ai_scene->mRootNode);

		if (asset_mesh)
		{
			// Store instance of AssetMesh in AssetManager registry
			import_params.asset_map->operator[](asset_mesh->Handle) = asset_mesh;
			import_params.asset_reg->operator[](asset_mesh->Handle) = metadata;
		}
		else
		{
			model_prefab->RemoveComponent<SkinnedMeshComponent>(root_entity_handle);
		}
	}

	void ModelImporter::ProcessAnimations(const AssetImporter::ImportParams& import_params, const aiScene* ai_scene, std::shared_ptr<Prefab> model_prefab, const std::filesystem::path& model_file_path)
	{
		if (!AssetManager::IsAssetLoaded(ImportedSkeletonHandle))
			return;

		std::unordered_set<AssetHandle> loaded_animation_handles;
		for (uint32_t animation_index = 0; animation_index < ai_scene->mNumAnimations; animation_index++)
		{
			aiAnimation* ai_animation = ai_scene->mAnimations[animation_index];
			if (!ai_animation)
				continue;

			AssetHandle animation_clip_handle = Utils::fnv1a_hash(
				AssetUtils::AssetTypeToString(AssetType::AnimationClip) + 
				Utils::NormalisePath(import_params.asset_meta_data.FilePath).string() + 
				ai_animation->mName.C_Str()
			);

			std::shared_ptr<AnimationClip> animation_clip_asset = std::make_shared<AnimationClip>();
			animation_clip_asset->Handle = animation_clip_handle;

			animation_clip_asset->SetDuration(static_cast<float>(ai_animation->mDuration));
			animation_clip_asset->SetTicksPerSecond(static_cast<uint32_t>(ai_animation->mTicksPerSecond));

			for (uint32_t channel_index = 0; channel_index < ai_animation->mNumChannels; channel_index++)
			{
				aiNodeAnim* ai_channel = ai_animation->mChannels[channel_index];
				if (!ai_channel)
					continue;

				BoneKeyframes bone_key_frame;

				for (uint32_t position_index = 0; position_index < ai_channel->mNumPositionKeys; ++position_index)
				{
					aiVector3D ai_position = ai_channel->mPositionKeys[position_index].mValue;
					float time_stamp = static_cast<float>(ai_channel->mPositionKeys[position_index].mTime);

					Keyframe_Position data{};
					data.Position = glm::vec3(ai_position.x, ai_position.y, ai_position.z);
					data.Time = time_stamp;
					bone_key_frame.PositionKeyframes.push_back(data);
				}

				for (uint32_t rotation_index = 0; rotation_index < ai_channel->mNumRotationKeys; ++rotation_index)
				{
					aiQuaternion ai_quaternion = ai_channel->mRotationKeys[rotation_index].mValue;
					float time_stamp = static_cast<float>(ai_channel->mRotationKeys[rotation_index].mTime);

					Keyframe_Rotation data{};
					data.Rotation = glm::quat(ai_quaternion.w, ai_quaternion.x, ai_quaternion.y, ai_quaternion.z);
					data.Rotation = glm::normalize(data.Rotation);
					data.Time = time_stamp;
					bone_key_frame.RotationKeyframes.push_back(data);
				}

				for (uint32_t scale_index = 0; scale_index < ai_channel->mNumScalingKeys; ++scale_index)
				{
					aiVector3D ai_scale = ai_channel->mScalingKeys[scale_index].mValue;
					float time_stamp = static_cast<float>(ai_channel->mScalingKeys[scale_index].mTime);

					Keyframe_Scale data{};
					data.Scale = glm::vec3(ai_scale.x, ai_scale.y, ai_scale.z);
					data.Time = time_stamp;
					bone_key_frame.ScaleKeyframes.push_back(data);
				}

				animation_clip_asset->AddBoneKeyframes(ai_channel->mNodeName.C_Str(), bone_key_frame);
			}

			AssetMetaData animation_clip_meta_data;
			animation_clip_meta_data.Type = AssetType::AnimationClip;
			animation_clip_meta_data.AssetName = ai_animation->mName.C_Str();
			animation_clip_meta_data.ParentAssetHandle = import_params.asset_handle;
			animation_clip_meta_data.FilePath = import_params.asset_meta_data.FilePath;
			animation_clip_meta_data.IsCustomAsset = import_params.asset_meta_data.IsCustomAsset;

			import_params.asset_map->operator[](animation_clip_asset->Handle) = animation_clip_asset;
			import_params.asset_reg->operator[](animation_clip_asset->Handle) = animation_clip_meta_data;

			loaded_animation_handles.insert(animation_clip_handle);
		}

		auto& basic_animation_component = model_prefab->AddComponent<BasicAnimationComponent>(model_prefab->GetRootEntity());
		basic_animation_component.AnimationClipHandles.reserve(loaded_animation_handles.size());
		basic_animation_component.AnimationClipHandles.insert(basic_animation_component.AnimationClipHandles.begin(), loaded_animation_handles.begin(), loaded_animation_handles.end());

		if (basic_animation_component.AnimationClipHandles.size() != 0)
		basic_animation_component.CurrentClipIndex = 0;
	}

#pragma endregion

#pragma region Compute Shader Import

	std::shared_ptr<Shader> ShaderImporter::ImportShader(const AssetImporter::ImportParams& import_params)
	{
		return LoadShader(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	std::shared_ptr<Shader> ShaderImporter::LoadShader(const std::filesystem::path& path)
	{
		if(AssetManager::GetAssetTypeFromFileExtension(path.extension()) == AssetType::Compute_Shader)
			return std::make_shared<Shader>(path.string().c_str(), true);
		else if (AssetManager::GetAssetTypeFromFileExtension(path.extension()) == AssetType::Shader)
			return std::make_shared<Shader>(path.string().c_str(), false);

		return nullptr;
	}

	std::shared_ptr<ComputeShaderAsset> ShaderImporter::ImportComputeShader(const AssetImporter::ImportParams& import_params)
	{
		return LoadComputeShader(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
	}

	std::shared_ptr<ComputeShaderAsset> ShaderImporter::LoadComputeShader(const std::filesystem::path& path) 
	{
		return std::make_shared<ComputeShaderAsset>(path);
	}

#pragma endregion

#pragma region Animation State Machine

    std::shared_ptr<Animation::StateMachine> AnimationStateMachineImporter::ImportStateMachine(const AssetImporter::ImportParams &import_params)
    {
        return LoadStateMachine(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
    }

    std::shared_ptr<Animation::StateMachine> AnimationStateMachineImporter::LoadStateMachine(const std::filesystem::path &path)
    {
		YAML::Node data;

		if (!std::filesystem::exists(path)) return nullptr;

		try 
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) 
		{
			L_CORE_ERROR("YAML-CPP Failed to Load Animation State Machine File: '{}', {}", path.string(), e.what());
			return nullptr;
		}

		if (!data) return nullptr;

		std::shared_ptr<Animation::StateMachine> asset = std::make_shared<Animation::StateMachine>();
		asset->Deserialise(data);

        return asset;
    }

#pragma endregion

#pragma region Humanoid

    std::shared_ptr<Humanoid> HumanoidImporter::ImportHumanoid(const AssetImporter::ImportParams &import_params)
    {
        return LoadHumanoid(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
    }

    std::shared_ptr<HumanoidMask> HumanoidImporter::ImportHumanoidMask(const AssetImporter::ImportParams &import_params)
    {
        return LoadHumanoidMask(import_params.asset_meta_data.IsCustomAsset ? import_params.asset_meta_data.FilePath : Project::GetActiveProject()->GetAssetDirectory() / import_params.asset_meta_data.FilePath);
    }

    std::shared_ptr<Humanoid> HumanoidImporter::LoadHumanoid(const std::filesystem::path &path)
    {
		YAML::Node data;

		if (!std::filesystem::exists(path)) return nullptr;

		try 
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) 
		{
			L_CORE_ERROR("YAML-CPP Failed to Load Humanoid File: '{}', {}", path.string(), e.what());
			return nullptr;
		}

		if (!data) return nullptr;

		std::shared_ptr<Humanoid> asset = std::make_shared<Humanoid>();
		asset->Deserialise(data);

        return asset;
    }

    std::shared_ptr<HumanoidMask> HumanoidImporter::LoadHumanoidMask(const std::filesystem::path &path)
    {
		YAML::Node data;

		if (!std::filesystem::exists(path)) return nullptr;

		try 
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) 
		{
			L_CORE_ERROR("YAML-CPP Failed to Load Humanoid Mask File: '{}', {}", path.string(), e.what());
			return nullptr;
		}

		if (!data) return nullptr;

		std::shared_ptr<HumanoidMask> asset = std::make_shared<HumanoidMask>();
		asset->Deserialise(data);

        return asset;
    }

#pragma endregion


}