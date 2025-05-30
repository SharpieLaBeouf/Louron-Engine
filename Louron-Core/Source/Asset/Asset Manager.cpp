#include "Asset Manager.h"

// Louron Core Headers
#include "Asset Importer.h"
#include "Asset Manager API.h"

#include "../Core/Engine.h"
#include "../Core/FileSystem Utilities.h"
#include "../Debug/Profiler.h"
#include "../Project/Project.h"

// C++ Standard Library Headers
#include <fstream>

// External Vendor Library Headers
#include <yaml-cpp/yaml.h>

namespace Louron 
{
	#pragma region Asset Manager Static API

	AssetType AssetManager::GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
	{
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			L_CORE_WARN("Could not find AssetType for {0}", extension.string().c_str());
			return AssetType::Unknown;
		}

		return s_AssetExtensionMap.at(extension);
	}

	bool AssetManager::IsExtensionSupported(const std::filesystem::path& extension) {
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
			return false;
		return true;
	}

	bool AssetManager::IsAssetTypeComposite(const AssetType& asset_type)
	{

		switch (asset_type) {

			case AssetType::Scene:
			case AssetType::Prefab:
			case AssetType::ModelImport:
			case AssetType::Material_Skybox:
			case AssetType::Material_Standard:
			case AssetType::AnimationStateMachine:
			{
				return true;
			}

		}

		return false;
	}

	#pragma endregion

	#pragma region Editor Asset Manager

	void EditorAssetManager::RefreshAssetRegistry(const std::filesystem::path& project_asset_directory)
	{
		if (!std::filesystem::exists(project_asset_directory))
		{
			L_CORE_WARN("Could Not Refresh Asset Registry: Project Asset Directory Does Not Exist {}", project_asset_directory.string());
			return;
		}

		AssetRegistry new_registry{};
		AssetMap new_asset_map{};

		auto temporary_import = [&](const std::filesystem::path& file_path) {

			// Generate New Meta Data
			AssetMetaData meta_data;
			meta_data.AssetName = file_path.stem().string();
			meta_data.FilePath = std::filesystem::relative(file_path, project_asset_directory);
			meta_data.Type = AssetManager::GetAssetTypeFromFileExtension(file_path.extension());
			meta_data.IsComposite = AssetManager::IsAssetTypeComposite(meta_data.Type);
			
			switch (meta_data.Type)
			{
				case AssetType::ModelImport:
				{
					meta_data.ImportConfig = std::make_shared<ModelImportConfig>();
					break;
				}
				default:
				{
					meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
					break;
				}
			}

			// Generate New Unique Handle
			AssetHandle handle = GenerateNewAssetHandle(meta_data.Type, meta_data.FilePath);

			// Temporarily Load Asset to Load MetaData
			if (new_asset_map.count(handle) == 0)
			{
				std::shared_ptr<Asset> asset = AssetImporter::ImportAsset(&new_asset_map, &new_registry, handle, meta_data, project_asset_directory);

				if (asset)
				{
					asset->Handle = handle;
					new_asset_map[handle] = asset;
					new_registry[handle] = meta_data;

					if (meta_data.ParentAssetHandle == NULL_UUID)
					{
						YAML::Emitter out;
						{
							out << YAML::BeginMap; // Root

							out << YAML::Key << "Asset Name" << YAML::Value << meta_data.AssetName;
							out << YAML::Key << "Asset Handle" << YAML::Value << handle;
							out << YAML::Key << "Asset Type" << YAML::Value << AssetUtils::AssetTypeToString(meta_data.Type);
							out << YAML::Key << "Asset Is Composite" << YAML::Value << meta_data.IsComposite;

							out << YAML::Key << "Asset Import Config" << YAML::Value << YAML::BeginMap;
							{
								if (meta_data.ImportConfig)
									out << YAML::Key << "AutoReImport" << YAML::Value << meta_data.ImportConfig->AutoReimport;

								switch (meta_data.Type)
								{

									case AssetType::ModelImport:
									{
										auto import_config = static_pointer_cast<ModelImportConfig>(meta_data.ImportConfig);

										if (!import_config)
											break;

										out << YAML::Key << "Import Position" << YAML::Value << YAML::Flow << YAML::BeginSeq;
										for (int i = 0; i < 3; i++)
											out << import_config->ImportPosition[i];
										out << YAML::EndSeq;

										out << YAML::Key << "Import Rotation" << YAML::Value << YAML::Flow << YAML::BeginSeq;
										for (int i = 0; i < 4; i++)
											out << import_config->ImportRotation[i];
										out << YAML::EndSeq;

										out << YAML::Key << "Import Scale" << YAML::Value << YAML::Flow << YAML::BeginSeq;
										for (int i = 0; i < 3; i++)
											out << import_config->ImportScale[i];
										out << YAML::EndSeq;

										out << YAML::Key << "Import Skeleton" << YAML::Value << import_config->ImportSkeleton;
										out << YAML::Key << "Import Animations" << YAML::Value << import_config->ImportAnimations;
										out << YAML::Key << "Import Materials" << YAML::Value << import_config->ImportMaterials;

										break;
									}

								}

							}
							out << YAML::EndMap;

							if (meta_data.IsComposite)
							{
								out << YAML::Key << "Composite Assets" << YAML::Value << YAML::BeginSeq;

								for (const auto& [sub_handle, sub_metadata] : new_registry)
								{
									if (sub_metadata.ParentAssetHandle != handle)
										continue;

									out << YAML::BeginMap;
									out << YAML::Key << "Asset Name" << YAML::Value << sub_metadata.AssetName;
									out << YAML::Key << "Asset Handle" << YAML::Value << sub_handle;
									out << YAML::Key << "Asset Type" << YAML::Value << AssetUtils::AssetTypeToString(sub_metadata.Type);
									out << YAML::EndMap;

								}

								out << YAML::EndSeq;
							}

							out << YAML::EndMap; // Root
						}

						std::ofstream fout(static_cast<std::filesystem::path>(file_path.string() + ".meta"));
						fout << out.c_str();
					}

				}
			}
		};

		for (const auto& entry_path : std::filesystem::recursive_directory_iterator(project_asset_directory)) 
		{
			if (entry_path.is_directory() || !entry_path.is_regular_file())
				continue; // Not a Regular File That Can Be Imported

			if (!AssetManager::IsExtensionSupported(entry_path.path().extension()))
				continue; // File Type Not Supported for Import

			const std::filesystem::path& file_path = entry_path.path();
			const std::filesystem::path& meta_file_path = file_path.string() + ".meta";

			if (std::filesystem::exists(meta_file_path))  // If there is already a meta data file created for this asset, simply read in the meta data
			{
				YAML::Node data;
				try
				{
					data = YAML::LoadFile(meta_file_path.string());
				}
				catch (YAML::ParserException e)
				{
					L_CORE_ERROR("EditorAssetManager::RefreshAssetRegistry: Failed to Load Asset Meta Data File: '{}', {}.", meta_file_path.string(), e.what());
					L_CORE_WARN("EditorAssetManager::RefreshAssetRegistry: Reimporting Asset '{}' and Creating New Meta Data File.", file_path.stem().string());
					temporary_import(file_path);
					continue; // Skip the deserialisation code after importing.
				}

				AssetHandle handle;
				AssetMetaData meta_data;

				if (data["Asset Name"])
					meta_data.AssetName = data["Asset Name"].as<std::string>();

				if(data["Asset Handle"])
					handle = data["Asset Handle"].as<uint32_t>();

				if(data["Asset Type"])
					meta_data.Type = AssetUtils::AssetTypeFromString(data["Asset Type"].as<std::string>());

				if(data["Asset Is Composite"])
					meta_data.IsComposite = data["Asset Is Composite"].as<bool>();

				meta_data.FilePath = std::filesystem::relative(file_path, project_asset_directory);

				switch (meta_data.Type)
				{
					case AssetType::ModelImport:
					{
						meta_data.ImportConfig = std::make_shared<ModelImportConfig>();

						auto import_config = static_pointer_cast<ModelImportConfig>(meta_data.ImportConfig);
						if (!import_config)
							break;

						if (auto import_config_node = data["Asset Import Config"]; import_config_node)
						{
							if (import_config_node["AutoReImport"])
								import_config->AutoReimport = import_config_node["AutoReImport"].as<bool>();

							if (import_config_node["Import Position"] && import_config_node["Import Position"].IsSequence() && import_config_node["Import Position"].size() == 3)
							{
								import_config->ImportPosition.x = import_config_node["Import Position"][0].as<float>();
								import_config->ImportPosition.y = import_config_node["Import Position"][1].as<float>();
								import_config->ImportPosition.z = import_config_node["Import Position"][2].as<float>();
							}
							if (import_config_node["Import Rotation"] && import_config_node["Import Rotation"].IsSequence() && import_config_node["Import Rotation"].size() == 4)
							{
								import_config->ImportRotation.w = import_config_node["Import Rotation"][0].as<float>();
								import_config->ImportRotation.x = import_config_node["Import Rotation"][1].as<float>();
								import_config->ImportRotation.y = import_config_node["Import Rotation"][2].as<float>();
								import_config->ImportRotation.z = import_config_node["Import Rotation"][3].as<float>();
							}
							if (import_config_node["Import Scale"] && import_config_node["Import Scale"].IsSequence() && import_config_node["Import Scale"].size() == 3)
							{
								import_config->ImportScale.x = import_config_node["Import Scale"][0].as<float>();
								import_config->ImportScale.y = import_config_node["Import Scale"][1].as<float>();
								import_config->ImportScale.z = import_config_node["Import Scale"][2].as<float>();
							}

							if (import_config_node["Import Skeleton"]) import_config->ImportSkeleton = import_config_node["Import Skeleton"].as<bool>();
							if (import_config_node["Import Animations"]) import_config->ImportAnimations = import_config_node["Import Animations"].as<bool>();
							if (import_config_node["Import Materials"]) import_config->ImportMaterials = import_config_node["Import Materials"].as<bool>();
						}

						break;
					}
					default:
					{
						meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
						break;
					}
				}

				if (auto import_config_node = data["Asset Import Config"]; import_config_node && meta_data.ImportConfig)
				{
					if (import_config_node["AutoReImport"])
						meta_data.ImportConfig->AutoReimport = import_config_node["AutoReImport"].as<bool>();
				}

				new_registry[handle] = meta_data;

				// If this Asset is a Composite Asset, we will loop through the Composite 
				// Assets YAML Map Node and find all related sub assets that are part of this asset
				if (meta_data.IsComposite)
				{
					for (const auto& sub_asset_data : data["Composite Assets"])
					{
						AssetHandle sub_handle;
						AssetMetaData sub_meta_data;

						if (sub_asset_data["Asset Name"])
							sub_meta_data.AssetName = sub_asset_data["Asset Name"].as<std::string>();

						if (sub_asset_data["Asset Handle"])
							sub_handle = sub_asset_data["Asset Handle"].as<uint32_t>();

						if (sub_asset_data["Asset Type"])
							sub_meta_data.Type = AssetUtils::AssetTypeFromString(sub_asset_data["Asset Type"].as<std::string>());

						sub_meta_data.ParentAssetHandle = handle;
						sub_meta_data.FilePath = meta_data.FilePath;

						new_registry[sub_handle] = sub_meta_data;
					}
				}
			}
			else // If there is not a meta data file, this is the first import and we will need to load the asset to generate the meta data file
			{
				temporary_import(file_path);
			}
		}

		// Remove Old Meta Data No Longer in Project From this->AssetRegistry
		for (auto it = m_AssetRegistry.begin(); it != m_AssetRegistry.end(); ) 
		{
			AssetHandle handle = it->first;
			if (new_registry.count(handle) == 0 && !it->second.IsCustomAsset) // Only erase if this is NOT a custom asset, and is not found in the new registry
			{				
				it = m_AssetRegistry.erase(it);
				
				if (IsAssetLoaded(handle))
					m_LoadedAssets.erase(handle);

				continue;
			}

			++it;
		}

		// Update Meta Data in this->AssetRegistry
		for (const auto& [asset_handle, asset_meta_data] : new_registry)
		{
			m_AssetRegistry[asset_handle] = asset_meta_data;
		}

		// Force Cleanup Even Though They Are Stack Allocated (can never be too sure)
		new_asset_map.clear();
		new_registry.clear();
	}

	void EditorAssetManager::UpdateAssetMetaData(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data)
	{
		m_AssetRegistry[asset_handle] = asset_meta_data;

		SerialiseMetaDataFile(asset_handle, asset_meta_data, Project::GetActiveProject()->GetAssetDirectory() / (asset_meta_data.FilePath.string() + ".meta"));

		if (!asset_meta_data.IsComposite)
			return;

		// Launch a detached thread to update children file path meta data
		std::thread([this, asset_handle, asset_meta_data]() {

			for (auto& [handle, meta_data] : m_AssetRegistry)
			{
				if (meta_data.ParentAssetHandle == asset_handle)
				{
					meta_data.FilePath = asset_meta_data.FilePath;
				}
			}

		}).detach();
	}

	void EditorAssetManager::AddCustomAsset(std::shared_ptr<Asset> asset, const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data)
	{
		if (!asset)
			return;

		if (m_AssetRegistry.count(asset_handle) != 0)
		{
			L_CORE_WARN("Could Not Add Custom Asset - Handle Already Exists.");
			return;
		}

		AssetMetaData meta_data = asset_meta_data;
		meta_data.IsCustomAsset = true;
		
		switch (meta_data.Type)
		{
			case AssetType::ModelImport:
			{
				meta_data.ImportConfig = std::make_shared<ModelImportConfig>();
				break;
			}
			default:
			{
				meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
				break;
			}
		}

		m_AssetRegistry[asset_handle] = meta_data;
		m_LoadedAssets[asset_handle] = asset;
	}

	void EditorAssetManager::ImportCustomAsset(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data)
	{
		if (m_AssetRegistry.count(asset_handle) != 0)
		{
			L_CORE_WARN("Could Not Add Custom Asset - Handle Already Exists.");
			return;
		}

		if (!std::filesystem::exists(asset_meta_data.FilePath))
		{
			L_CORE_WARN("Could Not Load Custom Asset - File Path Does Not Exist.");
			return;
		}

		// Temporarily Load Asset to Load MetaData
		if (m_LoadedAssets.count(asset_handle) == 0)
		{
			AssetMetaData meta_data = asset_meta_data;
			meta_data.IsCustomAsset = true;
			std::shared_ptr<Asset> asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, asset_handle, meta_data, "");

			if (asset)
			{
				asset->Handle = asset_handle;
				m_LoadedAssets[asset_handle] = asset;
				m_AssetRegistry[asset_handle] = meta_data;
			}
		}
	}

	void EditorAssetManager::ReImportCustomAsset(const std::filesystem::path& asset_file_path)
	{
		AssetHandle handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(AssetManager::GetAssetTypeFromFileExtension(asset_file_path.extension())) + 
			"InBuiltAsset" + 
			asset_file_path.stem().string()
		);

		AssetMetaData meta_data = GetMetadata(handle);

		if (handle != NULL_UUID && IsAssetHandleValid(handle)) // If handle is valid, then we remove the asset
			RemoveAsset(handle);

		ImportCustomAsset(handle, meta_data);
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory, const AssetHandle custom_handle)
	{
		if (!std::filesystem::exists(asset_file_path))
		{
			L_CORE_WARN("Could Not Load Asset File That Does Not Exist: {}", asset_file_path.generic_string());
			return NULL_UUID;
		}

		if (!AssetManager::IsExtensionSupported(asset_file_path.extension()))
		{
			L_CORE_WARN("Could Not Load UnSupported Asset File Type: {}", asset_file_path.generic_string());
			return NULL_UUID;
		}

		AssetMetaData meta_data;
		AssetHandle handle;
		
		if (std::filesystem::exists(asset_file_path.generic_string() + ".meta"))
		{
			YAML::Node data;
			try
			{
				data = YAML::LoadFile(asset_file_path.generic_string() + ".meta");

				if (data["Asset Name"])
					meta_data.AssetName = data["Asset Name"].as<std::string>();

				if (data["Asset Handle"])
					handle = data["Asset Handle"].as<uint32_t>();

				if (data["Asset Type"])
					meta_data.Type = AssetUtils::AssetTypeFromString(data["Asset Type"].as<std::string>());

				if (data["Asset Is Composite"])
					meta_data.IsComposite = data["Asset Is Composite"].as<bool>();
				
				switch (meta_data.Type)
				{
					case AssetType::ModelImport:
					{
						meta_data.ImportConfig = std::make_shared<ModelImportConfig>();

						auto import_config = static_pointer_cast<ModelImportConfig>(meta_data.ImportConfig);
						if (!import_config)
							break;

						if (auto import_config_node = data["Asset Import Config"]; import_config_node)
						{
							if (import_config_node["AutoReImport"])
								import_config->AutoReimport = import_config_node["AutoReImport"].as<bool>();

							if (import_config_node["Import Position"] && import_config_node["Import Position"].IsSequence() && import_config_node["Import Position"].size() == 3)
							{
								import_config->ImportPosition.x = import_config_node["Import Position"][0].as<float>();
								import_config->ImportPosition.y = import_config_node["Import Position"][1].as<float>();
								import_config->ImportPosition.z = import_config_node["Import Position"][2].as<float>();
							}
							if (import_config_node["Import Rotation"] && import_config_node["Import Rotation"].IsSequence() && import_config_node["Import Rotation"].size() == 4)
							{
								import_config->ImportRotation.w = import_config_node["Import Rotation"][0].as<float>();
								import_config->ImportRotation.x = import_config_node["Import Rotation"][1].as<float>();
								import_config->ImportRotation.y = import_config_node["Import Rotation"][2].as<float>();
								import_config->ImportRotation.z = import_config_node["Import Rotation"][3].as<float>();
							}
							if (import_config_node["Import Scale"] && import_config_node["Import Scale"].IsSequence() && import_config_node["Import Scale"].size() == 3)
							{
								import_config->ImportScale.x = import_config_node["Import Scale"][0].as<float>();
								import_config->ImportScale.y = import_config_node["Import Scale"][1].as<float>();
								import_config->ImportScale.z = import_config_node["Import Scale"][2].as<float>();
							}

							if (import_config_node["Import Skeleton"]) import_config->ImportSkeleton = import_config_node["Import Skeleton"].as<bool>();
							if (import_config_node["Import Animations"]) import_config->ImportAnimations = import_config_node["Import Animations"].as<bool>();
							if (import_config_node["Import Materials"]) import_config->ImportMaterials = import_config_node["Import Materials"].as<bool>();
						}

						break;
					}
					default:
					{
						meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
						break;
					}
				}

				if (auto import_config_node = data["Asset Import Config"]; import_config_node && meta_data.ImportConfig)
				{
					if (import_config_node["AutoReImport"])
						meta_data.ImportConfig->AutoReimport = import_config_node["AutoReImport"].as<bool>();
				}

				meta_data.FilePath = std::filesystem::relative(asset_file_path, project_asset_directory);
			}
			catch (YAML::ParserException e)
			{
				meta_data.AssetName = asset_file_path.stem().string();
				meta_data.FilePath = std::filesystem::relative(asset_file_path, project_asset_directory);
				meta_data.Type = AssetManager::GetAssetTypeFromFileExtension(asset_file_path.extension());
				meta_data.IsComposite = AssetManager::IsAssetTypeComposite(meta_data.Type);
				
				switch (meta_data.Type)
				{
					case AssetType::ModelImport:
					{
						meta_data.ImportConfig = std::make_shared<ModelImportConfig>();
						break;
					}
					default:
					{
						meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
						break;
					}
				}
				handle = (custom_handle == NULL_UUID) ? GenerateNewAssetHandle(meta_data.Type, meta_data.FilePath) : custom_handle;
			}
		}
		else
		{
			meta_data.AssetName = asset_file_path.stem().string();
			meta_data.FilePath = std::filesystem::relative(asset_file_path, project_asset_directory);
			meta_data.Type = AssetManager::GetAssetTypeFromFileExtension(asset_file_path.extension());
			meta_data.IsComposite = AssetManager::IsAssetTypeComposite(meta_data.Type);
			
			switch (meta_data.Type)
			{
				case AssetType::ModelImport:
				{
					meta_data.ImportConfig = std::make_shared<ModelImportConfig>();
					break;
				}
				default:
				{
					meta_data.ImportConfig = std::make_shared<AssetImportConfig>();
					break;
				}
			}
			
			handle = (custom_handle == NULL_UUID) ? GenerateNewAssetHandle(meta_data.Type, meta_data.FilePath) : custom_handle;
		}

		L_CORE_ASSERT(meta_data.Type != AssetType::Unknown, "Cannot Load Asset as MetaData Type Is NULL.");

		// Temporarily Load Asset to Load MetaData
		if (m_LoadedAssets.count(handle) == 0)
		{
			std::shared_ptr<Asset> asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, handle, meta_data, project_asset_directory);

			if (asset)
			{
				asset->Handle = handle;
				m_LoadedAssets[handle] = asset;
				m_AssetRegistry[handle] = meta_data;

				SerialiseMetaDataFile(handle, meta_data, static_cast<std::filesystem::path>(asset_file_path.generic_string() + ".meta"));
			}
			else {
				handle = NULL_UUID;
			}
		}
		else {
			L_CORE_INFO("Asset Already Registered: {0}", meta_data.FilePath.generic_string());
		}

		return handle;
	}

	AssetHandle EditorAssetManager::ReImportAsset(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory)
	{
		AssetHandle asset_handle = GetHandleFromFilePath(asset_file_path, project_asset_directory); // Get the handle

		if(IsAssetHandleValid(asset_handle)) // If handle is valid, then we remove the asset
			RemoveAsset(asset_handle);

		// TODO: Think About If We Should Try Loading First, then Removing, 
		// e.g., keep the current asset loaded if new import fails - Throw message if fails?
		auto asset = ImportAsset(asset_file_path, project_asset_directory);

		if (GetAssetType(asset_handle) != AssetType::Shader)
			return asset;

		// Launch a detached thread to update all PBR Materials
		std::thread([this, project_asset_directory]() {

			AssetRegistry registry_copy = m_AssetRegistry;
			for (auto& [handle, meta_data] : registry_copy)
			{
				if (meta_data.ParentAssetHandle == NULL_UUID && meta_data.Type == AssetType::Material_Standard)
				{
					Engine::Get().SubmitToMainThread([this, meta_data, project_asset_directory]() {
						ReImportAsset(project_asset_directory / meta_data.FilePath, project_asset_directory);
					});
				}
			}

		}).detach();

		return asset;
	}

	std::shared_ptr<Asset> EditorAssetManager::LoadAsset(const AssetHandle& asset_handle)
	{
		if (!IsAssetHandleValid(asset_handle))
			return nullptr;

		std::shared_ptr<Asset> asset = nullptr;
		if (IsAssetLoaded(asset_handle)) {
			asset = m_LoadedAssets.at(asset_handle);
		}
		else
		{
			const AssetMetaData& meta_data = m_AssetRegistry[asset_handle];
			const AssetHandle& parent_handle = meta_data.ParentAssetHandle;

			if (parent_handle == NULL_UUID) // No Parent, Just Load
			{
				asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, asset_handle, meta_data, Project::GetActiveProject()->GetAssetDirectory());
				if (!asset) 
				{
					L_CORE_ERROR("EditorAssetManager::GetAsset - Asset Load Failed!");
					return nullptr;
				}

				SerialiseMetaDataFile(asset_handle, meta_data, Project::GetActiveProject()->GetAssetDirectory() / (meta_data.FilePath.string() + ".meta"));

				asset->Handle = asset_handle;
				m_LoadedAssets[asset_handle] = asset;
			}
			else // Load Parent First
			{
				// Check if Parent Handle is Valid
				if (!IsAssetHandleValid(parent_handle)) {

					// If it is not valid, remove all other children that reference this parent handle
					for (auto it = m_AssetRegistry.begin(); it != m_AssetRegistry.end(); )
					{
						if (it->second.ParentAssetHandle == parent_handle)
							it = m_AssetRegistry.erase(it);
						else 
							++it;
					}

					return nullptr;
				}

				// Load the Parent Asset if it is Valid
				const AssetMetaData& parent_metadata = GetMetadata(parent_handle);
				asset = AssetImporter::ImportAsset(&m_LoadedAssets, &m_AssetRegistry, parent_handle, parent_metadata, Project::GetActiveProject()->GetAssetDirectory());
				if (!asset) {
					L_CORE_ERROR("EditorAssetManager::GetAsset - Parent Asset Import Failed!");
					return nullptr;
				}

				SerialiseMetaDataFile(parent_handle, parent_metadata, Project::GetActiveProject()->GetAssetDirectory() / (parent_metadata.FilePath.string() + ".meta"));

				m_LoadedAssets[parent_handle] = asset;

				// Get loaded child asset
				if (IsAssetLoaded(asset_handle)) {
					asset->Handle = asset_handle;
					asset = m_LoadedAssets.at(asset_handle);
				}
				else {
					L_CORE_ERROR("EditorAssetManager::GetAsset - Child Not Available After Parent Loaded!");
					asset = nullptr; // If the child asset was not loaded when the parent was, then it is invalid!
				}
			}
		}

		return asset;
	}

	void EditorAssetManager::UnLoadAsset(const AssetHandle& asset_handle)
	{
		bool is_composite = false;

		if (m_AssetRegistry.count(asset_handle) != 0)
			is_composite = m_AssetRegistry.at(asset_handle).IsComposite;
		
		if (m_LoadedAssets.count(asset_handle) != 0)
			m_LoadedAssets.erase(asset_handle);

		if (!is_composite)
			return;

		// Launch a detached thread to unload children
		std::thread([this, asset_handle]() {

			for (const auto& [handle, meta_data] : m_AssetRegistry) 
			{
				if (meta_data.ParentAssetHandle == asset_handle)
				{
					m_LoadedAssets.erase(handle);	// Unload internal assets of composite asset
				}
			}

		}).detach();
	}

	void EditorAssetManager::RemoveAsset(const AssetHandle& asset_handle)
	{
		bool is_composite = false;

		if (m_AssetRegistry.count(asset_handle) != 0)
		{
			is_composite = m_AssetRegistry.at(asset_handle).IsComposite;
			m_AssetRegistry.erase(asset_handle);
		}

		if (m_LoadedAssets.count(asset_handle) != 0)
			m_LoadedAssets.erase(asset_handle);

		if (!is_composite)
			return;

		// Launch a detached thread to remove children
		std::thread([this, asset_handle]() {

			for (auto it = m_AssetRegistry.begin(); it != m_AssetRegistry.end();)
			{
				if (it->second.ParentAssetHandle == asset_handle)
				{
					m_LoadedAssets.erase(it->first);	// Unload child assets
					it = m_AssetRegistry.erase(it);		// Erase safely
				}
				else
				{
					++it;
				}
			}

		}).detach();

	}

	std::shared_ptr<Asset> EditorAssetManager::GetAsset(const AssetHandle& asset_handle)
	{
		// 1. VALID - Check if the Asset Handle is valid
		if (!IsAssetHandleValid(asset_handle))
			return nullptr;

		// 2. GET LOADED ASSET OR LOAD
		return LoadAsset(asset_handle);
	}

	void EditorAssetManager::AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data)
	{
		asset->Handle = asset_handle;
		m_LoadedAssets[asset_handle] = asset;
		m_AssetRegistry[asset_handle] = asset_meta_data;
		m_RuntimeCreatedAssetRegistry.push_back(asset_handle);
	}

	void EditorAssetManager::RemoveRuntimeAsset(AssetHandle asset_handle)
	{
		if (std::find(m_RuntimeCreatedAssetRegistry.begin(), m_RuntimeCreatedAssetRegistry.end(), asset_handle) == m_RuntimeCreatedAssetRegistry.end())
			return; // Only delete if it is a runtime asset, we do not want to delete any other assets not created during runtime!

		m_LoadedAssets.erase(asset_handle);
		m_AssetRegistry.erase(asset_handle);
		m_RuntimeCreatedAssetRegistry.erase(std::remove(m_RuntimeCreatedAssetRegistry.begin(), m_RuntimeCreatedAssetRegistry.end(), asset_handle), m_RuntimeCreatedAssetRegistry.end());
	}

	void EditorAssetManager::ClearRuntimeAssets()
	{
		for (const auto& handle : m_RuntimeCreatedAssetRegistry)
		{
			if(m_LoadedAssets.contains(handle))
				m_LoadedAssets.erase(handle);

			if (m_AssetRegistry.contains(handle))
				m_AssetRegistry.erase(handle);
		}

		m_RuntimeCreatedAssetRegistry.clear();
	}

	std::shared_ptr<Shader> EditorAssetManager::GetInbuiltShader(const std::string& default_shader_name, bool is_compute)
	{
		AssetHandle handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(AssetType::Shader) + 
			"InBuiltAsset" + 
			default_shader_name
		);

		return static_pointer_cast<Shader>(LoadAsset(handle));
	}

	void EditorAssetManager::InitDefaultResources()
	{
		AssetMetaData meta_data;
		meta_data.IsCustomAsset = true;

		// SHADERS
		for (const auto& entry : std::filesystem::recursive_directory_iterator("Resources/Shaders/"))
		{
			if (!AssetManager::IsExtensionSupported(entry.path().extension()))
				continue;

			AssetType type = AssetManager::GetAssetTypeFromFileExtension(entry.path().extension());
			if (type != AssetType::Shader && type != AssetType::Compute_Shader)
				continue;

			meta_data.AssetName = entry.path().stem().string();
			meta_data.Type = AssetType::Shader;
			meta_data.FilePath = entry.path();
			meta_data.IsComposite = false;

			AssetHandle handle = Utils::fnv1a_hash(
				AssetUtils::AssetTypeToString(meta_data.Type) + 
				"InBuiltAsset" + 
				entry.path().stem().string()
			);

			ImportCustomAsset(handle, meta_data);
		}

		// MODELS
		for (const auto& entry : std::filesystem::recursive_directory_iterator("Resources/Models/"))
		{
			if (!AssetManager::IsExtensionSupported(entry.path().extension()))
				continue;

			AssetType type = AssetManager::GetAssetTypeFromFileExtension(entry.path().extension());
			if (type != AssetType::ModelImport)
				continue;

			meta_data.AssetName = entry.path().stem().string();
			meta_data.Type = AssetType::ModelImport;
			meta_data.FilePath = entry.path();
			meta_data.IsComposite = true;

			AssetHandle handle = Utils::fnv1a_hash(
				AssetUtils::AssetTypeToString(meta_data.Type) + 
				"InBuiltAsset" + 
				entry.path().stem().string()
			);

			ImportCustomAsset(handle, meta_data);
		}

		// TEXTURES
		for (const auto& entry : std::filesystem::recursive_directory_iterator("Resources/Textures/"))
		{
			if (!AssetManager::IsExtensionSupported(entry.path().extension()))
				continue;

			AssetType type = AssetManager::GetAssetTypeFromFileExtension(entry.path().extension());
			if (type != AssetType::Texture2D)
				continue;

			meta_data.AssetName = entry.path().stem().string();
			meta_data.Type = AssetType::Texture2D;
			meta_data.FilePath = entry.path();
			meta_data.IsComposite = false;

			AssetHandle handle = Utils::fnv1a_hash(
				AssetUtils::AssetTypeToString(meta_data.Type) + 
				"InBuiltAsset" + 
				entry.path().stem().string()
			);

			ImportCustomAsset(handle, meta_data);
		}

		meta_data.AssetName = "Default_White_Texture";
		meta_data.Type = AssetType::Texture2D;
		meta_data.FilePath = "";
		meta_data.IsComposite = false;

		unsigned char texture_data[] = { 255, 255, 255, 255 };
		std::shared_ptr<Texture2D> default_texture = std::make_shared<Texture2D>(texture_data, 1, 1, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8);

		AssetHandle handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(meta_data.Type) + 
			"InBuiltAsset" + 
			meta_data.AssetName
		);
		default_texture->Handle = handle;

		AddCustomAsset(default_texture, handle, meta_data);

		texture_data[0] = 128;
		texture_data[1] = 128;

		std::shared_ptr<Texture2D> default_normal_texture = std::make_shared<Texture2D>(texture_data, 1, 1, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8);

		meta_data.AssetName = "Default_Normal_Texture";
		handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(meta_data.Type) + 
			"InBuiltAsset" + 
			meta_data.AssetName
		);
		default_normal_texture->Handle = handle;

		AddCustomAsset(default_normal_texture, handle, meta_data);

		std::shared_ptr<Material> default_material = std::make_shared<Material>();

		meta_data.AssetName = "Default_Material";
		meta_data.Type = AssetType::Material_Standard;
		handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(meta_data.Type) + 
			"InBuiltAsset" + 
			meta_data.AssetName
		);
		default_material->Handle = handle;

		AddCustomAsset(default_material, handle, meta_data);
	}

	bool EditorAssetManager::IsAssetHandleValid(const AssetHandle& asset_handle) const {
		return asset_handle != NULL_UUID && m_AssetRegistry.find(asset_handle) != m_AssetRegistry.end();
	}

	bool EditorAssetManager::IsAssetLoaded(const AssetHandle& asset_handle) const {
		return m_LoadedAssets.find(asset_handle) != m_LoadedAssets.end();
	}

	AssetType EditorAssetManager::GetAssetType(const AssetHandle& asset_handle) const {
		if (!IsAssetHandleValid(asset_handle))
			return AssetType::Unknown;

		return m_AssetRegistry.at(asset_handle).Type;
	}

	const AssetMetaData& EditorAssetManager::GetMetadata(const AssetHandle& asset_handle) const {

		auto it = m_AssetRegistry.find(asset_handle);
		if (it == m_AssetRegistry.end())
		{
			static AssetMetaData s_NullMetadata;
			return s_NullMetadata;
		}

		return it->second;
	}

	AssetHandle EditorAssetManager::GetHandleFromFilePath(const std::filesystem::path& asset_file_path, const std::filesystem::path& project_asset_directory) {

		if (!asset_file_path.is_absolute() || (!project_asset_directory.empty() && !project_asset_directory.is_absolute()))
		{
			L_CORE_WARN("EditorAssetManager::GetHandleFromFilePath: Could Not Get Handle - Path's Are Not Absolute.");
			return NULL_UUID;
		}

		std::filesystem::path meta_data_file_path = asset_file_path.string() + ".meta";
		if (std::filesystem::exists(meta_data_file_path))
		{
			YAML::Node data;
			try
			{
				data = YAML::LoadFile(meta_data_file_path.string());
			}
			catch (YAML::ParserException e)
			{
				L_CORE_ERROR("EditorAssetManager::GetHandleFromFilePath: Failed to Load Asset Meta Data File: '{}', {}.", meta_data_file_path.string(), e.what());
				L_CORE_WARN("EditorAssetManager::GetHandleFromFilePath: Reimporting Asset '{}' and Creating New Meta Data File.", meta_data_file_path.stem().string());
				return NULL_UUID;
			}

			if (data["Asset Handle"])
				return data["Asset Handle"].as<uint32_t>();
			else
				return NULL_UUID;
		}

		// Try to get the relative path
		std::filesystem::path file_path = std::filesystem::relative(asset_file_path, project_asset_directory);

		// If the relative path starts with "..", the asset is outside the project directory, or if the project directory is just empty
		bool is_relative_to_project = true;

		if (file_path.string().starts_with("..") || project_asset_directory.empty())
			is_relative_to_project = false;

		AssetType type = AssetManager::GetAssetTypeFromFileExtension(asset_file_path.extension());
		AssetHandle handle = GenerateNewAssetHandle(type, is_relative_to_project ? file_path : asset_file_path);

		return handle;
	}

	const std::filesystem::path& EditorAssetManager::GetFilePathFromHandle(const AssetHandle& asset_handle) const
	{
		return GetMetadata(asset_handle).FilePath;
	}

	AssetHandle EditorAssetManager::GenerateNewAssetHandle(const AssetType& asset_type, const std::filesystem::path& asset_file_path)
	{
		AssetHandle handle = Utils::fnv1a_hash(
			AssetUtils::AssetTypeToString(asset_type) + 
			Utils::NormalisePath(asset_file_path).string()
		);

		return handle;
	}

	void EditorAssetManager::SerialiseMetaDataFile(const AssetHandle& asset_handle, const AssetMetaData& asset_meta_data, const std::filesystem::path& meta_data_file_path)
	{

		if (asset_meta_data.ParentAssetHandle != NULL_UUID)
		{
			L_CORE_WARN("Serialise MetaData File: Cannot Serialise Meta Data of Sub Asset.");
			return;
		}

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root

			out << YAML::Key << "Asset Name"			<< YAML::Value << asset_meta_data.AssetName;
			out << YAML::Key << "Asset Handle"			<< YAML::Value << asset_handle;
			out << YAML::Key << "Asset Type"			<< YAML::Value << AssetUtils::AssetTypeToString(asset_meta_data.Type);
			out << YAML::Key << "Asset Is Composite"	<< YAML::Value << asset_meta_data.IsComposite;

			out << YAML::Key << "Asset Import Config"	<< YAML::Value << YAML::BeginMap;
			{
				if (asset_meta_data.ImportConfig)
					out << YAML::Key << "AutoReImport" << YAML::Value << asset_meta_data.ImportConfig->AutoReimport;

				switch (asset_meta_data.Type)
				{

					case AssetType::ModelImport:
					{
						auto import_config = static_pointer_cast<ModelImportConfig>(asset_meta_data.ImportConfig);

						if (!import_config)
							break;

						out << YAML::Key << "Import Position" << YAML::Value << YAML::Flow << YAML::BeginSeq;
						for (int i = 0; i < 3; i++)
							out << import_config->ImportPosition[i];
						out << YAML::EndSeq;

						out << YAML::Key << "Import Rotation" << YAML::Value << YAML::Flow << YAML::BeginSeq;
						for (int i = 0; i < 4; i++)
							out << import_config->ImportRotation[i];
						out << YAML::EndSeq;

						out << YAML::Key << "Import Scale" << YAML::Value << YAML::Flow << YAML::BeginSeq;
						for (int i = 0; i < 3; i++)
							out << import_config->ImportScale[i];
						out << YAML::EndSeq;

						out << YAML::Key << "Import Skeleton"			<< YAML::Value << import_config->ImportSkeleton;
						out << YAML::Key << "Import Animations"			<< YAML::Value << import_config->ImportAnimations;
						out << YAML::Key << "Import Materials"			<< YAML::Value << import_config->ImportMaterials;
					
						break;
					}

				}

			}
			out << YAML::EndMap;

			if(asset_meta_data.IsComposite)
			{
				out << YAML::Key << "Composite Assets" << YAML::Value << YAML::BeginSeq;


				for (auto it = m_AssetRegistry.begin(); it != m_AssetRegistry.end();)
				{
					if (it->second.ParentAssetHandle != asset_handle)
					{
						++it;
						continue;
					}

					// Validate Asset Before Serialising
					if (!IsAssetLoaded(it->first) || (IsAssetLoaded(it->first) && !GetAsset(it->first)))
					{
						it = m_AssetRegistry.erase(it);

						if (IsAssetLoaded(it->first))
							m_LoadedAssets.erase(it->first);

						continue;
					}

					out << YAML::BeginMap;
					out << YAML::Key << "Asset Name" << YAML::Value << it->second.AssetName;
					out << YAML::Key << "Asset Handle" << YAML::Value << it->first;
					out << YAML::Key << "Asset Type" << YAML::Value << AssetUtils::AssetTypeToString(it->second.Type);
					out << YAML::EndMap;

					++it;
				}

				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(meta_data_file_path);
		fout << out.c_str();
	}

	#pragma endregion 

}
