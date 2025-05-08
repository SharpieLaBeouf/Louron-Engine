#include "../Louron Editor Layer.h"
#include "Properties Panel.h"

#include "../Utils/Editor Script Utils.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

using namespace Louron;

void PropertiesPanel::OnImGuiRender(const std::shared_ptr<Scene>& scene_ref, Entity selected_entity, LouronEditorLayer* editor_layer)
{
	std::vector<AssetHandle> material_list;

	ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

	// Right-click on blank space
	if (ImGui::BeginPopupContextWindow())
	{

		// Add Component Section
		{
			if (ImGui::MenuItem("Add Camera Component")) {
				if (!selected_entity.HasComponent<CameraComponent>()) {
					auto& component = selected_entity.AddComponent<CameraComponent>();

					auto& frame_buffer_config = Project::GetActiveScene()->GetSceneFrameBuffer()->GetConfig();
					component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);
				}
			}

			if (ImGui::MenuItem("Add Script Component")) {
				if (!selected_entity.HasComponent<ScriptComponent>())
					selected_entity.AddComponent<ScriptComponent>();
			}

			if (ImGui::MenuItem("Add Skybox Component")) {
				if (!selected_entity.HasComponent<SkyboxComponent>())
					selected_entity.AddComponent<SkyboxComponent>();
			}

			if (ImGui::MenuItem("Add MeshFilter")) {
				if (!selected_entity.HasComponent<MeshFilterComponent>())
					selected_entity.AddComponent<MeshFilterComponent>();
			}

			if (ImGui::MenuItem("Add MeshRenderer")) {
				if (!selected_entity.HasComponent<MeshRendererComponent>())
					selected_entity.AddComponent<MeshRendererComponent>();
			}

			if (ImGui::MenuItem("Add SkinnedMesh")) {
				if (!selected_entity.HasComponent<SkinnedMeshComponent>())
					selected_entity.AddComponent<SkinnedMeshComponent>();
			}

			if (ImGui::MenuItem("Add Animator")) {
				if (!selected_entity.HasComponent<AnimatorComponent>())
					selected_entity.AddComponent<AnimatorComponent>();
			}

			if (ImGui::MenuItem("Add LOD Mesh Component")) {
				if (!selected_entity.HasComponent<LODMeshComponent>())
					selected_entity.AddComponent<LODMeshComponent>();
			}

			if (ImGui::MenuItem("Add Rigidbody")) {
				if (!selected_entity.HasComponent<RigidbodyComponent>())
					selected_entity.AddComponent<RigidbodyComponent>();
			}

			if (ImGui::MenuItem("Add Sphere Collider")) {
				if (!selected_entity.HasComponent<SphereColliderComponent>())
					selected_entity.AddComponent<SphereColliderComponent>();
			}

			if (ImGui::MenuItem("Add Box Collider")) {
				if (!selected_entity.HasComponent<BoxColliderComponent>())
					selected_entity.AddComponent<BoxColliderComponent>();
			}

			if (ImGui::MenuItem("Add Point Light Component")) {
				if (!selected_entity.HasComponent<PointLightComponent>())
					selected_entity.AddComponent<PointLightComponent>();
			}

			if (ImGui::MenuItem("Add Spot Light Component")) {
				if (!selected_entity.HasComponent<SpotLightComponent>())
					selected_entity.AddComponent<SpotLightComponent>();
			}

			if (ImGui::MenuItem("Add Directional Light Component")) {
				if (!selected_entity.HasComponent<DirectionalLightComponent>())
					selected_entity.AddComponent<DirectionalLightComponent>();
			}
		}

		ImGui::EndPopup();
	}

	ImGui::BeginChild("##Immutable Components", {}, ImGuiChildFlags_AutoResizeY);

	float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;

	if (selected_entity.HasComponent<TagComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Columns(2, "entity_properties_cols", false);
		ImGui::SetColumnWidth(-1, ImGui::CalcTextSize("Name").x + 10.0f);

		ImGui::Text("Name");

		ImGui::NextColumn();

		auto& component = selected_entity.GetComponent<TagComponent>();

		char tag_buffer[256];

	#if defined(L_PLATFORM_WINDOWS)
		strncpy_s(tag_buffer, component.Tag.c_str(), sizeof(tag_buffer));
	#else
		strncpy(tag_buffer, component.Tag.c_str(), sizeof(tag_buffer));
	#endif

		tag_buffer[sizeof(tag_buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::InputText("##TagDisplay", tag_buffer, sizeof(tag_buffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			// Update the tag if the input box is deactivated (Enter pressed or box loses focus)
			component.Tag = std::string(tag_buffer);
		}

		ImGui::NextColumn();

		ImGui::Text("ID");

		ImGui::NextColumn();

		char id_buffer[11];

	#if defined(L_PLATFORM_WINDOWS)
		strncpy_s(id_buffer, std::to_string(selected_entity.GetUUID()).c_str(), sizeof(id_buffer));
	#else
		strncpy(id_buffer, std::to_string(selected_entity.GetUUID()).c_str(), sizeof(id_buffer));
	#endif

		id_buffer[sizeof(id_buffer) - 1] = '\0'; // Ensure null-termination

		ImGui::InputText("##IDDisplay", id_buffer, sizeof(id_buffer), ImGuiInputTextFlags_ReadOnly);

		ImGui::Columns(1);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<TransformComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::Text("Transform Component\n ");
		TransformComponent& entity_transform = selected_entity.GetTransform();

		ImGui::Columns(2, "transform_columns", false);

		ImGui::SetColumnWidth(-1, glm::min(ImGui::CalcTextSize("Position").x + ImGui::GetStyle().ItemSpacing.x * 2, first_coloumn_width));

		ImGui::Text("Position");
		ImGui::NextColumn();

		// Get the width of the current column
		float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

		glm::vec3 value = entity_transform.GetLocalPosition();
		bool updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		std::string label = "##Local PositionX" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local PositionY" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local PositionZ" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetPosition(value);

		ImGui::NextColumn();
		ImGui::Text("Rotation");
		ImGui::NextColumn();

		value = entity_transform.GetLocalRotation();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local RotationX" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local RotationY" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local RotationZ" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetRotation(value);

		ImGui::NextColumn();
		ImGui::Text("Scale");
		ImGui::NextColumn();

		value = entity_transform.GetLocalScale();
		updated = false;

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local ScaleX" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.x, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local ScaleY" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.y, 0.1f, 0, 0, "%.2f")) updated = true;

		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
		label = "##Local ScaleZ" + std::to_string(selected_entity.GetUUID());
		if (ImGui::DragFloat(label.c_str(), &value.z, 0.1f, 0, 0, "%.2f")) updated = true;

		if (updated) entity_transform.SetScale(value);

		ImGui::NextColumn();

		ImGui::Columns(1);

		ImGui::Dummy({ 0.0f, 5.0f });
	}

	ImGui::EndChild();

	ImGui::Separator();

	ImGui::BeginChild("##Mutable Components", {}, ImGuiChildFlags_AutoResizeY);

	if (selected_entity.HasComponent<ScriptComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		bool selected = false;

		ImGui::BeginChild("##ScriptComponentChild", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Script Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<ScriptComponent>();
			std::vector<std::pair<std::string, bool>>& component_script_vector = component.Scripts;

			int index = 0;
			for (auto it = component_script_vector.begin(); it != component_script_vector.end(); ) {

				bool& active = it->second;
				std::string& script_name = it->first;

				std::string label = "Script##" + std::to_string(index);

				if (ImGui::TreeNodeEx(label.c_str(), tree_node_flags | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {

					ImGui::Columns(2, "script_columns", false);
					ImGui::SetColumnWidth(-1, ImGui::CalcTextSize("Script").x + 10.0f);

					ImGui::Text("Active");
					ImGui::NextColumn();
					ImGui::Checkbox("##ActiveCheckBox", &active);
					ImGui::NextColumn();

					ImGui::Text("Script");
					ImGui::NextColumn();

					const auto& script_classes = ScriptManager::Get()->GetAllClasses();
					std::vector<const char*> available_scripts;
					available_scripts.reserve(script_classes.size() + 1);
					available_scripts.push_back(" ");

					int i = 0;
					int current_item = -1;
					for (const auto& script : script_classes) {

						available_scripts.push_back(script.first.c_str());

						if (script.first == script_name)
							current_item = i + 1;

						i++;
					}

					std::string combo_label = "##ScriptCombo_" + script_name;
					if (ImGui::BeginCombo(combo_label.c_str(), current_item != -1 ? available_scripts.data()[current_item] : "")) {

						for (int n = 0; n < available_scripts.size(); n++)
						{
							const bool is_selected = (current_item == n);
							if (ImGui::Selectable(available_scripts[n], is_selected))
							{
								current_item = n;
								script_name = available_scripts[current_item];
							}

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					ImGui::Columns(1);

					// Ensure that the script class is valid
					// Check if has fields, if no fields, we won't display anything for fields
					auto script_class = script_classes.find(script_name);
					if (script_class != script_classes.end() && script_class->second && script_class->second && script_class->second->field_count > 0) 
					{
						ImGui::Dummy({ 0.0f, 5.0f });
						ImGui::SeparatorText("Fields");
						ImGui::Dummy({ 0.0f, 5.0f });

						// SCRIPT FIELDS
						DisplayScriptFields(script_name, selected_entity);

						ImGui::Dummy({ 0.0f, 5.0f });
						ImGui::SeparatorText("");
					}

					ImGui::Dummy({ 0.0f, 2.5f });

					ImVec2 available = ImGui::GetContentRegionAvail();
					float button_width = ImGui::CalcTextSize("Create New Script").x + ImGui::GetStyle().FramePadding.x * 2.0f;
					if (available.x > button_width) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available.x - button_width) * 0.5f);

					if (ImGui::Button("Remove Script")) {
						it = component_script_vector.erase(it);
						ImGui::TreePop();
						continue;
					}

					ImGui::Dummy({ 0.0f, 2.5f });

					ImGui::TreePop();
				}

				++it;
				index++;
			}

			ImGui::Dummy({ 0.0f, 10.0f });

			ImGui::Columns(2, "script_columns_buttons", false);

			static bool script_create = false;

			if (scene_ref->IsRunning()) ImGui::BeginDisabled();
			if (ImGui::Button("Create New Script") || script_create) {
				script_create = true;
			}
			if (scene_ref->IsRunning()) ImGui::EndDisabled();

			ImGui::NextColumn();

			if (ImGui::Button("Add Existing Script")) {
				component_script_vector.push_back({ "", true });
			}

			ImGui::Columns(1);

			auto script_full_name = ::Utils::OnCreateNewScriptGUI(&script_create);
			if (!script_full_name.empty())
				component_script_vector.push_back({ script_full_name, true });

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<ScriptComponent>("Script Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<CameraComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		bool selected = false;

		ImGui::BeginChild("##CamerComponentChild", {}, ImGuiChildFlags_AutoResizeY);


		if (ImGui::TreeNodeEx(("Camera Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<CameraComponent>();

			ImGui::Columns(2, "camera_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);


			std::array<const char*, 2> camera_projection_types = { "Perspective", "Orthographic" };
			uint8_t item_current = static_cast<uint8_t>(component.CameraInstance->GetProjectionType());
			ImGui::Text("Projection Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##CameraProjectTypes", camera_projection_types[item_current])) {

				for (int n = 0; n < camera_projection_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(camera_projection_types[n], is_selected))
					{
						item_current = n;
						component.CameraInstance->SetProjectionType(static_cast<SceneCamera::ProjectionType>(item_current));
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Text("FOV");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			auto& frame_buffer_config = scene_ref->GetSceneFrameBuffer()->GetConfig();
			float data = glm::degrees(component.CameraInstance->GetPerspectiveVerticalFOV());
			if (ImGui::DragFloat("##CameraFOV", &data, 0.05f, 0.1f, FLT_MAX, "%.2f"))
			{
				component.CameraInstance->SetPerspectiveVerticalFOV(glm::radians(data));
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);

				if(component.CameraFramebuffer)
					component.CameraFramebuffer->Resize({ frame_buffer_config.Width, frame_buffer_config.Height });
			}
			ImGui::NextColumn();

			ImGui::Text("Near");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			data = component.CameraInstance->GetPerspectiveNearClip();
			if (ImGui::DragFloat("##CameraNear", &data, 0.05f, 0.1f, FLT_MAX, "%.2f"))
			{
				component.CameraInstance->SetPerspectiveNearClip(data);
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);

				if (component.CameraFramebuffer)
					component.CameraFramebuffer->Resize({ frame_buffer_config.Width, frame_buffer_config.Height });
			}
			ImGui::NextColumn();

			ImGui::Text("Far");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			data = component.CameraInstance->GetPerspectiveFarClip();
			if (ImGui::DragFloat("##CameraFar", &data, 0.05f, 0.1f, FLT_MAX, "%.2f"))
			{
				component.CameraInstance->SetPerspectiveFarClip(data);
				component.CameraInstance->SetViewportSize(frame_buffer_config.Width, frame_buffer_config.Height);

				if (component.CameraFramebuffer)
					component.CameraFramebuffer->Resize({ frame_buffer_config.Width, frame_buffer_config.Height });
			}
			ImGui::NextColumn();

			ImGui::Text("Primary");
			ImGui::NextColumn();
			ImGui::Checkbox("##PrimaryCheckBox", &component.Primary);
			ImGui::NextColumn();

			std::array<const char*, 2> camera_clear_types = { "Colour Only", "Skybox" };
			item_current = static_cast<uint8_t>(component.ClearFlags);
			ImGui::Text("Clear Flag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##CameraClearColour", camera_clear_types[item_current])) {

				for (int n = 0; n < camera_clear_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(camera_clear_types[n], is_selected))
					{
						item_current = n;
						component.ClearFlags = static_cast<CameraClearFlags>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Text("Clear Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##SpotLightColour", glm::value_ptr(component.ClearColour));
			ImGui::NextColumn();

			ImGui::Text("Camera Depth");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);

			// Slider for Position
			int temp_depth = component.CameraDepth;
			if (ImGui::DragInt("##Camera Depth", &temp_depth, 0.5f, 0, UINT8_MAX))
			{
				component.CameraDepth = static_cast<uint8_t>(temp_depth);
			}
			ImGui::NextColumn();

			glm::vec4 temp_viewport = component.GetViewport();
			glm::vec2 temp_viewport_pos = { temp_viewport.x, temp_viewport.y };
			glm::vec2 temp_viewport_size = { temp_viewport.z, temp_viewport.w };

			ImGui::Text("Viewport Position");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);

			// Slider for Position
			if (ImGui::DragFloat2("##Viewport Position", glm::value_ptr(temp_viewport_pos), 0.001f, -1.0f, 1.0f))
			{
				temp_viewport.x = temp_viewport_pos.x;
				temp_viewport.y = temp_viewport_pos.y;
			}
			ImGui::NextColumn();

			ImGui::Text("Viewport Size");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);

			// Slider for Size
			if (ImGui::DragFloat2("##Viewport Size", glm::value_ptr(temp_viewport_size), 0.001f, 0.001f, 1.0f))
			{
				temp_viewport.z = temp_viewport_size.x;
				temp_viewport.w = temp_viewport_size.y;
			}

			// Update Changes
			if (temp_viewport != component.GetViewport())
			{
				component.SetViewport(temp_viewport, editor_layer->m_ViewportWindowSize);
			}

			ImGui::NextColumn();

			ImGui::Text("Display to Final ViewPort");
			ImGui::NextColumn();
			ImGui::Checkbox("##Display to Final ViewPort", &component.DisplayToMainViewport);
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<CameraComponent>("Camera Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SkyboxComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Skybox Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Skybox Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SkyboxComponent>();
			material_list.push_back(component.SkyboxMaterialAssetHandle);

			std::string skybox_material_name;
			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			if (component.SkyboxMaterialAssetHandle != NULL_UUID && Project::GetStaticEditorAssetManager()->IsAssetHandleValid(component.SkyboxMaterialAssetHandle)) {
				skybox_material_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.SkyboxMaterialAssetHandle).AssetName;
			}
			else if (component.SkyboxMaterialAssetHandle != NULL_UUID) {
				skybox_material_name = "Asset Handle Invalid: " + std::to_string(component.SkyboxMaterialAssetHandle);
				text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
			}
			else {
				skybox_material_name = "None";
			}

			char asset_name_buf[256];
			
		#if defined(L_PLATFORM_WINDOWS)
			strncpy_s(asset_name_buf, skybox_material_name.c_str(), sizeof(asset_name_buf));
		#else
			strncpy(asset_name_buf, skybox_material_name.c_str(), sizeof(asset_name_buf));
		#endif

			asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

			ImGui::Columns(2, "skybox_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);
			ImGui::Text("Material:");
			ImGui::NextColumn();

			// Calculate the width of the text labels and buttons
			float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
			float closeButtonWidth = ImGui::CalcTextSize(" X ").x + ImGui::GetStyle().FramePadding.x * 2;
			float spacing = ImGui::GetStyle().ItemSpacing.x;

			// Calculate the available width for the InputText
			float availableWidth = ImGui::GetColumnWidth() - buttonWidth - closeButtonWidth - spacing * 4;

			ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
			ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

			ImGui::BeginDisabled(true);

			if (availableWidth > ImGui::CalcTextSize(skybox_material_name.c_str()).x)
				ImGui::SetNextItemWidth(availableWidth);
			else
				ImGui::SetNextItemWidth(-1);

			ImGui::InputText("##SkyboxAssetMaterial", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();

			// Drag target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
					AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

					if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Skybox) {
						component.SkyboxMaterialAssetHandle = dropped_asset_handle;
					}
					else {
						L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
					}
				}

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

					std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
					std::filesystem::path dropped_asset_path = dropped_asset_path_string;

					if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

						AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());

						if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Skybox) {
							component.SkyboxMaterialAssetHandle = dropped_asset_handle;
						}
						else {
							L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
						}
					}
					else {
						L_APP_WARN("Invalid File Path Dropped on Skybox Material Target.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (availableWidth > ImGui::CalcTextSize(skybox_material_name.c_str()).x)
				ImGui::SameLine();

			if (ImGui::Button("...")) {
				L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR SKYBOX MATERIAL!");
			}

			ImGui::SameLine();
			if (ImGui::Button(" X ")) {
				component.SkyboxMaterialAssetHandle = NULL_UUID;
			}

			ImGui::NextColumn();
			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SkyboxComponent>("Skybox Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<LODMeshComponent>())
	{
		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##LODMeshComponent Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("LOD Mesh Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<LODMeshComponent>();

			std::vector<float> lod_ranges;

			for (const auto& element : component.LOD_Elements)
				lod_ranges.push_back(element.DistanceThresholdNormalised);

			ImGui::Dummy({ 0.0f, 5.0f });
			::Utils::GUI::MultiRangeLODSliderFloat("LODMeshComponentSlider", lod_ranges, 0.0f, 1.0f, 0.01f);

			ImGui::Checkbox("Prefer Max Distance Over Far Plane", &component.MaxDistanceOverFarPlane);

			if(component.MaxDistanceOverFarPlane)
			{
				ImGui::Text("Max Distance");
				ImGui::SameLine();

				float value = component.MaxDistance;
				if (ImGui::InputFloat("##LODMeshComponentMaxDistance", &value, 1.0f, 0.0f, "%.2f") && value > 0.0f)
					component.MaxDistance = value;
			}

			ImGui::Dummy({ 0.0f, 2.5f });
			ImGui::SeparatorText("LOD Levels");
			ImGui::Dummy({ 0.0f, 2.5f });

			for (int i = 0; i < component.LOD_Elements.size(); i++)
			{
				component.LOD_Elements[i].DistanceThresholdNormalised = lod_ranges[i];

				std::string label = "LOD Level " + std::to_string(i+1);
				if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
				{

					ImGui::Text("Threshold Distance: %.4f", component.LOD_Elements[i].DistanceThresholdNormalised);
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
						ImGui::SetTooltip("This is the normalised threshold distance between the camera position to the far plane. \n\nFor Example: If the far plane of the camera is 1000, and the threshold is 0.10, this LOD level will pop when the distance to the camera is 100 units away.", ImGui::GetStyle().HoverDelayNormal);

					ImGui::SeparatorText("Entities");

					for (int j = 0; j < component.LOD_Elements[i].MeshRendererEntities.size(); j++)
					{
						Entity entity = scene_ref->HasEntityByUUID(component.LOD_Elements[i].MeshRendererEntities[j]) ? scene_ref->FindEntityByUUID(component.LOD_Elements[i].MeshRendererEntities[j]) : Entity{};
						std::string entity_name = entity ? entity.GetName() + " (Mesh Renderer)" : component.LOD_Elements[i].MeshRendererEntities[j] == NULL_UUID ? "(None)" : std::to_string(component.LOD_Elements[i].MeshRendererEntities[j]) + " (Invalid Entity Handle)";

						char buffer[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(buffer, entity_name.c_str(), sizeof(buffer));
					#else
						strncpy(buffer, entity_name.c_str(), sizeof(buffer));
					#endif

						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						ImGui::InputText(("##" + entity_name + std::to_string(j)).c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

						// Drag target
						if (ImGui::BeginDragDropTarget()) {

							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID")) {
								Louron::UUID droppedEntityUUID = *(const Louron::UUID*)payload->Data;
								Entity droppedEntity = component.GetComponent<HierarchyComponent>().FindChild(droppedEntityUUID);

								if (droppedEntity)
								{
									if (droppedEntity.HasComponent<MeshFilterComponent>() && droppedEntity.HasComponent<MeshRendererComponent>())
									{
										component.LOD_Elements[i].MeshRendererEntities[j] = droppedEntity.GetUUID();
									}
									else
									{
										L_APP_INFO("Cannot Assign Entity to LOD Element that does not have MeshFilter and MeshRenderer!");
									}

								}
								else
								{
									L_APP_INFO("Cannot Assign Entity to LOD Element that is not a child!");
								}
							}

							ImGui::EndDragDropTarget();
						}

					}

					ImGui::Text("Add/Remove Entity");
					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::CalcTextSize(" + ").x + ImGui::GetStyle().ItemSpacing.x) * 1.5f);

					if (ImGui::SmallButton(" + "))
					{
						// Add a new entity (NULL_UUID by default)
						component.LOD_Elements[i].MeshRendererEntities.push_back(NULL_UUID);
					}

					ImGui::SameLine();

					if (ImGui::SmallButton(" - "))
					{
						if(component.LOD_Elements[i].MeshRendererEntities.size() > 1)
							component.LOD_Elements[i].MeshRendererEntities.pop_back();
					}

					ImGui::TreePop();
				}

			}

			ImGui::Dummy({ 0.0f, 5.0f });

			ImGui::Separator();

			ImGui::Dummy({ 0.0f, 2.5f });

			// Centered Button Row
			ImGui::Text("Add/Remove Level");
			ImGui::SameLine();

			// Centered Button Row
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::CalcTextSize(" + ").x + ImGui::GetStyle().ItemSpacing.x) * 1.5f);

			if (ImGui::SmallButton(" + "))
			{
				float distance = 1.0f;

				// Ensure distance values shift down if necessary
				for (int k = (int)component.LOD_Elements.size() - 1; k >= 0; k--)
				{
					if (component.LOD_Elements[k].DistanceThresholdNormalised >= distance)
					{
						distance -= 0.01f;
						component.LOD_Elements[k].DistanceThresholdNormalised = distance;
					}
					else
					{
						break;
					}
				}

				component.LOD_Elements.push_back({ 1.0f, {} });
			}

			ImGui::SameLine();

			if (ImGui::SmallButton(" - "))
			{
				if (component.LOD_Elements.size() > 1)
					component.LOD_Elements.pop_back();
			}

			ImGui::TreePop();
		}



		ImGui::EndChild();

		ShowComponentContextPopup<LODMeshComponent>("LOD Mesh Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();

	}

	if (selected_entity.HasComponent<SkinnedMeshComponent>())
	{
		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Skinned Mesh Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Skinned Mesh Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {
			auto& component = selected_entity.GetComponent<SkinnedMeshComponent>();

			std::string asset_mesh_name;
			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(component.StaticMeshHandle)) 
			{
				asset_mesh_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.StaticMeshHandle).AssetName;
			}
			else if (component.StaticMeshHandle != NULL_UUID) 
			{
				asset_mesh_name = "Asset Handle Invalid: " + std::to_string(component.StaticMeshHandle);
				text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
			}
			else 
			{
				asset_mesh_name = "None";
			}

			char asset_name_buf[256];

			#if defined(L_PLATFORM_WINDOWS)
				strncpy_s(asset_name_buf, asset_mesh_name.c_str(), sizeof(asset_name_buf));
			#else
				strncpy(asset_name_buf, asset_mesh_name.c_str(), sizeof(asset_name_buf));
			#endif

			asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

			ImGui::Columns(2, "skinned_mesh_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);
			ImGui::Text("Skinned Mesh Filter");
			ImGui::NextColumn();

			ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
			ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

			ImGui::BeginDisabled(true);
			{

				float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
				float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
				ImGui::PushItemWidth(availableWidth);

				ImGui::InputText("##SkinnedMeshFilterName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

				ImGui::PopItemWidth();
			}
			ImGui::EndDisabled();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			// Drag target
			if (ImGui::BeginDragDropTarget()) 
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) 
				{
					AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

					if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Mesh) {
						component.StaticMeshHandle = dropped_asset_handle;
						component.AABBNeedsUpdate = true;
						component.OctreeNeedsUpdate = true;
						AssetManager::GetAsset<StaticMesh>(component.StaticMeshHandle); // Force load the Asset on the main thread/GL context
					}
					else {
						L_APP_WARN("Invalid Asset Type Dropped on Skinned Mesh Filter Target.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button("...")) {
				L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MESHES!");
			}

			ImGui::NextColumn();

			ImGui::Text("Show Debug AABB");
			ImGui::NextColumn();
			ImGui::Checkbox("##DebugAABBCheckBox", &component.DisplayDebugAABB);

			ImGui::Columns(1);

			ImGui::Dummy({ 0.0f, 2.5f });
			ImGui::Separator();
			ImGui::Dummy({ 0.0f, 2.5f });

			if (ImGui::TreeNodeEx("Materials", tree_node_flags)) {

				ImGui::Dummy({ 0.0f, 5.0f });

				int i = 0;
				ImGui::Columns(2, "skinned_mesh_material_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);

				for (auto& pair : component.MaterialHandles) {

					std::string material_name;
					ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

					if (pair.first != NULL_UUID && AssetManager::IsAssetHandleValid(pair.first)) {
						material_name = Project::GetStaticEditorAssetManager()->GetMetadata(pair.first).AssetName;
					}
					else if (pair.first != NULL_UUID) {
						material_name = "Invalid Asset: " + std::to_string(pair.first);
						text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
					}
					else {
						material_name = "None";
					}

					char asset_name_buf[256];

				#if defined(L_PLATFORM_WINDOWS)
					strncpy_s(asset_name_buf, material_name.c_str(), sizeof(asset_name_buf));
				#else
					strncpy(asset_name_buf, material_name.c_str(), sizeof(asset_name_buf));
				#endif

					asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

					ImGui::Text("Element %i: ", i);

					ImGui::NextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						ImGui::InputText("##MaterialName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
						{
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
								pair.first = dropped_asset_handle;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skinned Mesh Material Target.");
							}
						}

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

							std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_asset_path = dropped_asset_path_string;

							if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
									pair.first = dropped_asset_handle;
								}
								else {
									L_APP_WARN("Invalid Asset Type Dropped on Skinned Mesh Material Target.");
								}
							}
							else {
								L_APP_WARN("Invalid File Path Dropped on Skinned Mesh Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("...")) {
						L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MATERIALS!");
					}

					ImGui::NextColumn();
					i++;
				}

				ImGui::Columns(1);

				// Centered Button Row
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::CalcTextSize(" + ").x + ImGui::GetStyle().ItemSpacing.x) * 1.5f);

				if (ImGui::SmallButton(" + "))
				{
					// Add a new entity (NULL_UUID by default)
					component.MaterialHandles.push_back({ NULL_UUID, nullptr });
				}

				ImGui::SameLine();

				if (ImGui::SmallButton(" - "))
				{
					if (component.MaterialHandles.size() > 1)
						component.MaterialHandles.pop_back();
				}

				ImGui::TreePop();
			}

			std::shared_ptr<Skeleton> asset_skeleton;
			if (AssetManager::IsAssetLoaded(component.SkeletonHandle))
				asset_skeleton = AssetManager::GetAsset<Skeleton>(component.SkeletonHandle);

			std::string label = "Skeleton";
			if (!asset_skeleton) 
			{
				label += " - Invalid Skeleton";
				ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.0f, 0.0f, 1.0f});
			}

			if (ImGui::TreeNodeEx(label.c_str(), tree_node_flags)) {

				ImGui::Dummy({ 0.0f, 5.0f });

				int i = 0;
				ImGui::Columns(2, "skinned_mesh_bone_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);

				std::function<void(BoneLayout&, int)> draw_bone_hierarchy = [&](BoneLayout& bone, int bone_level)
				{
					std::string text_label;
					for (int i = 0; i < bone_level; i++)
					{
						text_label += " ";
					}
					text_label += bone.BoneName;
					ImGui::Text(text_label.c_str());

					ImGui::NextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						std::string label = "##BoneReference" + bone.BoneName;

						Entity bone_entity = scene_ref->FindEntityByUUID(component.SkeletonBoneMapping[bone.BoneID]);
						std::string entity_name = bone_entity ? bone_entity.GetName() : "Invalid Entity";

						char entity_name_buf[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(entity_name_buf, entity_name.c_str(), sizeof(entity_name_buf));
					#else
						strncpy(entity_name_buf, entity_name.c_str(), sizeof(entity_name_buf));
					#endif

						entity_name_buf[sizeof(entity_name_buf) - 1] = '\0'; // Ensure null-termination

						ImGui::InputText(label.c_str(), entity_name_buf, sizeof(entity_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropTarget()) 
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_UUID"))
						{
							Louron::UUID dropped_uuid = *(const Louron::UUID*)payload->Data;

							component.SkeletonBoneMapping[bone.BoneID] = dropped_uuid;

						}

						ImGui::EndDragDropTarget();
					}

					ImGui::NextColumn();

					for (auto& child_bone : bone.BoneChildren)
					{
						draw_bone_hierarchy(child_bone, bone_level + 1);
					}
				};

				std::string skeleton_asset_name;

				if (component.SkeletonHandle == NULL_UUID)
				{
					skeleton_asset_name = "None (Skeleton)";
				}
				else if (!AssetManager::IsAssetLoaded(component.SkeletonHandle))
				{
					skeleton_asset_name = std::to_string(component.SkeletonHandle) + " (Invalid Handle)";
				}
				else
				{
					skeleton_asset_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.SkeletonHandle).AssetName;
				}

				ImGui::Text("Skeleton");

				ImGui::NextColumn();

				// Display Skeleton Asset Name
				{
					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						std::string label = "##SkeletonReference" + skeleton_asset_name;

						char skeleton_asset_name_buf[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(skeleton_asset_name_buf, skeleton_asset_name.c_str(), sizeof(skeleton_asset_name_buf));
					#else
						strncpy(skeleton_asset_name_buf, skeleton_asset_name.c_str(), sizeof(skeleton_asset_name_buf));
					#endif

						skeleton_asset_name_buf[sizeof(skeleton_asset_name_buf) - 1] = '\0'; // Ensure null-termination

						ImGui::InputText(label.c_str(), skeleton_asset_name_buf, sizeof(skeleton_asset_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					// Drag target
					if (ImGui::BeginDragDropTarget()) 
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
						{
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Skeleton) {
								component.SkeletonHandle = dropped_asset_handle;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skeleton Asset Target.");
							}
						}

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

							std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_asset_path = dropped_asset_path_string;

							if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) 
							{
								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::ModelImport) 
								{
									const auto& asset_reg = Project::GetStaticEditorAssetManager()->GetAssetRegistry();
									for (const auto& [handle, meta_data] : asset_reg)
									{
										if (meta_data.ParentAssetHandle == dropped_asset_handle && meta_data.Type == AssetType::Skeleton)
										{
											component.SkeletonHandle = handle;
										}
									}
								}
								else 
								{
									L_APP_WARN("Invalid Asset Type Dropped on Skinned Mesh Material Target.");
								}
							}
							else 
							{
								L_APP_WARN("Invalid File Path Dropped on Skinned Mesh Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("...")) {
						L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR SKELETONS!");
					}

				}

				ImGui::NextColumn();

				if (asset_skeleton)
				{
					ImGui::Dummy({ 0.0f, 2.5f });
					ImGui::Separator();
					ImGui::Dummy({ 0.0f, 2.5f });

					draw_bone_hierarchy(asset_skeleton->SkeletonLayout, 0);
				}

				ImGui::TreePop();
			}

			if (!asset_skeleton)
				ImGui::PopStyleColor();

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SkinnedMeshComponent>("Skinned Mesh Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<AnimatorComponent>())
	{
		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::BeginChild("##Animator Component Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Animator Component##" + selected_entity.GetName()).c_str(), tree_node_flags))
		{
			ImGui::Indent();
			auto& component = selected_entity.GetComponent<AnimatorComponent>();

			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			ImGui::Text("Current Animation:");

			std::vector<const char*> animation_names;
			for (const auto& handle : component.AnimationClipHandles)
			{
				if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(handle))
				{
					animation_names.push_back(Project::GetStaticEditorAssetManager()->GetMetadata(handle).AssetName.c_str());
				}
				else if (handle != NULL_UUID)
				{
					animation_names.push_back("Invalid Animation");
				}
				else
				{
					animation_names.push_back("None (Animation)");
				}
			}

			ImGui::Combo("Current Animation", &component.CurrentClipIndex, animation_names.data(), static_cast<int>(animation_names.size()));

			ImGui::Checkbox("Auto Play Animation", &component.IsPlaying);

			ImGui::Checkbox("Looping", &component.IsLooping);

			ImGui::DragFloat("Playback Speed", &component.PlaybackSpeed, 0.01f, 0.1f, 10.0f, "%.2f");

			float max_duration = 1.0f;

			if (component.CurrentClipIndex >= 0 && component.CurrentClipIndex < component.AnimationClipHandles.size() && Project::GetStaticEditorAssetManager()->IsAssetLoaded(component.AnimationClipHandles[component.CurrentClipIndex]))
			{
				max_duration = AssetManager::GetAsset<AnimationClip>(component.AnimationClipHandles[component.CurrentClipIndex]) ->GetDuration();
			}

			ImGui::SliderFloat("Current Time", &component.CurrentTime, 0.0f, max_duration, "%.2f");

			std::array<const char*, 3> culling_modes = { "Always Animate", "Only Step Animation Timer", "No Animate" };
			uint8_t item_current = static_cast<uint8_t>(component.CullingMode);
			if (ImGui::BeginCombo("Animation When Culled", culling_modes[item_current])) {

				for (int n = 0; n < culling_modes.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(culling_modes[n], is_selected))
					{
						item_current = n;
						component.CullingMode = static_cast<AnimatorComponent::AnimationCullingMode>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Dummy({ 0.0f, 2.5f });
			ImGui::Separator();
			ImGui::Dummy({ 0.0f, 2.5f });

			if (ImGui::TreeNodeEx(("Animations##" + selected_entity.GetName()).c_str(), tree_node_flags)) {
				
				ImGui::Columns(2, "animations_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);

				for (int i = 0; i < animation_names.size(); i++)
				{
					ImGui::Text(("Animation " + std::to_string(i)).c_str());

					ImGui::NextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						char entity_name_buf[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(entity_name_buf, animation_names[i], sizeof(entity_name_buf));
					#else
						strncpy(entity_name_buf, animation_names[i], sizeof(entity_name_buf));
					#endif

						entity_name_buf[sizeof(entity_name_buf) - 1] = '\0'; // Ensure null-termination

						std::string label = "##AnimationName" + std::string(animation_names[i]);
						ImGui::InputText(label.c_str(), entity_name_buf, sizeof(entity_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE"))
						{
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::AnimationClip) {
								
								component.AnimationClipHandles[i] = dropped_asset_handle;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Animation Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("...")) {
						L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR ANIMATIONS!");
					}

					ImGui::NextColumn();
				}

				ImGui::Columns(1);

				// Centered Button Row
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::CalcTextSize(" + ").x + ImGui::GetStyle().ItemSpacing.x) * 1.5f);

				if (ImGui::SmallButton(" + "))
				{
					// Add a new entity (NULL_UUID by default)
					component.AnimationClipHandles.emplace_back(NULL_UUID);
				}

				ImGui::SameLine();

				if (ImGui::SmallButton(" - "))
				{
					if (component.AnimationClipHandles.size() > 1)
						component.AnimationClipHandles.pop_back();
				}

				ImGui::TreePop();
			}


			ImGui::Unindent();
			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<AnimatorComponent>("Animator Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<MeshFilterComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Mesh Filter Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Mesh Filter Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {
			auto& component = selected_entity.GetComponent<MeshFilterComponent>();

			std::string mesh_filter_name;
			ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

			if (component.StaticMeshHandle != NULL_UUID && Project::GetStaticEditorAssetManager()->IsAssetHandleValid(component.StaticMeshHandle)) {
				mesh_filter_name = Project::GetStaticEditorAssetManager()->GetMetadata(component.StaticMeshHandle).AssetName;
			}
			else if (component.StaticMeshHandle != NULL_UUID) {
				mesh_filter_name = "Asset Handle Invalid: " + std::to_string(component.StaticMeshHandle);
				text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
			}
			else {
				mesh_filter_name = "None";
			}

			char asset_name_buf[256];

		#if defined(L_PLATFORM_WINDOWS)
			strncpy_s(asset_name_buf, mesh_filter_name.c_str(), sizeof(asset_name_buf));
		#else
			strncpy(asset_name_buf, mesh_filter_name.c_str(), sizeof(asset_name_buf));
		#endif

			asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

			ImGui::Columns(2, "mesh_filter_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);
			ImGui::Text("Mesh Filter");
			ImGui::NextColumn();

			ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
			ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

			ImGui::BeginDisabled(true);
			{

				float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
				float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
				ImGui::PushItemWidth(availableWidth);

				ImGui::InputText("##MeshFilterName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

				ImGui::PopItemWidth();
			}
			ImGui::EndDisabled();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			// Drag target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
					AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

					if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Mesh) {
						component.StaticMeshHandle = dropped_asset_handle;
						component.AABBNeedsUpdate = true;
						component.OctreeNeedsUpdate = true;
						AssetManager::GetAsset<StaticMesh>(component.StaticMeshHandle); // Force load the Asset on the main thread/GL context
					}
					else {
						L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			if (ImGui::Button("...")) {
				L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MESHES!");
			}

			ImGui::NextColumn();

			ImGui::Text("Show Debug AABB");
			ImGui::NextColumn();
			bool show_debug = component.GetShouldDisplayDebugLines();
			if (ImGui::Checkbox("##DebugAABBCheckBox", &show_debug))
				component.SetShouldDisplayDebugLines(show_debug);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<MeshFilterComponent>("Mesh Filter Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<MeshRendererComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Mesh Renderer Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Mesh Renderer Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<MeshRendererComponent>();

			if (component.MaterialHandles.empty()) {
				component.MaterialHandles.push_back({ NULL_UUID, nullptr });
			}

			for (const auto& pair : component.MaterialHandles) {
				if (pair.first != NULL_UUID) {
					material_list.push_back(pair.first);
				}
			}

			ImGui::Columns(2, "mesh_renderer_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");

			ImGui::NextColumn();

			ImGui::Checkbox("##MeshRendererActive", &component.Active);

			ImGui::NextColumn();

			ImGui::Text("Cast Shadows");

			ImGui::NextColumn();

			ImGui::Checkbox("##MeshRendererCastShadows", &component.CastShadows);

			ImGui::NextColumn();

			ImGui::Columns(1);

			if (ImGui::TreeNodeEx("Materials", tree_node_flags)) {

				ImGui::Dummy({ 0.0f, 5.0f });

				int i = 0;
				ImGui::Columns(2, "mesh_renderer_material_columns", false);
				ImGui::SetColumnWidth(-1, first_coloumn_width);

				for (auto& pair : component.MaterialHandles) {

					std::string material_name;
					ImVec4 text_colour = ImGui::GetStyleColorVec4(ImGuiCol_Text);

					if (pair.first != NULL_UUID && AssetManager::IsAssetHandleValid(pair.first)) {
						material_name = Project::GetStaticEditorAssetManager()->GetMetadata(pair.first).AssetName;
					}
					else if (pair.first != NULL_UUID) {
						material_name = "Invalid Asset: " + std::to_string(pair.first);
						text_colour = { 1.0f, 0.35f, 0.35f, 1.0f };
					}
					else {
						material_name = "None";
					}

					char asset_name_buf[256];

				#if defined(L_PLATFORM_WINDOWS)
					strncpy_s(asset_name_buf, material_name.c_str(), sizeof(asset_name_buf));
				#else
					strncpy(asset_name_buf, material_name.c_str(), sizeof(asset_name_buf));
				#endif

					asset_name_buf[sizeof(asset_name_buf) - 1] = '\0'; // Ensure null-termination

					ImGui::Text("Element %i: ", i);

					ImGui::NextColumn();

					ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
					ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);

					ImGui::BeginDisabled(true);
					{
						float buttonWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2;
						float availableWidth = ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x;
						ImGui::PushItemWidth(availableWidth);

						ImGui::InputText("##MaterialName", asset_name_buf, sizeof(asset_name_buf), ImGuiInputTextFlags_ReadOnly);

						ImGui::PopItemWidth();
					}

					ImGui::EndDisabled();

					ImGui::PopStyleVar();
					ImGui::PopStyleColor();

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) 
						{
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
								pair.first = dropped_asset_handle;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Mesh Renderer Material Target.");
							}
						}

						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

							std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_asset_path = dropped_asset_path_string;

							if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Material_Standard) {
									pair.first = dropped_asset_handle;
								}
								else {
									L_APP_WARN("Invalid Asset Type Dropped on Mesh Renderer Material Target.");
								}
							}
							else {
								L_APP_WARN("Invalid File Path Dropped on Mesh Renderer Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("...")) {
						L_APP_INFO("Lets Implement Opening an Asset Directory Window - FOR MATERIALS!");
					}

					ImGui::NextColumn();
					i++;
				}

				ImGui::Columns(1);

				// Centered Button Row
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - (ImGui::CalcTextSize(" + ").x + ImGui::GetStyle().ItemSpacing.x) * 1.5f);

				if (ImGui::SmallButton(" + "))
				{
					// Add a new entity (NULL_UUID by default)
					component.MaterialHandles.push_back({ NULL_UUID, nullptr });
				}

				ImGui::SameLine();

				if (ImGui::SmallButton(" - "))
				{
					if (component.MaterialHandles.size() > 1)
						component.MaterialHandles.pop_back();
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<MeshRendererComponent>("Mesh Renderer Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<RigidbodyComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Rigidbody Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Rigidbody Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<RigidbodyComponent>();

			ImGui::Columns(2, "rb_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Mass");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			float value = component.GetMass();
			if (ImGui::DragFloat("##RB_Mass", &value, 0.05f, 0.0f, FLT_MAX, "%.2f"))
				component.SetMass(value);
			ImGui::NextColumn();

			ImGui::Text("Drag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			value = component.GetDrag();
			if (ImGui::DragFloat("##RB_Drag", &value, 0.05f, 0.0f, FLT_MAX, "%.2f"))
				component.SetDrag(value);
			ImGui::NextColumn();

			ImGui::Text("Angular Drag");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			value = component.GetAngularDrag();
			if (ImGui::DragFloat("##RB_Angular Drag", &value, 0.05f, 0.0f, FLT_MAX, "%.2f"))
				component.SetAngularDrag(value);
			ImGui::NextColumn();

			ImGui::Text("Automatic Centre of Mass");
			ImGui::NextColumn();
			bool value_2 = component.IsAutomaticCentreOfMassEnabled();
			if (ImGui::Checkbox("##RB_Automatic Centre of Mass", &value_2))
				component.SetAutomaticCentreOfMass(value_2);
			ImGui::NextColumn();

			ImGui::Text("Use Gravity");
			ImGui::NextColumn();
			value_2 = component.IsGravityEnabled();
			if (ImGui::Checkbox("##RB_Gravity", &value_2))
				component.SetGravity(value_2);
			ImGui::NextColumn();

			ImGui::Text("Is Kinematic");
			ImGui::NextColumn();
			value_2 = component.IsKinematicEnabled();
			if (ImGui::Checkbox("##RB_Kinematic", &value_2))
				component.SetKinematic(value_2);
			ImGui::NextColumn();

			ImGui::Columns(1);

			if (ImGui::TreeNodeEx("Constraints", tree_node_flags)) {

				ImGui::Columns(2, "rb_constraints", false);
				ImGui::Text("Freeze Position");
				ImGui::NextColumn();

				bool constraint_modified = false;
				glm::bvec3 value_3 = component.GetPositionConstraint();
				if (ImGui::Checkbox("X##PositionConstraint", &value_3.x))
					constraint_modified = true;

				ImGui::SameLine();
				if (ImGui::Checkbox("Y##PositionConstraint", &value_3.y))
					constraint_modified = true;

				ImGui::SameLine();
				if (ImGui::Checkbox("Z##PositionConstraint", &value_3.z))
					constraint_modified = true;

				if(constraint_modified)
					component.SetPositionConstraint(value_3);

				ImGui::NextColumn();
				ImGui::Text("Freeze Rotation");
				ImGui::NextColumn();

				constraint_modified = false;
				value_3 = component.GetRotationConstraint();
				if (ImGui::Checkbox("X##RotationConstraint", &value_3.x))
					constraint_modified = true;

				ImGui::SameLine();
				if (ImGui::Checkbox("Y##RotationConstraint", &value_3.y))
					constraint_modified = true;

				ImGui::SameLine();
				if (ImGui::Checkbox("Z##RotationConstraint", &value_3.z))
					constraint_modified = true;

				if (constraint_modified)
					component.SetRotationConstraint(value_3);

				ImGui::NextColumn();

				ImGui::Columns(1);

				ImGui::TreePop();
			}


			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<RigidbodyComponent>("Rigidbody Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SphereColliderComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Sphere Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Sphere Collider Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SphereColliderComponent>();

			ImGui::Columns(2, "sc_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Radius");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			float value = component.GetRadius();
			if (ImGui::DragFloat("##SC_Radius", &value, 0.05f, 0.0f, FLT_MAX, "%.2f"))
				component.SetRadius(value);
			ImGui::NextColumn();

			ImGui::Text("Is Trigger");
			ImGui::NextColumn();
			bool value_2 = component.IsTrigger();
			if (ImGui::Checkbox("##SC_Is Trigger", &value_2))
				component.SetIsTrigger(value_2);
			ImGui::NextColumn();

			ImGui::Text("Centre");
			ImGui::NextColumn();

			// Get the width of the current column
			float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

			glm::vec3 value_3 = component.GetCentre();
			bool updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetCentre(value_3);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SphereColliderComponent>("Sphere Collider Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<BoxColliderComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Box Collider Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Box Collider Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<BoxColliderComponent>();

			ImGui::Columns(2, "bc_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Is Trigger");
			ImGui::NextColumn();
			bool value_2 = component.IsTrigger();
			if (ImGui::Checkbox("##SC_Is Trigger", &value_2))
				component.SetIsTrigger(value_2);
			ImGui::NextColumn();

			ImGui::Text("Centre");
			ImGui::NextColumn();

			// Get the width of the current column
			float columnWidth = ImGui::GetColumnWidth() - (ImGui::GetStyle().ItemSpacing.x * 6 + ImGui::CalcTextSize("X").x * 3); // Account for spacing

			glm::vec3 value_3 = component.GetCentre();
			bool updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local PositionZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetCentre(value_3);

			ImGui::NextColumn();
			ImGui::Text("Size");
			ImGui::NextColumn();

			value_3 = component.GetSize();
			updated = false;

			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeX", &value_3.x, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeY", &value_3.y, 0.01f, 0, 0, "%.2f")) updated = true;

			ImGui::SameLine();
			ImGui::Text("Z");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(columnWidth / 3); // Set width to one-third of column width
			if (ImGui::DragFloat("##Local SizeZ", &value_3.z, 0.01f, 0, 0, "%.2f")) updated = true;

			if (updated) component.SetSize(value_3);

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<BoxColliderComponent>("Box Collider Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<PointLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Point Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Point Light Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<PointLightComponent>();

			ImGui::Columns(2, "point_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Radius");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##PointLightRadius", &component.Radius, 0.05f, 0.0f, FLT_MAX, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##PointLightIntensity", &component.Intensity, 0.05f, 0.0f, FLT_MAX, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##PointLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##PointShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<PointLightComponent>("Point Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<SpotLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild("##Spot Light Child", {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx(("Spot Light Component##" + selected_entity.GetName()).c_str(), tree_node_flags)) {

			auto& component = selected_entity.GetComponent<SpotLightComponent>();

			ImGui::Columns(2, "spot_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Angle");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##SpotLightAngle", &component.Angle, 1.0f, 179.0f, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Range");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##SpotLightRange", &component.Range, 0.5f, 0.0f, FLT_MAX, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##SpotLightIntensity", &component.Intensity, 0.5f, 0.0f, FLT_MAX, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##SpotLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##SpotShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<SpotLightComponent>("Spot Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	if (selected_entity.HasComponent<DirectionalLightComponent>()) {

		ImGui::Dummy({ 0.0f, 5.0f });

		ImGui::BeginChild(("##Directional Light Child##" + selected_entity.GetName()).c_str(), {}, ImGuiChildFlags_AutoResizeY);

		if (ImGui::TreeNodeEx("Directional Light Component", tree_node_flags)) {

			auto& component = selected_entity.GetComponent<DirectionalLightComponent>();

			ImGui::Columns(2, "directional_light_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			ImGui::Text("Active");
			ImGui::NextColumn();
			ImGui::Checkbox("##ActiveCheckBox", &component.Active);
			ImGui::NextColumn();

			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::DragFloat("##DirectionalLightIntensity", &component.Intensity, 0.5f, 0.0f, FLT_MAX, "%.2f");
			ImGui::NextColumn();

			ImGui::Text("Colour");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ColorEdit4("##DirectionalLightColour", glm::value_ptr(component.Colour));
			ImGui::NextColumn();

			static std::array<const char*, 3> shadow_types = { "No Shadows", "Hard Shadows", "Soft Shadows" };
			uint8_t item_current = static_cast<uint8_t>(component.ShadowFlag);
			ImGui::Text("Shadow Type");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(-1.0f);
			if (ImGui::BeginCombo("##DirectionalShadowType", shadow_types[item_current])) {

				for (int n = 0; n < shadow_types.size(); n++)
				{
					const bool is_selected = (item_current == n);
					if (ImGui::Selectable(shadow_types[n], is_selected))
					{
						item_current = n;
						component.ShadowFlag = static_cast<ShadowTypeFlag>(item_current);
					}

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::NextColumn();

			if (component.ShadowFlag != ShadowTypeFlag::NoShadows)
			{
				ImGui::TextWrapped("Max Visible Shadow Distance");
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
					ImGui::SetTooltip("This is the normalised threshold distance between the camera position to the far plane. \n\nFor Example: If the far plane of the camera is 1000, and the threshold is 0.10, this max shadow distance be 100 units away.", ImGui::GetStyle().HoverDelayNormal);

				ImGui::NextColumn();

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##DirectionalLightMaxVisibleShadowDistance", &component.MaxShadowVisibleDistance, 0.01f, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay))
					ImGui::SetTooltip("This is the normalised threshold distance between the camera position to the far plane. \n\nFor Example: If the far plane of the camera is 1000, and the threshold is 0.10, this max shadow distance be 100 units away.", ImGui::GetStyle().HoverDelayNormal);

				ImGui::NextColumn();
			}

			ImGui::Columns(1);

			ImGui::TreePop();
		}

		ImGui::EndChild();

		ShowComponentContextPopup<DirectionalLightComponent>("Directional Light Component Options", selected_entity);

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Separator();
	}

	// Material Inspector
	if (selected_entity.HasComponent<MeshRendererComponent>() || selected_entity.HasComponent<SkyboxComponent>()) {

		if (material_list.empty())
		{
			if (selected_entity.HasComponent<MeshRendererComponent>())
			{
				auto& component = selected_entity.GetComponent<MeshRendererComponent>();
				for (const auto& pair : component.MaterialHandles) {
					if (pair.first != NULL_UUID) {
						material_list.push_back(pair.first);
					}
				}
			}

			if (selected_entity.HasComponent<SkyboxComponent>())
				material_list.push_back(selected_entity.GetComponent<SkyboxComponent>().SkyboxMaterialAssetHandle);

		}

		ImGui::Dummy({ 0.0f, 5.0f });
		ImGui::Text("Materials Inspector");
		ImGui::Dummy({ 0.0f, 5.0f });

		int counter = 0;
		for (const auto& asset_handle : material_list) {

			auto& metadata_material = Project::GetStaticEditorAssetManager()->GetMetadata(asset_handle);

			switch (metadata_material.Type) {

			case AssetType::Material_Standard:
			{
				if (!AssetManager::IsAssetHandleValid(asset_handle))
					continue;

				auto asset_material = AssetManager::GetAsset<Material>(asset_handle);
				if (!asset_material)
					continue;

				// Custom Assets are ones that are embedded in other importable files, or inbuilt resources
				bool immutable_material = metadata_material.IsCustomAsset || metadata_material.ParentAssetHandle != NULL_UUID;
				bool material_modified = false;

				if (ImGui::TreeNode(std::string("Material: " + metadata_material.AssetName + "##" + std::to_string(counter)).c_str())) {

					ImGui::Dummy({ 0.0f, 5.0f });

					if (immutable_material) {
						ImGui::BeginDisabled();
					}

					GLuint texture_id = AssetManager::IsAssetHandleValid(asset_material->GetAlbedoTextureAssetHandle()) ? AssetManager::GetAsset<Texture2D>(asset_material->GetAlbedoTextureAssetHandle())->GetID() : 0;

					// Texture and text alignment
					ImGui::ImageButton("##Albedo Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetAlbedoTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Albedo Texture").y / 2.0f));
					ImGui::Text("Albedo Texture");

					// ColorEdit4 button size adjustment
					glm::vec4 colour = asset_material->GetAlbedoTintColour();

					// Size of font impacts size of ColorEdit4 button, and we want this to be uniform to the ImageButton, so we add FramePadding similar to how ImageButton does this internally
					ImGuiContext& context = *ImGui::GetCurrentContext();
					float padding = (32.0f - context.FontSize) / 2.0f + context.Style.FramePadding.y;

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, padding));
					if (ImGui::ColorEdit4("##Albedo Colour", glm::value_ptr(colour), ImGuiColorEditFlags_NoInputs))
					{
						asset_material->SetAlbedoTintColour(colour);
						material_modified = true;
					}
					ImGui::PopStyleVar();

					ImGui::SameLine();

					ImGui::Text("Albedo Colour");

					texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_material->GetMetallicTextureAssetHandle()) ? AssetManager::GetAsset<Texture2D>(asset_material->GetMetallicTextureAssetHandle())->GetID() : 0;

					ImGui::ImageButton("##Metallic Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetMetallicTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Metallic Texture").y / 2.0f));
					ImGui::Text("Metallic Texture");

					if (texture_id == 0) {
						ImGui::Text("Metallic Factor");
						ImGui::SameLine();
						float metallic_temp = asset_material->GetMetallic();
						if (ImGui::SliderFloat("##Metallic", &metallic_temp, 0.0f, 1.0f, "%.2f"))
						{
							asset_material->SetMetallic(metallic_temp);
							material_modified = true;
						}
					}

					ImGui::Text("Roughness");
					ImGui::SameLine();
					float roughness_temp = asset_material->GetRoughness();
					if (ImGui::SliderFloat("##Roughness", &roughness_temp, 0.0f, 1.0f, "%.2f")) 
					{
						asset_material->SetRoughness(roughness_temp);
						material_modified = true;
					}

					texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_material->GetNormalTextureAssetHandle()) ? AssetManager::GetAsset<Texture2D>(asset_material->GetNormalTextureAssetHandle())->GetID() : 0;

					ImGui::ImageButton("##Normal Texture", (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

					if (texture_id == 0 && ImGui::IsItemHovered()) {
						ImGui::BeginTooltip();
						ImGui::Text("Invalid Asset");
						ImGui::EndTooltip();
					}

					// Drag target
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
							AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

							if (Project::GetStaticEditorAssetManager()->IsAssetHandleValid(dropped_asset_handle) && Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
								asset_material->SetNormalTexture(dropped_asset_handle);
								material_modified = true;
							}
							else {
								L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize("Normal Texture").y / 2.0f));
					ImGui::Text("Normal Texture");

					if (immutable_material) {
						ImGui::EndDisabled();
					}


					ImGui::TreePop();
				}
				
				if (material_modified)
				{
					YAML::Emitter out;
					out << YAML::BeginMap;
					asset_material->Serialize(out);
					out << YAML::EndMap;

					std::ofstream fout(Project::GetActiveProject()->GetAssetDirectory() / metadata_material.FilePath); // Create the file
					fout << out.c_str();
				}

				break;
			}

			case AssetType::Material_Skybox:
			{
				if (!Project::GetStaticEditorAssetManager()->IsAssetHandleValid(asset_handle))
					continue;

				auto asset_material = AssetManager::GetAsset<SkyboxMaterial>(asset_handle);
				bool material_modified = false;
				if (ImGui::TreeNode(std::string("Skybox Material: " + metadata_material.AssetName + "##" + std::to_string(counter)).c_str())) {

					ImGui::Dummy({ 0.0f, 5.0f });

					auto& sb_texture_asset_handles = asset_material->GetTextureAssetHandles();

					for (int i = 0; i < sb_texture_asset_handles.size(); i++) {


						GLuint texture_id = Project::GetStaticEditorAssetManager()->IsAssetHandleValid(sb_texture_asset_handles[i]) ? AssetManager::GetAsset<Texture2D>(sb_texture_asset_handles[i])->GetID() : 0;

						static const std::array<std::string, 6> skybox_binding_names = {
							"Right",
							"Left",
							"Top",
							"Bottom",
							"Back",
							"Front"
						};

						// Texture and text alignment
						std::string label = "##" + metadata_material.AssetName + "_" + skybox_binding_names[i] + "_Texture";
						ImGui::ImageButton(label.c_str(), (ImTextureID)(uintptr_t)texture_id, { 32.0f, 32.0f });

						if (texture_id == 0 && ImGui::IsItemHovered()) {
							ImGui::BeginTooltip();
							ImGui::Text("Invalid Asset");
							ImGui::EndTooltip();
						}

						// Drag target
						if (ImGui::BeginDragDropTarget()) {
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_HANDLE")) {
								AssetHandle dropped_asset_handle = *(const AssetHandle*)payload->Data;

								if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
									asset_material->SetSkyboxFaceTexture(static_cast<L_SKYBOX_BINDING>(i), dropped_asset_handle);
									material_modified = true;
								}
								else {
									L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
								}
							}

							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) {

								std::string dropped_asset_path_string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
								std::filesystem::path dropped_asset_path = dropped_asset_path_string;

								if (AssetManager::IsExtensionSupported(dropped_asset_path.extension())) {

									AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_asset_path, Project::GetActiveProject()->GetAssetDirectory());

									if (Project::GetStaticEditorAssetManager()->GetAssetType(dropped_asset_handle) == AssetType::Texture2D) {
										asset_material->SetSkyboxFaceTexture(static_cast<L_SKYBOX_BINDING>(i), dropped_asset_handle);
										material_modified = true;
									}
									else {
										L_APP_WARN("Invalid Asset Type Dropped on Skybox Material Target.");
									}
								}
								else {
									L_APP_WARN("Invalid File Path Dropped on Skybox Material Target.");
								}
							}
							ImGui::EndDragDropTarget();
						}

						ImGui::SameLine();

						std::string label_text = skybox_binding_names[i] + " Texture";
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (32.0f / 2.0f - ImGui::CalcTextSize(label_text.c_str()).y / 2.0f));
						ImGui::Text(label_text.c_str());
					}


					ImGui::TreePop();
				}

				if (material_modified)
				{
					YAML::Emitter out;
					out << YAML::BeginMap;
					asset_material->Serialize(out);
					out << YAML::EndMap;

					std::ofstream fout(Project::GetActiveProject()->GetAssetDirectory() / metadata_material.FilePath); // Create the file
					fout << out.c_str();
				}

				break;
			}

			}

			counter++;
			ImGui::Dummy({ 0.0f, 5.0f });
		}
	}

	ImGui::EndChild();

	DisplayEntitySelectionModal(selected_entity);
}

static bool modal_box_open = false;
static ScriptFieldType modal_box_field_type = ScriptFieldType::Unknown;
static std::string modal_box_script_name = "";
static std::string modal_box_field_name = "";
static Entity modal_box_selected_entity{};

void PropertiesPanel::DisplayScriptFields(const std::string& script_name, Entity selected_entity)
{
	auto script_manager = ScriptManager::Get();

	if (!script_manager)
		return;

	auto scene_ref = Project::GetActiveScene();
	bool script_class_exists = script_manager->ScriptClassExists(script_name);

#pragma region Display Script Fields

	if (scene_ref->IsRunning()) {

		if (auto instance = script_manager->GetScriptClassInstance(selected_entity.GetUUID(), script_name); instance) {

			auto script_class = instance->GetScriptClass().lock();

			if (!script_class || !script_class->fields)
				return;

			auto fields = script_class->fields;

			float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;
			ImGui::Columns(2, "script_field_columns", false);
			ImGui::SetColumnWidth(-1, first_coloumn_width);

			for (int i = 0; i < script_class->field_count; ++i)
			{
				const auto& field = fields[i];
				switch (field.type)
				{
					// Inbuilt Types
					case ScriptFieldType::Float:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						float data;
						if (!instance->GetFieldValue<float>(field.name, data)) data = 0.00f;

						if (ImGui::DragFloat(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data, 0.01f, -FLT_MAX, FLT_MAX, "%.2f")) {
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Double:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						double data;
						if (!instance->GetFieldValue<double>(field.name, data)) data = 0.00;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, &data, 0.01f, nullptr, nullptr, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;

					}
					case ScriptFieldType::Int8:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						int8_t data;
						if (!instance->GetFieldValue<int8_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S8, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;

					}
					case ScriptFieldType::Int16: 
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						int16_t data;
						if (!instance->GetFieldValue<int16_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S16, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Int32: 
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						int32_t data;
						if (!instance->GetFieldValue<int32_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Int64: 
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						int64_t data;
						if (!instance->GetFieldValue<int64_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S64, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::UInt8:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						uint8_t data;
						if (!instance->GetFieldValue<uint8_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U8, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;

					}
					case ScriptFieldType::UInt16:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						uint16_t data;
						if (!instance->GetFieldValue<uint16_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U16, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::UInt32:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						uint32_t data;
						if (!instance->GetFieldValue<uint32_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::UInt64:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						uint64_t data;
						if (!instance->GetFieldValue<uint64_t>(field.name, data)) data = 0;

						if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U64, &data, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Bool: 
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						bool data;
						if (!instance->GetFieldValue<bool>(field.name, data)) data = false;

						if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data)) 
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::CString:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						const char* data = "";
						if (!instance->GetFieldValue<const char*>(field.name, data)) data = "";

						static char buffer[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(buffer, data, sizeof(buffer) - 1);
					#else
						strncpy(buffer, data, sizeof(buffer) - 1);
					#endif

						if (ImGui::InputText(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), buffer, sizeof(buffer)))
						{
							instance->SetFieldValue<const char*>(field.name, buffer);
						}

						ImGui::NextColumn();
						break;
					}

					// Custom Types
					case ScriptFieldType::Vector2: 
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::vec2 data;
						if (!instance->GetFieldValue<glm::vec2>(field.name, data)) data = {0.0f, 0.0f};

						if (ImGui::DragFloat2(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Vector3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::vec3 data;
						if (!instance->GetFieldValue<glm::vec3>(field.name, data)) data = { 0.0f, 0.0f, 0.0f };

						if (ImGui::DragFloat3(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Vector4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::vec4 data;
						if (!instance->GetFieldValue<glm::vec4>(field.name, data)) data = { 0.0f, 0.0f, 0.0f, 0.0f };

						if (ImGui::DragFloat4(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}

					case ScriptFieldType::UVector2:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::uvec2 data;
						if (!instance->GetFieldValue<glm::uvec2>(field.name, data)) data = { 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 2, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::UVector3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::uvec3 data;
						if (!instance->GetFieldValue<glm::uvec3>(field.name, data)) data = { 0, 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 3, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::UVector4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::uvec4 data;
						if (!instance->GetFieldValue<glm::uvec4>(field.name, data)) data = { 0, 0, 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 4, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}

					case ScriptFieldType::IVector2:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::ivec2 data;
						if (!instance->GetFieldValue<glm::ivec2>(field.name, data)) data = { 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 2, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::IVector3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::ivec3 data;
						if (!instance->GetFieldValue<glm::ivec3>(field.name, data)) data = { 0, 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 3, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::IVector4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::ivec4 data;
						if (!instance->GetFieldValue<glm::ivec4>(field.name, data)) data = { 0, 0, 0, 0 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 4, 1))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}

					case ScriptFieldType::DVector2:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::dvec2 data;
						if (!instance->GetFieldValue<glm::dvec2>(field.name, data)) data = { 0.00, 0.00 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 2, 0.01f, nullptr, nullptr, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::DVector3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::dvec3 data;
						if (!instance->GetFieldValue<glm::dvec3>(field.name, data)) data = { 0.00, 0.00, 0.00 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 3, 0.01f, nullptr, nullptr, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::DVector4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::dvec4 data;
						if (!instance->GetFieldValue<glm::dvec4>(field.name, data)) data = { 0.00, 0.00, 0.00, 0.00 };

						if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 4, 0.01f, nullptr, nullptr, "%.2f"))
						{
							instance->SetFieldValue(field.name, data);
						}

						ImGui::NextColumn();
						break;
					}

					case ScriptFieldType::BVector2:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::bvec2 data;
						if (!instance->GetFieldValue<glm::bvec2>(field.name, data)) data = { false, false };

						bool modified = false;
						for (int i = 0; i < 2; ++i)
						{
							switch (i)
							{
								case 0: ImGui::Text("X"); break;
								case 1: ImGui::Text("Y"); break;
							}

							ImGui::SameLine();

							if (ImGui::Checkbox(std::string("##"  + std::string{field.name} + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
								modified = true;

							ImGui::SameLine();
						}

						if (modified)
							instance->SetFieldValue(field.name, data);

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::BVector3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::bvec3 data;
						if (!instance->GetFieldValue<glm::bvec3>(field.name, data)) data = { false, false, false };

						bool modified = false;
						for (int i = 0; i < 3; ++i)
						{
							switch (i)
							{
								case 0: ImGui::Text("X"); break;
								case 1: ImGui::Text("Y"); break;
								case 2: ImGui::Text("Z"); break;
							}

							ImGui::SameLine();

							if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
								modified = true;

							ImGui::SameLine();
						}

						if (modified)
							instance->SetFieldValue(field.name, data);

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::BVector4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::bvec4 data;
						if (!instance->GetFieldValue<glm::bvec4>(field.name, data)) data = { false, false, false, false };

						bool modified = false;
						for (int i = 0; i < 4; ++i)
						{
							switch (i)
							{
								case 0: ImGui::Text("X"); break;
								case 1: ImGui::Text("Y"); break;
								case 2: ImGui::Text("Z"); break;
								case 3: ImGui::Text("W"); break;
							}

							ImGui::SameLine();

							if(ImGui::Checkbox(std::string("##" + std::string{ field.name } + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
								modified = true;

							ImGui::SameLine();
						}

						if(modified)
							instance->SetFieldValue(field.name, data);

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Mat3:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::mat3 data;
						if (!instance->GetFieldValue<glm::mat3>(field.name, data)) data = glm::mat3(1.0f);

						bool modified = false;
						for (int row = 0; row < 3; ++row)
						{
							glm::vec3 rowVec = glm::vec3(data[0][row], data[1][row], data[2][row]);

							if (ImGui::DragFloat3(std::string("##" + std::string{ field.name } + "_row" + std::to_string(row)).c_str(), glm::value_ptr(rowVec), 0.01f))
							{
								data[0][row] = rowVec.x;
								data[1][row] = rowVec.y;
								data[2][row] = rowVec.z;
								modified = true;
							}
						}

						if (modified)
							instance->SetFieldValue(field.name, data);

						ImGui::NextColumn();
						break;
					}
					case ScriptFieldType::Mat4:
					{
						ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
						ImGui::NextColumn();

						glm::mat4 data;
						if (!instance->GetFieldValue<glm::mat4>(field.name, data)) data = glm::mat4(1.0f);

						bool modified = false;
						for (int row = 0; row < 4; ++row)
						{
							glm::vec4 rowVec = glm::vec4(data[0][row], data[1][row], data[2][row], data[3][row]);

							if (ImGui::DragFloat4(std::string("##" + std::string{ field.name } + "_row" + std::to_string(row)).c_str(), glm::value_ptr(rowVec), 0.01f))
							{
								data[0][row] = rowVec.x;
								data[1][row] = rowVec.y;
								data[2][row] = rowVec.z;
								data[3][row] = rowVec.w;
								modified = true;
							}
						}

						if (modified)
							instance->SetFieldValue(field.name, data);

						ImGui::NextColumn();
						break;
					}

					// ECS Types
					case ScriptFieldType::Entity:

					case ScriptFieldType::IDComponent:
					case ScriptFieldType::TagComponent:
					case ScriptFieldType::HierarchyComponent:
					case ScriptFieldType::ScriptComponent:
					case ScriptFieldType::TransformComponent:

					case ScriptFieldType::CameraComponent:

					case ScriptFieldType::AudioListenerComponent:
					case ScriptFieldType::AudioEmitterComponent:

					case ScriptFieldType::MeshFilterComponent:
					case ScriptFieldType::MeshRendererComponent:
					case ScriptFieldType::LODMeshComponent:

					case ScriptFieldType::SkinnedMeshComponent:
					case ScriptFieldType::AnimatorComponent:

					case ScriptFieldType::SkyboxComponent:

					case ScriptFieldType::PointLightComponent:
					case ScriptFieldType::SpotLightComponent:
					case ScriptFieldType::DirectionalLightComponent:

					case ScriptFieldType::RigidbodyComponent:
					case ScriptFieldType::BoxColliderComponent:
					case ScriptFieldType::SphereColliderComponent:

					case ScriptFieldType::Component:
					{
						ImGui::Text(field.name);
						ImGui::NextColumn();

						uint32_t data;
						if (!instance->GetFieldValue<uint32_t>(field.name, data)) data = NULL_UUID;

						std::string label = "##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type));
						std::string text = (scene_ref->HasEntityByUUID(data)) ? scene_ref->FindEntityByUUID(data).GetName().c_str() : ("None (" + std::string(ScriptUtils::FieldTypeToString(field.type)) + ")");

						char buffer[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(buffer, text.c_str(), sizeof(buffer));
					#else
						strncpy(buffer, text.c_str(), sizeof(buffer));
					#endif

						ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
						{
							modal_box_open = true;
							modal_box_field_name = field.name;
							modal_box_field_type = field.type;
							modal_box_script_name = script_name;
						}

						ImGui::SameLine();

						if (ImGui::Button(std::string("...##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type))).c_str()))
						{
							modal_box_open = true;
							modal_box_field_name = field.name;
							modal_box_field_type = field.type;
							modal_box_script_name = script_name;
						}

						ImGui::NextColumn();
						break;
					}

					// Asset Types
					case ScriptFieldType::Prefab:
					case ScriptFieldType::Shader:
					case ScriptFieldType::ComputeShader:
					case ScriptFieldType::Material:
					case ScriptFieldType::Texture2D:
					case ScriptFieldType::TextureCubeMap:
					case ScriptFieldType::StaticMesh:
					case ScriptFieldType::AudioClip:
					case ScriptFieldType::Skeleton:
					case ScriptFieldType::AnimationClip:
					{
						ImGui::Text(field.name);
						ImGui::NextColumn();

						uint32_t data;
						if (!instance->GetFieldValue<uint32_t>(field.name, data)) data = NULL_UUID;

						std::string label = "##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type));

						AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(data);
						std::string text = (meta_data.AssetName != "") ? meta_data.AssetName + " (" + ScriptUtils::FieldTypeToString(field.type) +")" : "None(" + ScriptUtils::FieldTypeToString(field.type) + ")";

						char buffer[256];

					#if defined(L_PLATFORM_WINDOWS)
						strncpy_s(buffer, text.c_str(), sizeof(buffer));
					#else
						strncpy(buffer, text.c_str(), sizeof(buffer));
					#endif

						ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

						if (ImGui::BeginDragDropTarget()) 
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE")) 
							{
								// Convert the payload data (string) back into a filesystem path
								std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
								std::filesystem::path dropped_path = dropped_path_str; // Convert to path

								if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::Unknown) {

									AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path, Project::GetActiveProject()->GetAssetDirectory());

									switch (asset_type) 
									{
										case AssetType::Prefab:
										case AssetType::Shader:
										case AssetType::Compute_Shader:
										case AssetType::Material_Standard:
										case AssetType::Texture2D:
										case AssetType::TextureCubeMap:
										case AssetType::Mesh:
										case AssetType::ModelImport:
										case AssetType::Audio:
										case AssetType::Skeleton:
										case AssetType::AnimationClip: instance->SetFieldValue(field.name, dropped_asset_handle);						break;

										default:  L_APP_WARN("Cannot Set Asset Type {} to Script Prefab Field.", dropped_path.extension().string());	break;
									}
								}
								else {
									L_APP_WARN("Cannot Set Prefab {} to Script.", dropped_path.filename().string());
								}

							}
							ImGui::EndDragDropTarget();
						}

						ImGui::NextColumn();


						break;
					}

				}
			}

			ImGui::Columns(1);

		}
	}
	else if (script_class_exists) {

		auto script_class = ScriptManager::Get()->GetScriptClass(script_name);

		if (!script_class || !script_class->fields)
			return;

		auto fields = script_class->fields;

		auto script_field_map = ScriptManager::Get()->GetScriptFieldMap(selected_entity.GetUUID(), script_name);
		auto script_field_default_values_map = ScriptManager::Get()->GetScriptFieldMap(NULL_UUID, script_name);

		if (!script_field_map || !script_field_default_values_map)
			return;

		float first_coloumn_width = ImGui::GetContentRegionAvail().x * 0.35f;
		ImGui::Columns(2, "script_field_columns", false);
		ImGui::SetColumnWidth(-1, first_coloumn_width);

		for (int i = 0; i < script_class->field_count; i++)
		{
			const auto& field = fields[i];

			bool script_field_instance_exists = script_field_map->find(field.name) != script_field_map->end();
			bool script_field_default_value_exists = script_field_default_values_map->find(field.name) != script_field_default_values_map->end();

			if (!script_field_instance_exists && !script_field_default_value_exists)
				continue;

			ScriptFieldInstance* script_field = script_field_instance_exists ? &script_field_map->at(field.name) : nullptr;
			ScriptFieldInstance* script_default_value_field = script_field_default_value_exists ? &script_field_default_values_map->at(field.name) : nullptr;
			switch (field.type) 
			{
				case ScriptFieldType::Float: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					float data = script_field_instance_exists ? script_field->GetValue<float>() : script_field_default_value_exists ? script_default_value_field->GetValue<float>() : 0.0f;
					if (ImGui::DragFloat(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data, 0.01f, 0.0f, 0.0f, "%.2f")) 
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Double: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					double data = script_field_instance_exists ? script_field->GetValue<double>() : script_field_default_value_exists ? script_default_value_field->GetValue<double>() : 0.00;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, &data, 0.01f, nullptr, nullptr, "%.2f")) 
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int8:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					int8_t data = script_field_instance_exists ? script_field->GetValue<int8_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<int8_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S8, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int16:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					int16_t data = script_field_instance_exists ? script_field->GetValue<int16_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<int16_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S16, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int32:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					int32_t data = script_field_instance_exists ? script_field->GetValue<int32_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<int32_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Int64:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					int64_t data = script_field_instance_exists ? script_field->GetValue<int64_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<int64_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S64, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UInt8:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					uint8_t data = script_field_instance_exists ? script_field->GetValue<uint8_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint8_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U8, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UInt16:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					uint16_t data = script_field_instance_exists ? script_field->GetValue<uint16_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint16_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U16, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UInt32:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					uint32_t data = script_field_instance_exists ? script_field->GetValue<uint32_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint32_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UInt64:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					uint64_t data = script_field_instance_exists ? script_field->GetValue<uint64_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint64_t>() : 0;
					if (ImGui::DragScalar(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U64, &data, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Bool: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					bool data = script_field_instance_exists ? script_field->GetValue<bool>() : script_field_default_value_exists ? script_default_value_field->GetValue<bool>() : false;
					if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data)) 
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::CString:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					const char* data = script_field_instance_exists ? script_field->GetCStringValue() : script_field_default_value_exists ? script_default_value_field->GetCStringValue() : "";

					static char buffer[256];

				#if defined(L_PLATFORM_WINDOWS)
					strncpy_s(buffer, data, sizeof(buffer) - 1);
				#else
					strncpy(buffer, data, sizeof(buffer) - 1);
				#endif

					if (ImGui::InputText(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), buffer, sizeof(buffer)))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetCStringValue(buffer);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector2: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::vec2 data = script_field_instance_exists ? script_field->GetValue<glm::vec2>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::vec2>() : glm::vec2(0.0f);
					if (ImGui::DragFloat2(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f")) 
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector3: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::vec3 data = script_field_instance_exists ? script_field->GetValue<glm::vec3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::vec3>() : glm::vec3(0.0f);

					std::string lower_name = field.name;
					std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

					if (lower_name.find("rgb") != std::string::npos || lower_name.find("colour") != std::string::npos ||
						lower_name.find("color") != std::string::npos || lower_name.find("col") != std::string::npos)
					{
						if (ImGui::ColorEdit3(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data)))
						{
							if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
							script_field->SetValue(data);
						}
					}
					else
					{
						if (ImGui::DragFloat3(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
						{
							if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
							script_field->SetValue(data);
						}
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Vector4: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::vec4 data = script_field_instance_exists ? script_field->GetValue<glm::vec4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::vec4>() : glm::vec4(0.0f);

					std::string lower_name = field.name;
					std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

					if (lower_name.find("rgb") != std::string::npos || lower_name.find("colour") != std::string::npos ||
						lower_name.find("color") != std::string::npos || lower_name.find("col") != std::string::npos)
					{
						if (ImGui::ColorEdit4(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data)))
						{
							if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
							script_field->SetValue(data);
						}
					}
					else
					{
						if (ImGui::DragFloat4(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), glm::value_ptr(data), 0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
						{
							if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
							script_field->SetValue(data);
						}
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UVector2: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::uvec2 data = script_field_instance_exists ? script_field->GetValue<glm::uvec2>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::uvec2>() : glm::uvec2(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 2, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UVector3: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::uvec3 data = script_field_instance_exists ? script_field->GetValue<glm::uvec3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::uvec3>() : glm::uvec3(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 3, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::UVector4: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::uvec4 data = script_field_instance_exists ? script_field->GetValue<glm::uvec4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::uvec4>() : glm::uvec4(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_U32, glm::value_ptr(data), 4, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::IVector2: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::ivec2 data = script_field_instance_exists ? script_field->GetValue<glm::ivec2>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::ivec2>() : glm::ivec2(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 2, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::IVector3: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::ivec3 data = script_field_instance_exists ? script_field->GetValue<glm::ivec3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::ivec3>() : glm::ivec3(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 3, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::IVector4: 
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::ivec4 data = script_field_instance_exists ? script_field->GetValue<glm::ivec4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::ivec4>() : glm::ivec4(0);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_S32, glm::value_ptr(data), 4, 1))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::DVector2:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::dvec2 data = script_field_instance_exists ? script_field->GetValue<glm::dvec2>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::dvec2>() : glm::dvec2(0.00);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 2, 0.01f, nullptr, nullptr, "%.2f"))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::DVector3:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::dvec3 data = script_field_instance_exists ? script_field->GetValue<glm::dvec3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::dvec3>() : glm::dvec3(0.00);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 3, 0.01f, nullptr, nullptr, "%.2f"))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::DVector4:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::dvec4 data = script_field_instance_exists ? script_field->GetValue<glm::dvec4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::dvec4>() : glm::dvec4(0.00);
					if (ImGui::DragScalarN(std::string("##" + std::string{ field.name } + ScriptUtils::FieldTypeToString(field.type)).c_str(), ImGuiDataType_Double, glm::value_ptr(data), 4, 0.01f, nullptr, nullptr, "%.2f"))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}
					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::BVector2:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::bvec2 data = script_field_instance_exists ? script_field->GetValue<glm::bvec2>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::bvec2>() : glm::bvec2(false);

					bool modified = false;
					for (int i = 0; i < 2; ++i)
					{
						switch (i)
						{
							case 0: ImGui::Text("X"); break;
							case 1: ImGui::Text("Y"); break;
						}

						ImGui::SameLine();

						if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
							modified = true;

						ImGui::SameLine();
					}

					if (modified)
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::BVector3:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::bvec3 data = script_field_instance_exists ? script_field->GetValue<glm::bvec3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::bvec3>() : glm::bvec3(false);

					bool modified = false;
					for (int i = 0; i < 3; ++i)
					{
						switch (i)
						{
						case 0: ImGui::Text("X"); break;
						case 1: ImGui::Text("Y"); break;
						case 2: ImGui::Text("Z"); break;
						}

						ImGui::SameLine();

						if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
							modified = true;

						ImGui::SameLine();
					}

					if (modified)
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::BVector4:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::bvec4 data = script_field_instance_exists ? script_field->GetValue<glm::bvec4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::bvec4>() : glm::bvec4(false);

					bool modified = false;
					for (int i = 0; i < 4; ++i)
					{
						switch (i)
						{
							case 0: ImGui::Text("X"); break;
							case 1: ImGui::Text("Y"); break;
							case 2: ImGui::Text("Z"); break;
							case 3: ImGui::Text("W"); break;
						}

						ImGui::SameLine();

						if (ImGui::Checkbox(std::string("##" + std::string{ field.name } + std::to_string(i).c_str() + ScriptUtils::FieldTypeToString(field.type)).c_str(), &data[i]))
							modified = true;

						ImGui::SameLine();
					}

					if (modified)
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Mat3:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::mat3 data = script_field_instance_exists ? script_field->GetValue<glm::mat3>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::mat3>() : glm::mat3(1.0f);

					bool modified = false;
					for (int row = 0; row < 3; ++row)
					{
						glm::vec3 rowVec = glm::vec3(data[0][row], data[1][row], data[2][row]);

						if (ImGui::DragFloat3(std::string("##" + std::string{ field.name } + "_row" + std::to_string(row)).c_str(), glm::value_ptr(rowVec), 0.01f))
						{
							data[0][row] = rowVec.x;
							data[1][row] = rowVec.y;
							data[2][row] = rowVec.z;
							modified = true;
						}
					}

					if (modified)
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}

					ImGui::NextColumn();
					break;
				}
				case ScriptFieldType::Mat4:
				{
					ImGui::Text("%s: %s", ScriptUtils::FieldTypeToString(field.type).c_str(), field.name);
					ImGui::NextColumn();

					glm::mat4 data = script_field_instance_exists ? script_field->GetValue<glm::mat4>() : script_field_default_value_exists ? script_default_value_field->GetValue<glm::mat4>() : glm::mat4(1.0f);

					bool modified = false;
					for (int row = 0; row < 4; ++row)
					{
						glm::vec4 rowVec = glm::vec4(data[0][row], data[1][row], data[2][row], data[3][row]);

						if (ImGui::DragFloat4(std::string("##" + std::string{ field.name } + "_row" + std::to_string(row)).c_str(), glm::value_ptr(rowVec), 0.01f))
						{
							data[0][row] = rowVec.x;
							data[1][row] = rowVec.y;
							data[2][row] = rowVec.z;
							data[3][row] = rowVec.w;
							modified = true;
						}
					}

					if (modified)
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						script_field->SetValue(data);
					}

					ImGui::NextColumn();
					break;
				}

				// ECS Types
				case ScriptFieldType::Entity:

				case ScriptFieldType::IDComponent:
				case ScriptFieldType::TagComponent:
				case ScriptFieldType::HierarchyComponent:
				case ScriptFieldType::ScriptComponent:
				case ScriptFieldType::TransformComponent:

				case ScriptFieldType::CameraComponent:

				case ScriptFieldType::AudioListenerComponent:
				case ScriptFieldType::AudioEmitterComponent:

				case ScriptFieldType::MeshFilterComponent:
				case ScriptFieldType::MeshRendererComponent:
				case ScriptFieldType::LODMeshComponent:

				case ScriptFieldType::SkinnedMeshComponent:
				case ScriptFieldType::AnimatorComponent:

				case ScriptFieldType::SkyboxComponent:

				case ScriptFieldType::PointLightComponent:
				case ScriptFieldType::SpotLightComponent:
				case ScriptFieldType::DirectionalLightComponent:

				case ScriptFieldType::RigidbodyComponent:
				case ScriptFieldType::BoxColliderComponent:
				case ScriptFieldType::SphereColliderComponent:

				case ScriptFieldType::Component:
				{
					ImGui::Text(field.name);
					ImGui::NextColumn();

					uint32_t data = script_field_instance_exists ? script_field->GetValue<uint32_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint32_t>() : NULL_UUID;

					std::string label = "##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type));
					std::string text = (scene_ref->HasEntityByUUID(data)) ? scene_ref->FindEntityByUUID(data).GetName().c_str() : ("None (" + std::string(ScriptUtils::FieldTypeToString(field.type)) + ")");

					char buffer[256];

				#if defined(L_PLATFORM_WINDOWS)
					strncpy_s(buffer, text.c_str(), sizeof(buffer));
				#else
					strncpy(buffer, text.c_str(), sizeof(buffer));
				#endif

					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						modal_box_open = true;
						modal_box_field_name = field.name;
						modal_box_field_type = field.type;
						modal_box_script_name = script_name;
					}

					ImGui::SameLine();

					if (ImGui::Button(std::string("...##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type))).c_str()))
					{
						if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
						modal_box_open = true;
						modal_box_field_name = field.name;
						modal_box_field_type = field.type;
						modal_box_script_name = script_name;
					}

					ImGui::NextColumn();
					break;
				}

				// Asset Types
				case ScriptFieldType::Prefab:
				case ScriptFieldType::Shader:
				case ScriptFieldType::ComputeShader:
				case ScriptFieldType::Material:
				case ScriptFieldType::Texture2D:
				case ScriptFieldType::TextureCubeMap:
				case ScriptFieldType::StaticMesh:
				case ScriptFieldType::AudioClip:
				case ScriptFieldType::Skeleton:
				case ScriptFieldType::AnimationClip:
				{
					ImGui::Text(field.name);
					ImGui::NextColumn();

					uint32_t data = script_field_instance_exists ? script_field->GetValue<uint32_t>() : script_field_default_value_exists ? script_default_value_field->GetValue<uint32_t>() : NULL_UUID;

					std::string label = "##" + std::string{ field.name } + std::string(ScriptUtils::FieldTypeToString(field.type));

					AssetMetaData meta_data = Project::GetStaticEditorAssetManager()->GetMetadata(data);
					std::string text = (meta_data.AssetName != "") ? meta_data.AssetName + " (" + ScriptUtils::FieldTypeToString(field.type) + ")" : "None(" + ScriptUtils::FieldTypeToString(field.type) + ")";

					char buffer[256];

				#if defined(L_PLATFORM_WINDOWS)
					strncpy_s(buffer, text.c_str(), sizeof(buffer));
				#else
					strncpy(buffer, text.c_str(), sizeof(buffer));
				#endif

					ImGui::InputText(label.c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_FILE"))
						{
							// Convert the payload data (string) back into a filesystem path
							std::string dropped_path_str(static_cast<const char*>(payload->Data), payload->DataSize - 1);
							std::filesystem::path dropped_path = dropped_path_str; // Convert to path

							if (AssetType asset_type = AssetManager::GetAssetTypeFromFileExtension(dropped_path.extension()); asset_type != AssetType::Unknown) {

								AssetHandle dropped_asset_handle = Project::GetStaticEditorAssetManager()->GetHandleFromFilePath(dropped_path, Project::GetActiveProject()->GetAssetDirectory());

								switch (asset_type)
								{
									case AssetType::Prefab:
									case AssetType::Shader:
									case AssetType::Compute_Shader:
									case AssetType::Material_Standard:
									case AssetType::Texture2D:
									case AssetType::TextureCubeMap:
									case AssetType::Mesh:
									case AssetType::ModelImport:
									case AssetType::Audio:
									case AssetType::Skeleton:
									case AssetType::AnimationClip:
									{
										if (!script_field) script_field = &(*script_field_map)[field.name]; *script_field = *script_default_value_field;
										script_field->SetValue(dropped_asset_handle);
										break;
									}
									default:
									{
										L_APP_WARN("Cannot Set Asset Type {} to Script Prefab Field.", dropped_path.extension().string());
										break;
									}
								}
							}
							else 
							{
								L_APP_WARN("Cannot Set Prefab {} to Script.", dropped_path.filename().string());
							}

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::NextColumn();


					break;
				}
			}
		}

		ImGui::Columns(1);
	}

#pragma endregion

}

void PropertiesPanel::DisplayEntitySelectionModal(Entity& selected_entity)
{
	auto scene_ref = Project::GetActiveScene();
	bool script_class_exists = ScriptManager::Get()->ScriptClassExists(modal_box_script_name);

#pragma region Entity Selection w/ Component Type

	if (modal_box_open) {
		ImGui::OpenPopup("Select Entity");

		// Calculate window size: 25% of the screen width, minimum 300.0f
		float windowWidth = glm::max(600.0f, ImGui::GetContentRegionAvail().x * 0.25f);
		float windowHeight = glm::max(600.0f, ImGui::GetContentRegionAvail().y * 0.25f); // Fixed height
		ImVec2 windowSize(windowWidth, windowHeight);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	}

	auto set_entity_value = [&](Entity& entity) {

		if (scene_ref->IsRunning()) {

			if (auto instance = ScriptManager::Get()->GetScriptClassInstance(selected_entity.GetUUID(), modal_box_script_name); instance)
			{
				instance->SetFieldValue(ScriptManager::Get()->GetScriptFieldMap(selected_entity.GetUUID(), modal_box_script_name)->at(modal_box_field_name).field.name, selected_entity.GetUUID());

				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::Unknown;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}

		}
		else if (script_class_exists) 
		{

			auto script_field_map = ScriptManager::Get()->GetScriptFieldMap(selected_entity.GetUUID(), modal_box_script_name);

			if (!script_field_map)
			{
				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::Unknown;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}

			// Field has been set in editor
			if (script_field_map->find(modal_box_field_name) != script_field_map->end())
			{
				ScriptFieldInstance& script_field = (*script_field_map)[modal_box_field_name];
				script_field.SetValue(entity.GetUUID());

				modal_box_open = false;
				modal_box_field_type = ScriptFieldType::Unknown;
				modal_box_script_name = "";
				modal_box_field_name = "";
				modal_box_selected_entity = {};
				return;
			}

		}
	};

	if (ImGui::BeginPopupModal("Select Entity", &modal_box_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

		std::vector<Entity> available_entities{};
		switch (modal_box_field_type) {

			case ScriptFieldType::TagComponent:
			case ScriptFieldType::TransformComponent:
			case ScriptFieldType::Entity: {

				auto view = scene_ref->GetAllEntitiesWith<TransformComponent, TagComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::ScriptComponent: {

				auto view = scene_ref->GetAllEntitiesWith<ScriptComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::PointLightComponent: {

				auto view = scene_ref->GetAllEntitiesWith<PointLightComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::SpotLightComponent: {

				auto view = scene_ref->GetAllEntitiesWith<SpotLightComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::DirectionalLightComponent: {

				auto view = scene_ref->GetAllEntitiesWith<DirectionalLightComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::RigidbodyComponent: {

				auto view = scene_ref->GetAllEntitiesWith<RigidbodyComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::BoxColliderComponent: {

				auto view = scene_ref->GetAllEntitiesWith<BoxColliderComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::SphereColliderComponent: {

				auto view = scene_ref->GetAllEntitiesWith<SphereColliderComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}
			case ScriptFieldType::MeshRendererComponent: {

				auto view = scene_ref->GetAllEntitiesWith<MeshRendererComponent>();

				for (auto entity_handle : view) {
					available_entities.push_back({ entity_handle, scene_ref.get() });
				}

				break;
			}

		}

		const float footer_height = 40.0f;
		const ImVec2 child_size(0, ImGui::GetContentRegionAvail().y - footer_height); // Adjust the height based on your preference
		ImGui::BeginChild("EntitySelectableList", child_size, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			// ImGui selectable list
			for (auto& entity : available_entities) {
				const std::string& entity_name = entity.GetName();

				// Check if this entity is the selected one
				bool isSelected = modal_box_selected_entity == entity;

				if (ImGui::Selectable(std::string(" - " + entity_name).c_str(), isSelected)) {
					modal_box_selected_entity = entity;
				}

				// Handle double click
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
					set_entity_value(entity);
					break;
				}
			}
		}
		ImGui::EndChild();

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footer_height);
		ImGui::BeginChild("EntitySelectableFooterBar", ImVec2(0, footer_height), false);
		{

			// Calculate the position for the button to be in the middle-right of the FooterBar
			static ImVec2 button_size = ImGui::CalcTextSize(" Select Entity ");
			float footer_middle_y = button_size.y * 0.5f;

			ImGui::SetCursorPosY(footer_middle_y);  // Center the button vertically within the footer

			// Calculate the position for the button to be in the middle-right of the FooterBar
			float content_region_width = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX(content_region_width - button_size.x - 10.0f);  // 10.0f for some padding from the right edge

			if (!modal_box_selected_entity)
				ImGui::BeginDisabled();

			if (ImGui::Button("Select Entity", ImVec2(button_size.x, 0)))
			{
				set_entity_value(modal_box_selected_entity);
			}
			else if (!modal_box_selected_entity)
				ImGui::EndDisabled();
		}

		ImGui::EndChild();

		ImGui::EndPopup();
	}


#pragma endregion

}
