#include "RendererPipeline.h"

// Louron Core Headers
#include "Renderer.h"
#include "../Scene/Spatial Partitioning/OctreeBounds.h"
#include "../OpenGL/Framebuffer.h"

#include "../Core/Time.h"
#include "../Core/Engine.h"
#include "../Debug/Profiler.h"
#include "../Asset/Asset Manager API.h"

#include "../Project/Project.h"

#include "../Scene/Entity.h"

// C++ Standard Library Headers
#include <future>

// External Vendor Library Headers
#include <entt/entt.hpp>

#include <glm/gtx/string_cast.hpp>

namespace Louron {

	//static GLFWwindow* s_ThreadWindow = nullptr;
	constexpr uint32_t EDITOR_CAMERA_HANDLE = NULL_UUID - 1;

	namespace SSBOLightStructs {

		struct alignas(16) PL_SSBO_DATA_LAYOUT {

			glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLfloat radius = 10.0f;
			GLfloat intensity = 1.0f;
			GLint active = true;
			GLint lastLight = false;

			GLuint shadowCastingType = 0;
			GLuint shadowLayerIndex = -1;

			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;

			PL_SSBO_DATA_LAYOUT() = default;

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& point_light, TransformComponent& transform) {

				position = { transform.GetGlobalPosition(), 1.0f };
				shadowCastingType = static_cast<GLuint>(point_light.ShadowFlag);

				colour = point_light.Colour;

				radius = point_light.Radius;
				intensity = point_light.Intensity;
				active = point_light.Active ? 1 : 0;

			}

			PL_SSBO_DATA_LAYOUT(const PointLightComponent& point_light) {

				colour = point_light.Colour;

				radius = point_light.Radius;
				intensity = point_light.Intensity;
				active = point_light.Active ? 1 : 0;

			}
			
		};

		struct alignas(16) SL_SSBO_DATA_LAYOUT {

			glm::vec4 position = { 0.0f, 0.0f, 0.0f, 0.0f };
			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLfloat range = 10.0f;
			GLfloat angle = 45.0f;
			GLfloat intensity = 1.0f;
			
			GLint active = true;
			GLint lastLight = false;

			GLuint shadowCastingType = 0;
			GLuint shadowLightIndex = 0;

			GLfloat m_Padding1 = 0.0f;

			SL_SSBO_DATA_LAYOUT() = default;

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& spot_light, TransformComponent& transform) {

				position  = { transform.GetGlobalPosition(), 1.0f };
				direction = { transform.GetForwardDirection(), 1.0f };
				shadowCastingType = static_cast<GLuint>(spot_light.ShadowFlag);

				colour = spot_light.Colour;

				range = spot_light.Range;
				angle = spot_light.Angle;
				intensity = spot_light.Intensity;
				active = spot_light.Active ? 1 : 0;

			}

			SL_SSBO_DATA_LAYOUT(const SpotLightComponent& spot_light) {

				colour = spot_light.Colour;

				range = spot_light.Range;
				angle = spot_light.Angle;
				intensity = spot_light.Intensity;
				active = spot_light.Active ? 1 : 0;
			}

		};

		struct alignas(16) DL_SSBO_DATA_LAYOUT {

			glm::vec4 direction = { 0.0f, 0.0f, -1.0f, 0.0f };

			glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			GLint active = true;
			GLfloat intensity = 1.0f;
			GLint lastLight = false;

			std::array<float, 5> shadowCascadePlaneDistances = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

			GLuint shadowCastingType = 0;
			GLuint shadowLightIndex = 0;

			// DO NOT USE - this is for SSBO alignment purposes ONLY
			GLfloat m_Padding1 = 0.0f;
			GLfloat m_Padding2 = 0.0f;

			DL_SSBO_DATA_LAYOUT() = default;

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& directional_light, TransformComponent& transform) {
				
				direction = { transform.GetForwardDirection(), 1.0f };
				shadowCastingType = static_cast<GLuint>(directional_light.ShadowFlag);

				active = directional_light.Active ? 1 : 0;
				colour = directional_light.Colour;
				intensity = directional_light.Intensity;
			}

			DL_SSBO_DATA_LAYOUT(const DirectionalLightComponent& directional_light) {

				active = directional_light.Active ? 1 : 0;
				colour = directional_light.Colour;
				intensity = directional_light.Intensity;
			}

		};

	}
		
#pragma region ForwardPlusPipeline

	//
	//	-- PUBLIC STATIC FUNCTIONS ---
	//

	void ForwardPlusPipeline::OnStartPipeline(Scene* scene) {

		// We want to benefit from the ConductDepthPass depth values in the depth buffer for the 
		// ConductRenderPass, so we use LEQUAL  to ensure that fragments are not discarded because 
		// the depth values from the depth pass will be EQUAL to the depth values in the render pass
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		
		s_SceneContext[scene] = std::make_unique<SceneContext>(scene);

		//if (!s_ThreadWindow)
		//{
		//	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		//	s_ThreadWindow = glfwCreateWindow(1, 1, "Threaded OpenGL Context Window", nullptr, (GLFWwindow*)Engine::Get().GetWindow().GetNativeWindow());

		//	L_CORE_ASSERT(s_ThreadWindow, "Could Not Create Threaded OpenGL Context");
		//}
	}

	void ForwardPlusPipeline::OnStopPipeline(Scene* scene) {
		
		s_SceneContext.erase(scene);
		
		Renderer::CleanupRenderData();

		//if(s_ThreadWindow)
		//{
		//	glfwDestroyWindow(s_ThreadWindow);
		//	
		//	s_ThreadWindow = nullptr;
		//}
	}

	void ForwardPlusPipeline::OnRenderScene(Scene* scene) 
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::OnRenderScene - Overall");

		if (!scene)
		{
			L_CORE_ERROR("F+Pipeline::OnRenderScene - Invalid Scene");
			return;
		}

		auto& scene_context = s_SceneContext[scene];

		// 1. Identify Cameras for Rendering
		std::vector<CameraContext*> active_cameras;
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::OnRenderScene - Identify Cameras for Rendering");
			const auto& scene_cameras = GetActiveCameras(scene);
			for (const auto& scene_camera : scene_cameras)
			{
				if (!scene_camera)
					continue;

				if (scene_context->CameraContexts.count(scene_camera.GetUUID()) == 0)
					continue;

				active_cameras.push_back(scene_context->CameraContexts[scene_camera.GetUUID()].get());
				active_cameras.back()->ValidateSceneCameraContext(scene);
			}
		}

		// 2. Dispatch Threads - OctreeUpdate_Thread
		{
			if (scene_context->OctreeUpdate_Thread.joinable())
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::OnRenderScene - Octree Thread Wait");
				scene_context->OctreeUpdate_Thread.join();
			}
			scene_context->OctreeUpdate_Thread = std::thread([&, scene]() -> void
			{
				scene_context->OnOctreeUpdate(scene);
			});
		}

		// 3. Perform Global Scene Functions

		Renderer::ClearRenderStats();

		scene_context->SortRenderQueue(scene, active_cameras);
		scene_context->UpdateGlobalShadowMaps(scene, active_cameras);
		scene_context->UpdateGlobalLightSSBOs(scene);
		scene_context->UpdateGlobalBoneSSBO(scene);

		// 4. Perform Camera Specific Rendering Functions

		glPolygonMode(GL_FRONT_AND_BACK, scene_context->Debug_RenderWireframe ? GL_LINE : GL_FILL);
		for (auto& camera_context : active_cameras)
		{
			if (!camera_context)
				continue;

			Entity camera_entity = scene->FindEntityByUUID(camera_context->CameraEntityUUID);
			if (!camera_entity || !camera_entity.HasComponent<CameraComponent>())
				continue;

			auto& camera_component = camera_entity.GetComponent<CameraComponent>();
			camera_component.CameraFramebuffer->Bind();
			{
				Renderer::ClearColour(camera_component.ClearColour);
				Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				// Render Passes
				camera_context->OnRender(scene, camera_component.CameraFramebuffer.get());

				camera_component.CameraFramebuffer->ResolveMultisampledFBO();
			}
			camera_component.CameraFramebuffer->Unbind();
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	void ForwardPlusPipeline::OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::OnRenderEditorScene - Overall");

		if (!scene)
		{
			L_CORE_ERROR("F+Pipeline::OnRenderEditorScene - Invalid Scene");
			return;
		}

		if (!editor_camera)
		{
			L_CORE_ERROR("F+Pipeline::OnRenderEditorScene - Invalid Editor Camera");
			return;
		}

		auto& scene_context = s_SceneContext[scene];

		// 1. Camera Identification and Validation
		if (scene_context->CameraContexts.count(EDITOR_CAMERA_HANDLE) == 0)
		{
			scene_context->CameraContexts[EDITOR_CAMERA_HANDLE] = std::make_unique<CameraContext>(scene, editor_camera);
		}

		std::vector<CameraContext*> active_cameras;
		active_cameras.push_back(scene_context->CameraContexts[EDITOR_CAMERA_HANDLE].get());
		active_cameras.back()->ValidateEditorCameraContext(scene, editor_camera);

		for (const auto& scene_camera : scene_cameras)
		{
			// Verify if Camera is Valid for Rendering
			if (!scene_camera || !scene_camera.HasComponent<CameraComponent>() || !scene_camera.GetComponent<CameraComponent>().CameraFramebuffer || !scene_camera.GetComponent<CameraComponent>().CameraInstance)
			{
				continue;
			}

			// Add Camera to Context List if Valid
			if(scene_context->CameraContexts.count(scene_camera.GetUUID()) == 0)
			{
				scene_context->CameraContexts[scene_camera.GetUUID()] = std::make_unique<CameraContext>(scene, &scene_camera, *scene_camera.GetComponent<CameraComponent>().CameraFramebuffer);
			}

			active_cameras.push_back(scene_context->CameraContexts[scene_camera.GetUUID()].get());
			active_cameras.back()->ValidateSceneCameraContext(scene);
		}

		// 2. Dispatch Threads - OctreeUpdate_Thread
		{
			if (scene_context->OctreeUpdate_Thread.joinable())
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::OnRenderEditorScene - Octree Thread Wait");
				scene_context->OctreeUpdate_Thread.join();
			}
			scene_context->OctreeUpdate_Thread = std::thread([&, scene]() -> void
				{
					scene_context->OnOctreeUpdate(scene);
				});
		}

		// 3. Perform Global Scene Functions

		Renderer::ClearRenderStats();

		scene_context->SortRenderQueue(scene, active_cameras);
		scene_context->UpdateGlobalShadowMaps(scene, active_cameras);
		scene_context->UpdateGlobalLightSSBOs(scene);
		scene_context->UpdateGlobalBoneSSBO(scene);

		// 4. Perform Camera Specific Rendering Functions

		glPolygonMode(GL_FRONT_AND_BACK, scene_context->Debug_RenderWireframe ? GL_LINE : GL_FILL);
		for (auto& camera_context : active_cameras)
		{
			if (!camera_context)
				continue;

			glm::vec4 clear_colour = { 0.1764f, 0.3294f, 0.5607f, 1.0f };
			FrameBuffer* camera_framebuffer = nullptr;
			if (camera_context->CameraEntityUUID == EDITOR_CAMERA_HANDLE)
			{
				camera_framebuffer = const_cast<FrameBuffer*>(&editor_camera->GetFrameBuffer());
			}
			else if(Entity camera_entity = scene->FindEntityByUUID(camera_context->CameraEntityUUID); camera_entity && camera_entity.HasComponent<CameraComponent>())
			{
				auto& camera_component = camera_entity.GetComponent<CameraComponent>();
				if (!camera_component.CameraFramebuffer)
					continue;

				camera_framebuffer = camera_component.CameraFramebuffer.get();
				clear_colour = camera_component.ClearColour;
			}

			if (!camera_framebuffer)
			{
				L_CORE_ERROR("F+Pipeline::OnRenderEditorScene - Could Not Render to Invalid Camera Framebuffer.");
				continue;
			}

			camera_framebuffer->Bind();
			{
				Renderer::ClearColour(clear_colour);
				Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				// Render Passes
				camera_context->OnRender(scene, camera_framebuffer);

				camera_framebuffer->ResolveMultisampledFBO();
			}
			camera_framebuffer->Unbind();
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	void ForwardPlusPipeline::OnViewportResize(Scene* scene, const glm::uvec2& viewport_size) 
	{
		if (!scene) 
		{
			L_CORE_ERROR("F+Pipeline::OnViewportResize - Invalid Scene!");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			Renderer::ClearColour({ 1.0f, 1.0f, 1.0f, 1.0f });
			Renderer::ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			return;
		}

		// Update Projection Matrix of All Scene Cameras
		auto camera_view = scene->GetAllEntitiesWith<CameraComponent>();
		for (const auto& entity : camera_view) 
		{
			CameraComponent& camera_component = camera_view.get<CameraComponent>(entity);

			glm::uvec2 final_size = {};
			final_size.x = static_cast<uint32_t>(viewport_size.x * camera_component.GetViewport().z);
			final_size.y = static_cast<uint32_t>(viewport_size.y * camera_component.GetViewport().w);

			camera_component.CameraFramebuffer->Resize(final_size);

			if (auto cam_ref = camera_component.CameraInstance; cam_ref) {
				cam_ref->SetViewportSize(final_size.x, final_size.y);
			}
		}

		s_SceneContext[scene]->UpdateComputeData(scene);
	}

	std::vector<Entity> ForwardPlusPipeline::GetActiveCameras(Scene* scene)
	{
		if (!scene || s_SceneContext.count(scene) == 0)
			return {};

		auto& scene_context = s_SceneContext[scene];

		std::vector<Entity> active_cameras;
		std::vector<std::reference_wrapper<CameraComponent>> sorted_cameras;

		auto camera_view = scene->GetAllEntitiesWith<CameraComponent>();

		// Collect cameras that have valid framebuffers
		for (const auto& entity_handle : camera_view)
		{
			auto& camera_component = camera_view.get<CameraComponent>(entity_handle);
			if (camera_component.CameraFramebuffer->IsValid())
			{
				sorted_cameras.push_back(camera_component);
			}
		}

		// Sort cameras by Depth (Highest First), then by Primary flag, then by viewport size
		std::sort(sorted_cameras.begin(), sorted_cameras.end(),
			[](const CameraComponent& a, const CameraComponent& b)
			{
				if (a.CameraDepth != b.CameraDepth)
					return a.CameraDepth > b.CameraDepth; // Higher depth first

				if (a.Primary != b.Primary)
					return a.Primary; // Primary cameras first

				float aSize = a.GetViewport().z * a.GetViewport().w; // Area of viewport
				float bSize = b.GetViewport().z * b.GetViewport().w;
				return aSize > bSize; // Larger viewports first
			});

		// Track cameras that should be rendered
		active_cameras.clear();

		// Iterate over sorted cameras, ensuring higher depth always renders
		for (size_t i = 0; i < sorted_cameras.size(); ++i)
		{
			auto& current_camera = sorted_cameras[i].get();
			bool is_occluded = false;

			// Only check occlusion against cameras with **lower depth**
			for (const auto& selected_camera : active_cameras)
			{
				if (selected_camera.GetComponent<CameraComponent>().CameraDepth >= current_camera.CameraDepth)
					continue; // Skip if the selected camera has equal or higher depth

				const glm::vec4& current_viewport = current_camera.GetViewport();
				const glm::vec4& selected_viewport = selected_camera.GetComponent<CameraComponent>().GetViewport();

				// Check if current camera is fully contained within a lower-depth camera
				if (current_viewport.x >= selected_viewport.x - 0.001f &&
					current_viewport.y >= selected_viewport.y - 0.001f &&
					current_viewport.x + current_viewport.z <= selected_viewport.x + selected_viewport.z + 0.001f &&
					current_viewport.y + current_viewport.w <= selected_viewport.y + selected_viewport.w + 0.001f)
				{
					is_occluded = true;
					break;
				}
			}

			if (!is_occluded)
			{
				Entity camera_entity = *current_camera.GetEntity();
				active_cameras.emplace_back(camera_entity);

				// Create Camera Context
				if (scene_context->CameraContexts.count(camera_entity.GetUUID()) == 0)
				{
					scene_context->CameraContexts[camera_entity.GetUUID()] =
						std::make_unique<CameraContext>(scene, &camera_entity,
							*camera_entity.GetComponent<CameraComponent>().CameraFramebuffer);
				}
			}
			else
			{
				// Remove cameras that are occluded and should not be rendered
				Entity camera_entity = *current_camera.GetEntity();
				if (scene_context->CameraContexts.count(camera_entity.GetUUID()) != 0)
				{
					scene_context->CameraContexts.erase(camera_entity.GetUUID());
				}
			}
		}


		// Swap Camera Render Queue Index
		// Remove Camera Context's of Camera's that have been removed from Scene
		for (auto it = scene_context->CameraContexts.begin(); it != scene_context->CameraContexts.end(); )
		{
			if (it->first == EDITOR_CAMERA_HANDLE)
			{
				++it;
				continue;
			}

			if (!scene->HasEntity(it->first))
			{
				it = scene_context->CameraContexts.erase(it);
				continue;
			}
			++it;
		}

		return active_cameras;
	}

	ForwardPlusPipeline::SceneContext* ForwardPlusPipeline::GetSceneContext(Scene* scene)
	{
		if (s_SceneContext.count(scene) != 0)
			return s_SceneContext[scene].get();
		return nullptr;
	}

	//
	//	--- CAMERA CONTEXT ---
	//

	#pragma region Camera Context

	ForwardPlusPipeline::CameraContext::CameraContext(Scene* scene, EditorCamera* editor_camera)
	{
		if (editor_camera)
		{
			CameraEntityUUID = EDITOR_CAMERA_HANDLE;
			Projection_Matrix = editor_camera->GetProjection();
			View_Matrix = editor_camera->GetViewMatrix();
			Position = editor_camera->GetPosition();
			NearClip = editor_camera->GetNearClip();
			FarClip = editor_camera->GetFarClip();

			CameraFrustum.RecalculateFrustum(Projection_Matrix * View_Matrix);

			InitCameraContext(editor_camera->GetFrameBuffer());
		}
		else
		{
			L_CORE_ERROR("Could Not Create Camera Context - Invalid Editor Camera.");
		}
	}

	ForwardPlusPipeline::CameraContext::CameraContext(Scene* scene, const Entity* camera_entity, const FrameBuffer& frame_buffer)
	{
		if (camera_entity)
		{
			CameraEntityUUID = camera_entity->GetUUID();
			View_Matrix = glm::inverse(camera_entity->GetTransform().GetGlobalTransform());
			Position = camera_entity->GetTransform().GetGlobalPosition();

			if (camera_entity->HasComponent<CameraComponent>())
			{
				auto& camera_component = camera_entity->GetComponent<CameraComponent>();

				// Just Make Sure Camera FrameBuffer & Camera Instance Has Correct ScreenSpace Dimensions
				auto& scene_framebuffer_config = scene->GetSceneFrameBuffer()->GetConfig();
				camera_component.SetViewport(camera_component.GetViewport(), { scene_framebuffer_config.Width, scene_framebuffer_config.Height });

				Projection_Matrix = camera_entity->GetComponent<CameraComponent>().CameraInstance->GetProjection();
				CameraFrustum.RecalculateFrustum(Projection_Matrix * View_Matrix);

				NearClip = camera_component.CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Orthographic ? camera_component.CameraInstance->GetOrthographicNearClip() : camera_component.CameraInstance->GetPerspectiveNearClip();
				FarClip = camera_component.CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Orthographic ? camera_component.CameraInstance->GetOrthographicFarClip() : camera_component.CameraInstance->GetPerspectiveFarClip();

			}
			InitCameraContext(frame_buffer);
		}
		else
		{
			L_CORE_ERROR("Could Not Create Camera Context - Invalid Camera Entity.");
		}
	}

	void ForwardPlusPipeline::CameraContext::InitCameraContext(const FrameBuffer& frame_buffer)
	{
		FrameBufferSize.x = frame_buffer.GetConfig().Width;
		FrameBufferSize.y = frame_buffer.GetConfig().Height;

		Entities_Visible.reserve(1024);
		PointLights_FrustumVisible.reserve(MAX_POINT_LIGHTS);
		SpotLights_FrustumVisible.reserve(MAX_SPOT_LIGHTS);
		DirectionalLights.reserve(MAX_DIRECTIONAL_LIGHTS);

		// Calculate workgroups and generate SSBOs from screen size
		TileWorkGroups.x = (unsigned int)std::ceil((float)FrameBufferSize.x / 16.0f);
		TileWorkGroups.y = (unsigned int)std::ceil((float)FrameBufferSize.y / 16.0f);

		size_t num_of_tiles = static_cast<size_t>(TileWorkGroups.x * TileWorkGroups.y);

		glGenBuffers(1, &PointLight_Indices_Buffer);						// Light Indices
		glGenBuffers(1, &SpotLight_Indices_Buffer);							// Light Indices		
		glGenBuffers(1, &DirectionalLight_Shadow_Matrix_Buffer);			// Shadow - cascade light space matrices		

		// Point Indices
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLight_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

		// Spot Indices
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

		// Directional Shadow Maps
		{
			// Directional Shadow Matrice + Cascade Plane Distances
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, DirectionalLight_Shadow_Matrix_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRE_LIGHT_SHADOW_MAPS * 5 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

			// DIRECTIONAL SHADOWS - Texture Array
			glGenTextures(1, &DirectionalLight_Shadow_Texture_Array);
			glBindTexture(GL_TEXTURE_2D_ARRAY, DirectionalLight_Shadow_Texture_Array);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION, MAX_DIRE_LIGHT_SHADOW_MAPS * 5); // Max 25 shadow maps, 5 cascades per light, max 5 shadow directional lights


			// DIRECTIONAL SHADOWS - Frame Buffer
			glGenFramebuffers(1, &DirectionalLight_Shadow_FrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, DirectionalLight_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DirectionalLight_Shadow_Texture_Array, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}
	}

	ForwardPlusPipeline::CameraContext::~CameraContext()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glDeleteBuffers(1, &PointLight_Indices_Buffer);
		glDeleteBuffers(1, &SpotLight_Indices_Buffer);
		glDeleteBuffers(1, &DirectionalLight_Shadow_Matrix_Buffer);

		glDeleteFramebuffers(1, &DirectionalLight_Shadow_FrameBuffer);
		glDeleteTextures(1, &DirectionalLight_Shadow_Texture_Array);
	}

	void ForwardPlusPipeline::CameraContext::ValidateSceneCameraContext(Scene* scene)
	{
		if (!scene)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::ValidateSceneCameraContext - Failed to Validate Editor Camera Context, Invalid Scene Pointer.");
			return;
		}

		Entity camera_entity = scene->FindEntityByUUID(CameraEntityUUID);
		if (!camera_entity || !camera_entity.HasComponent<CameraComponent>())
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::ValidateSceneCameraContext - Failed to Validate Camera Context, Invalid Camera Entity or Does Not Have Camera Component.");
			return;
		}

		auto& camera_component = camera_entity.GetComponent<CameraComponent>();

		Projection_Matrix = camera_component.CameraInstance->GetProjection();
		View_Matrix = glm::inverse(camera_entity.GetTransform().GetGlobalTransform());
		CameraFrustum.RecalculateFrustum(Projection_Matrix * View_Matrix);

		Position = camera_entity.GetTransform().GetGlobalPosition();

		NearClip = camera_component.CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Orthographic ? camera_component.CameraInstance->GetOrthographicNearClip() : camera_component.CameraInstance->GetPerspectiveNearClip();
		FarClip = camera_component.CameraInstance->GetProjectionType() == SceneCamera::ProjectionType::Orthographic ? camera_component.CameraInstance->GetOrthographicFarClip() : camera_component.CameraInstance->GetPerspectiveFarClip();

		glm::uvec2 new_framebuffer_size = { camera_component.CameraFramebuffer->GetConfig().Width, camera_component.CameraFramebuffer->GetConfig().Height };
		if (FrameBufferSize != new_framebuffer_size)
		{
			FrameBufferSize = new_framebuffer_size;

			// Calculate Workgroups and Generate SSBOs from Screen Size
			TileWorkGroups.x = static_cast<unsigned int>(std::ceil(static_cast<float>(FrameBufferSize.x / 16.0f)));
			TileWorkGroups.y = static_cast<unsigned int>(std::ceil(static_cast<float>(FrameBufferSize.y / 16.0f)));

			size_t num_of_tiles = static_cast<size_t>(TileWorkGroups.x * TileWorkGroups.y);

			// Update Light Indice Buffers
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void ForwardPlusPipeline::CameraContext::ValidateEditorCameraContext(Scene* scene, EditorCamera* editor_camera)
	{
		if (!scene)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::ValidateEditorCameraContext - Failed to Validate Editor Camera Context, Invalid Scene Pointer.");
			return;
		}

		if (!editor_camera)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::ValidateEditorCameraContext - Failed to Validate Editor Camera Context, Invalid Editor Camera Pointer.");
			return;
		}

		Projection_Matrix = editor_camera->GetProjection();
		View_Matrix = editor_camera->GetViewMatrix();
		CameraFrustum.RecalculateFrustum(Projection_Matrix * View_Matrix);

		Position = editor_camera->GetPosition();

		NearClip = editor_camera->GetNearClip();
		FarClip = editor_camera->GetFarClip();

		glm::uvec2 new_framebuffer_size = { editor_camera->GetFrameBuffer().GetConfig().Width, editor_camera->GetFrameBuffer().GetConfig().Height };
		if (FrameBufferSize != new_framebuffer_size)
		{
			FrameBufferSize = new_framebuffer_size;

			// Calculate Workgroups and Generate SSBOs from Screen Size
			TileWorkGroups.x = static_cast<unsigned int>(std::ceil(static_cast<float>(FrameBufferSize.x / 16.0f)));
			TileWorkGroups.y = static_cast<unsigned int>(std::ceil(static_cast<float>(FrameBufferSize.y / 16.0f)));

			size_t num_of_tiles = static_cast<size_t>(TileWorkGroups.x * TileWorkGroups.y);

			// Update Light Indice Buffers
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void ForwardPlusPipeline::CameraContext::OnRender(Scene* scene, FrameBuffer* camera_framebuffer)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::OnRender - OnRender");

		if (!camera_framebuffer)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::OnRender - Could Not Render to Invalid Camera Framebuffer.");
			return;
		}

		auto& scene_context = s_SceneContext[scene];

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointLight_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SpotLight_Indices_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, DirectionalLight_Shadow_Matrix_Buffer);

		DepthPass(scene, camera_framebuffer);

		Light_ComputeTileBasedCull(scene, camera_framebuffer);
		Light_UpdateCameraContextSSBOs(scene);

		RenderPass(scene, camera_framebuffer);
	}

	void ForwardPlusPipeline::CameraContext::DepthPass(Scene* scene, FrameBuffer* camera_framebuffer)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::DepthPass - Depth Pass");

		if (!scene)
			return;

		if (!camera_framebuffer)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::DepthPass - Could Not Render Camera Depth, Invalid Camera Framebuffer.");
			return;
		}

		auto& scene_context = s_SceneContext[scene];

		if (Depth_RenderQueue.empty())
			return;
		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		camera_framebuffer->ClearEntityPixelData(NULL_UUID);

		if (auto shader = AssetManager::GetInbuiltShader("FP_Depth"); shader)
		{
			shader->Bind();
			camera_framebuffer->Bind();
			camera_framebuffer->BindEntitySSBO();

			shader->SetMat4("u_Proj", Projection_Matrix);
			shader->SetMat4("u_View", View_Matrix);
			shader->SetIntVec2("u_ScreenSize", glm::ivec2(FrameBufferSize));

			for (auto& [distance, entity] : Depth_RenderQueue)
			{
				if (!entity) continue;

				shader->SetMat4("u_Model", entity.GetTransform().GetGlobalTransform());
				shader->SetUInt("u_EntityID", entity.GetUUID());

				AssetHandle asset_mesh_handle = NULL_UUID;

				if (entity.HasComponent<MeshFilterComponent>())
					asset_mesh_handle = entity.GetComponent<MeshFilterComponent>().StaticMeshHandle;

				if (entity.HasComponent<SkinnedMeshComponent>())
					asset_mesh_handle = entity.GetComponent<SkinnedMeshComponent>().StaticMeshHandle;

				// Check if Asset Handle is Valid
				if (!AssetManager::IsAssetHandleValid(asset_mesh_handle))
					continue;

				// Retrieve Cached Mesh Asset
				auto mesh_asset = scene_context->CachedMeshes[asset_mesh_handle].lock();

				// Check if Loaded
				if (!mesh_asset)
				{
					// If Not Loaded, Call GetAsset to Load
					scene_context->CachedMeshes[asset_mesh_handle] = AssetManager::GetAsset<StaticMesh>(asset_mesh_handle);
					mesh_asset = scene_context->CachedMeshes[asset_mesh_handle].lock();

					// If Failed to Load - Continue
					if (!mesh_asset)
						continue;
				}

				// Gather Appropriate Material Vector
				std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> material_handles;
				if (entity.HasComponent<MeshRendererComponent>()) {
					material_handles = entity.GetComponent<MeshRendererComponent>().MaterialHandles;
				}
				else if (entity.HasComponent<SkinnedMeshComponent>()) {
					material_handles = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;
				}

				if (material_handles.empty())
					continue;

				// GET FINAL BONE MATRICES FOR SKELETONS
				bool skinned = false;
				if (entity.HasComponent<SkinnedMeshComponent>())
				{
					auto index_found = scene_context->BoneTransform_Offset.find(entity.GetUUID());
					if (index_found != scene_context->BoneTransform_Offset.end())
					{
						skinned = true;
						shader->SetBool("u_Skinned", true);
						shader->SetUInt("u_BoneOffset", index_found->second);
						shader->SetUInt("u_BoneCount", static_cast<uint32_t>(entity.GetComponent<SkinnedMeshComponent>().FinalBoneTransformations.size()));
					}
				}

				const UUID& entity_uuid = entity.GetUUID();
				if (EntityOcclusionQueries.count(entity_uuid) == 0)
					EntityOcclusionQueries[entity_uuid] = Query(Query::Type::AnySamplesPassed);

				if (EntityOcclusionHistory.count(entity_uuid) == 0)
					EntityOcclusionHistory[entity_uuid] = 5;

				bool conduct_query = !EntityOcclusionQueries[entity_uuid].IsProcessing();
				if (conduct_query)
					EntityOcclusionQueries[entity_uuid].Begin();

				// Should I draw this to the depth map? How can I determine 
				// whether to draw this to the depth map or not?
				for (int i = 0; i < mesh_asset->SubMeshes.size(); i++)
				{
					auto& material_asset_handle = i < material_handles.size() ? material_handles[i].first : material_handles.back().first;
					auto asset_material = scene_context->CachedMaterials[material_asset_handle].lock();

					if (!asset_material)
					{
						// If Not Loaded, Call GetAsset to Load
						scene_context->CachedMaterials[material_asset_handle] = AssetManager::GetAsset<Material>(material_asset_handle);
						asset_material = scene_context->CachedMaterials[material_asset_handle].lock();

						// If Failed to Load - Continue
						if (!asset_material)
							continue;
					}

					bool disable_depth = (asset_material->GetRenderType() != RenderType::L_MATERIAL_OPAQUE);
					if (disable_depth) glDepthMask(GL_FALSE);

					Renderer::DrawSubMesh(mesh_asset->SubMeshes[i], true);

					if (disable_depth) glDepthMask(GL_TRUE);
				}

				// Render AABB bounding box to determine visibility for occlusion query
				if (conduct_query)
					EntityOcclusionQueries[entity_uuid].End();

				if (skinned)
					shader->SetBool("u_Skinned", false);
			}

			camera_framebuffer->UnBindEntitySSBO();
			camera_framebuffer->Unbind();
			shader->UnBind();
		}
		else
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::DepthPass - FP Depth Shader Not Found.");
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glFlush();
	}

	void ForwardPlusPipeline::CameraContext::RenderPass(Scene* scene, FrameBuffer* camera_framebuffer)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::RenderPass - Colour Pass");

		if (!scene)
			return;

		if (!camera_framebuffer)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::RenderPass - Could Not Render Camera Colour, Invalid Camera Framebuffer.");
			return;
		}

		auto& scene_context = s_SceneContext[scene];

		// Skybox
		std::shared_ptr<SkyboxMaterial> skybox_material = nullptr;

		if (CameraEntityUUID == EDITOR_CAMERA_HANDLE)
		{
			// If editor camera, just use the skybox of the primary scene camera
			Entity primary_camera_entity = scene->GetPrimaryCameraEntity();
			if (primary_camera_entity && primary_camera_entity.GetComponent<CameraComponent>().ClearFlags == CameraClearFlags::SKYBOX && primary_camera_entity.HasComponent<SkyboxComponent>())
			{
				skybox_material = AssetManager::GetAsset<SkyboxMaterial>(primary_camera_entity.GetComponent<SkyboxComponent>().SkyboxMaterialAssetHandle);
			}
		}
		else if (Entity camera_entity = scene->FindEntityByUUID(CameraEntityUUID); camera_entity && camera_entity.HasComponent<CameraComponent>() && camera_entity.HasComponent<SkyboxComponent>())
		{
			if (camera_entity.GetComponent<CameraComponent>().ClearFlags == CameraClearFlags::SKYBOX)
			{
				skybox_material = AssetManager::GetAsset<SkyboxMaterial>(camera_entity.GetComponent<SkyboxComponent>().SkyboxMaterialAssetHandle);
			}
		}

		// RENDER SOME TRIANGLES HEHE!
		camera_framebuffer->Bind();

		if (skybox_material && skybox_material->Bind()) {

			// Save the current depth function
			GLenum originalDepthFunc{};
			glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&originalDepthFunc);

			// Change depth function to GL_ALWAYS for the skybox
			glDepthFunc(GL_ALWAYS);

			auto shader_ref = skybox_material->GetShader();

			shader_ref->SetMat4("u_VertexIn.Proj", Projection_Matrix);
			shader_ref->SetMat4("u_VertexIn.View", glm::mat4(glm::mat3(View_Matrix))); // Strip Scaling
			skybox_material->UpdateUniforms();

			Renderer::DrawSkybox();

			skybox_material->UnBind();

			// Restore the original depth function after the skybox is rendered
			glDepthFunc(originalDepthFunc);
			
		}
		
		// Opaque
		if (!Opaque_RenderQueue.empty())
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::RenderPass - Opaque Queue");

			for (const auto& [material_wrapper_pair, mesh_map] : Opaque_RenderQueue)
			{
				const auto& material_asset = material_wrapper_pair.material;

				if (!material_asset)
					continue;

				if (!material_asset->Bind())
					continue;

				auto shader = material_asset->GetShader();
				if (shader->IsValid())
				{
					material_asset->UpdateUniforms(material_wrapper_pair.uniform_block); // Change

					// Update Specific Forward Plus Uniforms
					shader->SetInt("u_TilesX", TileWorkGroups.x);
					shader->SetInt("u_ShowLightComplexity", s_SceneContext[scene]->Debug_ShowLightComplexity);

					shader->SetFloat("u_Near", NearClip);
					shader->SetFloat("u_Far", FarClip);

					shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					shader->SetMat4("u_VertexIn.View", View_Matrix);
					shader->SetFloatVec3("u_CameraPos", Position);

					shader->SetIntVec2("u_ScreenSize", glm::ivec2(FrameBufferSize));

					if (camera_framebuffer->IsMultiSampled())
					{
						shader->SetInt("u_Samples", camera_framebuffer->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, camera_framebuffer->GetMultiSampledTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth_MS", 3);
					}
					else
					{
						shader->SetInt("u_Samples", camera_framebuffer->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D, camera_framebuffer->GetTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth", 3);

					}

					// Texture binding 4 is dedicated for Point Light Shadow Cube Map Array
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, scene_context->PointLight_Shadow_CubeMapArray);
					shader->SetInt("u_PL_ShadowCubeMapArray", 4);

					// Texture binding 5 is dedicated for Directional Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D_ARRAY, DirectionalLight_Shadow_Texture_Array);
					shader->SetInt("u_DL_ShadowMapArray", 5);

					// Texture binding 6 is dedicated for Spot Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D_ARRAY, scene_context->SpotLight_Shadow_TextureArray);
					shader->SetInt("u_SL_ShadowMapArray", 6);
				}
				else
				{
					shader = AssetManager::GetInbuiltShader("Invalid Shader");
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, AssetManager::GetInbuiltAsset<Texture2D>("Invalid Checkered Texture")->GetID());
					shader->SetInt("u_InvalidTexture", 0);
					shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					shader->SetMat4("u_VertexIn.View", View_Matrix);
				}

				for (const auto& [sub_mesh, entities] : mesh_map)
				{
					size_t entity_count = entities.size();
					if (entity_count == 0)
						continue;

					bool use_instance_data = (entity_count > 1);
					std::vector<Entity> deferred_entities_from_instanced{};

					if (use_instance_data)
					{
						std::vector<glm::mat4> transforms;
						transforms.reserve(entity_count);

						for (const auto& entity : entities)
						{
							if (!entity)
								continue;

							// Separate Skinned Meshes from Instance Batches
							if (entity.HasComponent<SkinnedMeshComponent>())
							{
								deferred_entities_from_instanced.emplace_back(entity);
								continue;
							}

							const auto& transform = entity.GetTransform().GetGlobalTransform();
							transforms.emplace_back(transform);
						}

						shader->SetBool("u_UseInstanceData", true);
						Renderer::DrawInstancedSubMesh(sub_mesh, transforms);
					}
					else
					{
						Entity entity = entities[0];

						if (!entity)
							continue;

						bool skinned = false;
						if (entity.HasComponent<SkinnedMeshComponent>())
						{
							auto index_found = scene_context->BoneTransform_Offset.find(entity.GetUUID());
							if (index_found != scene_context->BoneTransform_Offset.end())
							{
								skinned = true;
								shader->SetBool("u_Skinned", true);
								shader->SetUInt("u_BoneOffset", index_found->second);
								shader->SetUInt("u_BoneCount", static_cast<uint32_t>(entity.GetComponent<SkinnedMeshComponent>().FinalBoneTransformations.size()));

							}
						}

						const auto& transform = entity.GetTransform().GetGlobalTransform();
						shader->SetMat4("u_VertexIn.Model", transform);
						Renderer::DrawSubMesh(sub_mesh);

						if (skinned)
							shader->SetBool("u_Skinned", false);
					}

					// Process any that can't be instanced
					shader->SetBool("u_UseInstanceData", false);
					for (const auto& entity : deferred_entities_from_instanced)
					{
						bool skinned = false;
						if (entity.HasComponent<SkinnedMeshComponent>())
						{
							auto index_found = scene_context->BoneTransform_Offset.find(entity.GetUUID());
							if (index_found != scene_context->BoneTransform_Offset.end())
							{
								skinned = true;
								shader->SetBool("u_Skinned", true);
								shader->SetUInt("u_BoneOffset", index_found->second);
								shader->SetUInt("u_BoneCount", static_cast<uint32_t>(entity.GetComponent<SkinnedMeshComponent>().FinalBoneTransformations.size()));
							}
						}

						const auto& transform = entity.GetTransform().GetGlobalTransform();
						shader->SetMat4("u_VertexIn.Model", transform);
						Renderer::DrawSubMesh(sub_mesh);

						if (skinned)
							shader->SetBool("u_Skinned", false);
					}
				}
			}
		}

		// Transparent
		if (!Transparent_RenderQueue.empty())
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::RenderPass - Transparent Queue");

			glDisable(GL_CULL_FACE);

			// Enable Blending During Transparency Pass
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Render Transparent Objects Back to Front One By One....
			for (const auto& [distance, material_wrapper_pair, sub_mesh, entity] : Transparent_RenderQueue)
			{
				if (!entity) continue;

				const auto& material_asset = material_wrapper_pair.material;

				if (!material_asset)
					continue;

				if (!material_asset->Bind())
					continue;

				auto shader = material_asset->GetShader();
				if (shader->IsValid())
				{
					// Check to Disable Depth Writing During Transparent Pass
					// Depth Already Written During Depth/Colour Pass
					// We may not want to override current depth during transparent pass
					glDepthMask(material_asset->IsTransparencyWriteDepth());

					material_asset->UpdateUniforms(material_wrapper_pair.uniform_block);

					// Update Specific Forward Plus Uniforms
					shader->SetInt("u_TilesX", TileWorkGroups.x);
					shader->SetInt("u_ShowLightComplexity", s_SceneContext[scene]->Debug_ShowLightComplexity);

					shader->SetFloat("u_Near", NearClip);
					shader->SetFloat("u_Far", FarClip);

					shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					shader->SetMat4("u_VertexIn.View", View_Matrix);
					shader->SetFloatVec3("u_CameraPos", Position);

					shader->SetIntVec2("u_ScreenSize", glm::ivec2(FrameBufferSize));

					// Texture binding 3 is dedicated for depth map
					if (camera_framebuffer->IsMultiSampled())
					{
						shader->SetInt("u_Samples", camera_framebuffer->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, camera_framebuffer->GetMultiSampledTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth_MS", 3);

					}
					else
					{
						shader->SetInt("u_Samples", camera_framebuffer->GetConfig().Samples);
						glActiveTexture(GL_TEXTURE3);
						glBindTexture(GL_TEXTURE_2D, camera_framebuffer->GetTexture(FrameBufferTexture::DepthTexture));
						shader->SetInt("u_Depth", 3);

					}

					// Texture binding 4 is dedicated for Point Light Shadow Cube Map Array
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, scene_context->PointLight_Shadow_CubeMapArray);
					shader->SetInt("u_PL_ShadowCubeMapArray", 4);

					// Texture binding 5 is dedicated for Directional Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D_ARRAY, DirectionalLight_Shadow_Texture_Array);
					shader->SetInt("u_DL_ShadowMapArray", 5);

					// Texture binding 6 is dedicated for Spot Light Shadow Texture Array
					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D_ARRAY, scene_context->SpotLight_Shadow_TextureArray);
					shader->SetInt("u_SL_ShadowMapArray", 6);
				}
				else
				{
					shader = AssetManager::GetInbuiltShader("Invalid Shader");
					shader->Bind();

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, AssetManager::GetInbuiltAsset<Texture2D>("Invalid Checkered Texture")->GetID());
					shader->SetInt("u_InvalidTexture", 0);

					shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					shader->SetMat4("u_VertexIn.View", View_Matrix);
				}

				shader->SetBool("u_UseInstanceData", false);

				bool skinned = false;
				if (entity.HasComponent<SkinnedMeshComponent>())
				{
					auto index_found = scene_context->BoneTransform_Offset.find(entity.GetUUID());
					if (index_found != scene_context->BoneTransform_Offset.end())
					{
						skinned = true;
						shader->SetBool("u_Skinned", true);
						shader->SetUInt("u_BoneOffset", index_found->second);
						shader->SetUInt("u_BoneCount", static_cast<uint32_t>(entity.GetComponent<SkinnedMeshComponent>().FinalBoneTransformations.size()));
					}
				}

				const auto& transform = entity.GetTransform().GetGlobalTransform();
				shader->SetMat4("u_VertexIn.Model", transform);

				Renderer::DrawSubMesh(sub_mesh);

				if (skinned)
					shader->SetBool("u_Skinned", false);
			}

			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glDisable(GL_BLEND);
		}

		// Debug Lines
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::RenderPass - Debug Queue");

			static std::vector<glm::mat4> bounds_matricies{};
			if (bounds_matricies.capacity() == 0)
				bounds_matricies.reserve(1024);
			else
				bounds_matricies.clear();

			// Octree Display Enabled
			if (auto octree_ref = scene->GetOctree().lock(); octree_ref && scene->GetDisplayOctree()) {

				// Draw Octree
				auto debug_line_shader = AssetManager::GetInbuiltShader("Debug_Line_Draw");
				if (debug_line_shader) 
				{
					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 1.0f, 0.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", View_Matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					Renderer::DrawInstancedDebugCube(octree_ref->GetAllOctreeBoundsMat4());

					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", View_Matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					// Draw All Bounds of Data Sources in Octree
					for (auto& entity : Entities_Visible) 
					{
						if (!scene->ValidEntity(entity)) continue;

						if (entity.HasComponent<MeshFilterComponent>())
							bounds_matricies.emplace_back(entity.GetComponent<MeshFilterComponent>().TransformedAABB.GetGlobalBoundsMat4());

						if (entity.HasComponent<SkinnedMeshComponent>())
							bounds_matricies.emplace_back(entity.GetComponent<SkinnedMeshComponent>().TransformedAABB.GetGlobalBoundsMat4());
					}

					Renderer::DrawInstancedDebugCube(bounds_matricies);

					debug_line_shader->UnBind();
				}
			}
			// Only Draw Debug MeshFilter AABB's
			else if (!Debug_RenderAABBs.empty())
			{
				auto debug_line_shader = AssetManager::GetInbuiltShader("Debug_Line_Draw");
				if (debug_line_shader)
				{
					debug_line_shader->Bind();
					debug_line_shader->SetFloatVec4("u_LineColor", { 0.0f, 1.0f, 0.0f, 1.0f });
					debug_line_shader->SetMat4("u_VertexIn.Proj", Projection_Matrix);
					debug_line_shader->SetMat4("u_VertexIn.View", View_Matrix);
					debug_line_shader->SetBool("u_UseInstanceData", true);

					Renderer::DrawInstancedDebugCube(Debug_RenderAABBs);

					debug_line_shader->UnBind();
				}
			}
		}

		camera_framebuffer->Unbind();
	}

	void ForwardPlusPipeline::CameraContext::Light_FrustumBasedCull(Scene* scene, SceneContext* scene_context)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Lights Frustum Cull");

		if (!scene || !scene_context)
			return;

		PointLights_FrustumVisible.clear();
		SpotLights_FrustumVisible.clear();
		DirectionalLights.clear();

		auto pl_view = scene->GetAllEntitiesWith<PointLightComponent>();
		for (const auto& entity_handle : pl_view) {

			if (PointLights_FrustumVisible.size() >= MAX_POINT_LIGHTS)
				break;

			if (!pl_view.get<PointLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene };
			if (CameraFrustum.Contains({ entity.GetTransform().GetGlobalPosition(), entity.GetComponent<PointLightComponent>().Radius }) != FrustumContainResult::DoesNotContain)
			{
				PointLights_FrustumVisible.emplace_back(entity);
				scene_context->PointLight_OverallVisible.insert(entity.GetUUID());
			}
		}

		auto sl_view = scene->GetAllEntitiesWith<SpotLightComponent>();
		for (const auto& entity_handle : sl_view) {

			if (SpotLights_FrustumVisible.size() >= MAX_SPOT_LIGHTS)
				break;

			if (!sl_view.get<SpotLightComponent>(entity_handle).Active)
				continue;

			Entity entity = { entity_handle, scene };

			auto& transform = entity.GetTransform();
			auto& light = entity.GetComponent<SpotLightComponent>();

			float half_angle = glm::radians(light.Angle * 0.5f);
			float cosPenumbra = cos(half_angle);

			Bounds_Sphere sphere;
			if (half_angle > glm::pi<float>() / 4.0f) {
				sphere.BoundsCentre = transform.GetGlobalPosition() + cosPenumbra * light.Range * transform.GetForwardDirection();
				sphere.BoundsRadius = sin(half_angle) * light.Range;
			}
			else
			{
				sphere.BoundsCentre = transform.GetGlobalPosition() + light.Range / (2.0f * cosPenumbra) * transform.GetForwardDirection();
				sphere.BoundsRadius = light.Range / (2.0f * cosPenumbra);
			}

			if (CameraFrustum.Contains(sphere) != FrustumContainResult::DoesNotContain) {
				SpotLights_FrustumVisible.emplace_back(entity);
				scene_context->SpotLight_OverallVisible.insert(entity.GetUUID());
			}

		}

		auto dl_view = scene->GetAllEntitiesWith<DirectionalLightComponent>();
		for (const auto& entity_handle : dl_view) {

			if (DirectionalLights.size() >= MAX_DIRECTIONAL_LIGHTS)
				break;

			if (!dl_view.get<DirectionalLightComponent>(entity_handle).Active)
				continue;

			DirectionalLights.emplace_back(entity_handle, scene);
		}
	}

	void ForwardPlusPipeline::CameraContext::Light_ComputeTileBasedCull(Scene* scene, FrameBuffer* camera_framebuffer) const 
	{		
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::Light_ComputeTileBasedCull - Light Compute Tile Based Cull");

		if (!scene)
			return;

		if (!camera_framebuffer)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::Light_ComputeTileBasedCull - Could Not Compute Tile Based Cull, Invalid Camera Framebuffer.");
			return;
		}

		// Conduct Light Cull
		std::shared_ptr<Shader> lightCull = AssetManager::GetInbuiltShader("FP_Light_Culling", true);
		if (!lightCull)
		{
			L_CORE_ERROR("F+Pipeline::CameraContext::Light_ComputeTileBasedCull - FP Light Cull Compute Shader Not Found.");
			return;
		}

		lightCull->Bind();
		lightCull->SetMat4("u_Proj", Projection_Matrix);		
		lightCull->SetMat4("u_View", View_Matrix);
		
		lightCull->SetIntVec2("u_ScreenSize", glm::ivec2(FrameBufferSize));
		
		// Bind depth to texture 3 so this does not interfere with any 
		// diffuse, normal, or specular textures used 
		// 
		// Texture binding 3 is dedicated for depth map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, camera_framebuffer->GetTexture(FrameBufferTexture::DepthTexture));
		lightCull->SetInt("u_Depth", 3);
		
		glDispatchCompute(TileWorkGroups.x, TileWorkGroups.y, 1);
		
		glFlush();
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	void ForwardPlusPipeline::CameraContext::Light_UpdateCameraContextSSBOs(Scene* scene)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::CameraContext::Light_UpdateCameraContextSSBOs - Light Update Light SSBO");

		if (!scene || s_SceneContext.count(scene) == 0)
			return;

		auto& scene_context = s_SceneContext[scene];
		// Directional Lights
		{
			static std::vector<SSBOLightStructs::DL_SSBO_DATA_LAYOUT> s_DirectionalLightVector(MAX_DIRECTIONAL_LIGHTS);
			s_DirectionalLightVector.clear();

			// Add lights to vector that are contained within the scene up to a maximum of 10
			for (auto& entity : DirectionalLights) {

				if (s_DirectionalLightVector.size() >= MAX_DIRECTIONAL_LIGHTS)
					break;

				auto& directional_light = entity.GetComponent<DirectionalLightComponent>();
				if (directional_light.Active)
				{
					s_DirectionalLightVector.push_back({ directional_light, entity.GetTransform() });

					if (DirectionalLight_Shadow_LightIndices.find(entity.GetUUID()) != DirectionalLight_Shadow_LightIndices.end())
						s_DirectionalLightVector.back().shadowLightIndex = DirectionalLight_Shadow_LightIndices.at(entity.GetUUID());

					if (DirectionalLight_Shadow_CascadeIndices.find(entity.GetUUID()) != DirectionalLight_Shadow_CascadeIndices.end())
						s_DirectionalLightVector.back().shadowCascadePlaneDistances = DirectionalLight_Shadow_CascadeIndices.at(entity.GetUUID());
				}
			}

			// Create Buffer Light at End of Vector if not full
			if (s_DirectionalLightVector.size() < MAX_DIRECTIONAL_LIGHTS) {
				DirectionalLightComponent temp_last_light;
				s_DirectionalLightVector.push_back(temp_last_light);
				s_DirectionalLightVector.back().lastLight = true;
			}

			// Update SSBO data with light data
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene_context->DirectionalLight_Data_Buffer);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_DirectionalLightVector.size() * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), s_DirectionalLightVector.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void ForwardPlusPipeline::CameraContext::Renderable_FrustumCull(Scene* scene, SceneContext* scene_context)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Renderable Frustum Cull");

		if (!scene || !scene_context)
			return;

		size_t entity_counter{};
		if (auto oct_ref = scene->GetOctree().lock(); oct_ref)
		{

			std::unique_lock lock(oct_ref->GetOctreeMutex());
			const auto& query_vec = oct_ref->Query(CameraFrustum);

			entity_counter = oct_ref->TotalCount();

			if (Entities_Visible.capacity() == 0)
				Entities_Visible.reserve(1024);

			if (query_vec.size() > Entities_Visible.capacity())
				Entities_Visible.reserve(Entities_Visible.capacity() * 2);

			Entities_Visible.clear();

			for (const auto& data : query_vec)
			{
				if (!data->Data)
					continue;

				if (data->Data.HasComponent<MeshRendererComponent>())
				{
					auto& component = data->Data.GetComponent<MeshRendererComponent>();
					if (component.Active)
					{
						Entities_Visible.emplace_back(data->Data);
					}
				}

				if (data->Data.HasComponent<SkinnedMeshComponent>())
				{
					auto& component = data->Data.GetComponent<SkinnedMeshComponent>();
					if (component.Active)
					{
						Entities_Visible.push_back(data->Data);
					}
				}
			}
		}

		// LOD Sorting
		const float& far_plane = FarClip;
		auto view = scene->GetAllEntitiesWith<LODMeshComponent>();
		for (auto& entity_handle : view)
		{
			Entity lod_entity = { entity_handle, scene };

			if (!lod_entity)
				continue;

			if (std::find_if(Entities_Visible.begin(),
				Entities_Visible.end(),
				[&](const Entity& visibleEntity)
				{
					return visibleEntity == lod_entity;
				})
				!= Entities_Visible.end())
			{
				auto& lod_component = lod_entity.GetComponent<LODMeshComponent>();

				glm::vec3 position = lod_entity.GetTransform().GetGlobalPosition();
				float distance_to_lod_entity = glm::distance(Position, position);

				// Normalise the distance within the frustum (0.0 = near plane, 1.0 = far plane)
				float max_distance = (lod_component.MaxDistanceOverFarPlane) ? lod_component.MaxDistance : FarClip;
				float normalised_distance = distance_to_lod_entity / max_distance;

				// Find the correct LOD level to keep based on normalised distance
				int keep_lod_index = -1;
				for (int i = 0; i < lod_component.LOD_Elements.size(); i++)
				{
					if (normalised_distance <= lod_component.LOD_Elements[i].DistanceThresholdNormalised)
					{
						keep_lod_index = i;
						break;
					}
				}

				for (int i = 0; i < lod_component.LOD_Elements.size(); i++)
				{
					if (i == keep_lod_index)
						continue;

					for (const auto& entity_handle : lod_component.LOD_Elements[i].MeshRendererEntities)
					{
						if (entity_handle == NULL_UUID)
							continue;

						Entities_Visible.erase(
							std::remove(Entities_Visible.begin(), Entities_Visible.end(), scene->FindEntityByUUID(entity_handle)), // Shift to end of vector
							Entities_Visible.end()); // Erase end element which is element just shifted there
					}
				}
			}
		}

		Renderer::s_RenderStats.Entities_Culled_Frustum += static_cast<GLuint>(entity_counter - Entities_Visible.size());
	}

	void ForwardPlusPipeline::CameraContext::Renderable_OcclusionCull(Scene* scene, SceneContext* scene_context)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Renderable Occlusion Cull");

		if (!scene || !scene_context)
			return;

		// Occlusion Culling Checks
		size_t entity_counter = Entities_Visible.size();
		for (auto it = EntityOcclusionQueries.begin(); it != EntityOcclusionQueries.end();)
		{
			Entity entity = scene->FindEntityByUUID(it->first);
			if (!entity)
			{
				it = EntityOcclusionQueries.erase(it); // remove invalid entities from occlusion queries
				continue;
			}

			Query& query = it->second;

			bool result_available = query.IsResultAvailable();

			if (!result_available && EntityOcclusionHistory[it->first] > 0)
			{
				EntityOcclusionHistory[it->first] -= 1;
				++it;
				continue;
			}

			bool is_visible = query.GetResult() != GL_FALSE;

			EntityOcclusionHistory[it->first] = (is_visible) ? 5 : 0; // Result stays visible for atleast 5 frames

			auto find_in_frustum_culled = std::find_if(
				Entities_Visible.begin(),
				Entities_Visible.end(),
				[&](Entity& entity_find) { return entity_find == entity; });

			if (find_in_frustum_culled != Entities_Visible.end())
			{
				if (!is_visible)  // Entity is occluded
					Entities_Visible.erase(find_in_frustum_culled);
			}
			else
			{
				if (EntityOcclusionHistory.count(it->first) != 0)
					EntityOcclusionHistory.erase(it->first);

				it = EntityOcclusionQueries.erase(it);
				continue;
			}
			++it;
		}

		Renderer::s_RenderStats.Entities_Culled_Occlusion += static_cast<GLuint>(entity_counter - Entities_Visible.size());
		Renderer::s_RenderStats.Entities_Culled_Remaining += static_cast<GLuint>(Entities_Visible.size());
	}

	void ForwardPlusPipeline::CameraContext::PrepareRenderPass_Depth(Scene* scene, SceneContext* scene_context)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Prepare Depth Pass");

		if (!scene || !scene_context)
			return;

		Depth_RenderQueue.clear();

		if (Entities_Visible.empty())
			return;

		for (auto& entity : Entities_Visible)
		{
			if (!scene->ValidEntity(entity))
				continue;

			const glm::vec3& objectPosition = entity.GetTransform().GetGlobalPosition();
			Bounds_AABB object_bounds;

			if (entity.HasComponent<MeshFilterComponent>())
			{
				object_bounds = entity.GetComponent<MeshFilterComponent>().TransformedAABB;
			}

			if (entity.HasComponent<SkinnedMeshComponent>())
			{
				object_bounds = entity.GetComponent<SkinnedMeshComponent>().TransformedAABB;
			}

			// Find distance from closest point of AABB from camera_position
			glm::vec3 camera_position = Position;
			float distance = glm::length(camera_position - object_bounds.ClosestPoint(camera_position));

			Depth_RenderQueue.emplace_back(distance, entity);
		}

		// Front-to-Back sorting
		std::sort(Depth_RenderQueue.begin(), Depth_RenderQueue.end(), [](const auto& a, const auto& b)
			{
				return std::get<0>(a) < std::get<0>(b);
			});
	}

	void ForwardPlusPipeline::CameraContext::PrepareRenderPass_Colour(Scene* scene, SceneContext* scene_context)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Prepare Colour Passes");

		if (!scene || !scene_context)
			return;

		Opaque_RenderQueue.clear();
		Transparent_RenderQueue.clear();

		if (Entities_Visible.empty())
			return;

		for (auto& entity : Entities_Visible)
		{
			if (!scene->ValidEntity(entity))
				continue;

			AssetHandle static_mesh_handle = NULL_UUID;

			if (entity.HasComponent<MeshFilterComponent>())
			{
				static_mesh_handle = entity.GetComponent<MeshFilterComponent>().StaticMeshHandle;
			}

			if (entity.HasComponent<SkinnedMeshComponent>())
			{
				static_mesh_handle = entity.GetComponent<SkinnedMeshComponent>().StaticMeshHandle;
			}

			// Check if Asset Handle is Valid
			if (!AssetManager::IsAssetHandleValid(static_mesh_handle))
				continue;

			// Retrieve Cached Mesh Asset
			auto mesh_asset = scene_context->CachedMeshes[static_mesh_handle].lock();

			// Check if Loaded
			if (!mesh_asset)
			{
				// If Not Loaded, Call GetAsset to Load
				scene_context->CachedMeshes[static_mesh_handle] = AssetManager::GetAsset<StaticMesh>(static_mesh_handle);
				mesh_asset = scene_context->CachedMeshes[static_mesh_handle].lock();

				// If Failed to Load - Continue
				if (!mesh_asset)
					continue;
			}

			// Retrieve Sub Meshes
			auto& sub_meshes = mesh_asset->SubMeshes;

			// Retrieve All MeshMaterialHandles
			std::vector<std::pair<AssetHandle, std::shared_ptr<MaterialUniformBlock>>> material_handles;
			if (entity.HasComponent<MeshRendererComponent>()) {
				material_handles = entity.GetComponent<MeshRendererComponent>().MaterialHandles;
			}
			else if (entity.HasComponent<SkinnedMeshComponent>()) {
				material_handles = entity.GetComponent<SkinnedMeshComponent>().MaterialHandles;
			}

			if (sub_meshes.empty() || material_handles.empty())
				continue;

			// MATERIAL AND MATERIAL UNIFORM BLOCK SORTING
			// Materials will be sorted based on their material, and the uniform 
			// block of an individual material on a MeshRendererComponent.
			int material_index = 0;
			for (int i = 0; i < sub_meshes.size(); ++i)
			{
				// Material Handle + Uniform Group in Mesh Renderer Component
				// Nullptr means there is no custom uniform block
				auto& mesh_renderer_material_pair = material_handles[material_index];

				// Retrieve Cached Mesh Asset
				auto material_asset = scene_context->CachedMaterials[mesh_renderer_material_pair.first].lock();

				// Check if Loaded
				if (!material_asset)
				{
					// If Not Loaded, Call GetAsset to Load
					scene_context->CachedMaterials[mesh_renderer_material_pair.first] = AssetManager::GetAsset<Material>(mesh_renderer_material_pair.first);
					material_asset = scene_context->CachedMaterials[mesh_renderer_material_pair.first].lock();

					// If Failed to Load - Continue
					if (!material_asset)
						continue;
				}

				// Opaque Sorting
				if (material_asset->GetRenderType() == RenderType::L_MATERIAL_OPAQUE)
				{

					// Retrieve the Uniform Block Associated w/ This Mesh Renderer Material
					const auto& uniform_block = (mesh_renderer_material_pair.second) ? mesh_renderer_material_pair.second : material_asset->GetUniformBlock();

					// Key the Material Wrapper to Secure Placement in Opaque Queue
					auto& sub_mesh_map = Opaque_RenderQueue[{ material_asset, uniform_block}]; // Keys this to the opaque renderables

					// Key the Sub Mesh to Retrieve Vector of Entities in this Rendering State
					auto& entity_vector = sub_mesh_map[sub_meshes[i]];

					// If First - Set Allocation for 8 entities
					if (entity_vector.size() == 0 && entity_vector.capacity() == 0)
						entity_vector.reserve(8);

					// If we need to reallocate, double if capacity 
					// is under 64, if above, we will + 8 only
					if (entity_vector.size() >= entity_vector.capacity())
						entity_vector.reserve(entity_vector.size() < 64 ? entity_vector.capacity() * 2 : entity_vector.capacity() + 8);

					entity_vector.emplace_back(entity);

				}
				// Transparent Sorting
				else if (material_asset->GetRenderType() == RenderType::L_MATERIAL_TRANSPARENT || material_asset->GetRenderType() == RenderType::L_MATERIAL_TRANSPARENT_WRITE_DEPTH)
				{
					const glm::vec3& objectPosition = entity.GetTransform().GetGlobalPosition();
					float distance = glm::length(objectPosition - Position);

					// If First - Set Allocation for 128 entities
					// This queue is not batched w/ multiple render
					// states or materials, these all need to be 
					// rendered back to front
					if (Transparent_RenderQueue.size() == 0 && Transparent_RenderQueue.capacity() == 0)
						Transparent_RenderQueue.reserve(128);

					// If we need to reallocate, double if capacity is under 1024, if above, we will + 16 only
					if (Transparent_RenderQueue.size() >= Transparent_RenderQueue.capacity())
						Transparent_RenderQueue.reserve(Transparent_RenderQueue.size() < 1024 ? Transparent_RenderQueue.capacity() * 2 : Transparent_RenderQueue.capacity() + 16);

					// Emplace to Back of TransparentRenderQueue
					Transparent_RenderQueue.emplace_back
					(
						distance,
						_MaterialWrapper{ material_asset, mesh_renderer_material_pair.second ? mesh_renderer_material_pair.second : material_asset->GetUniformBlock() },
						sub_meshes[i],
						entity
					);
				}

				// Makes sure we don't exceed the maximum materials, if we do, then we will 
				// just continue using the last material in the Mesh Renderer materials vector
				if (material_index < material_handles.size() - 1)
					material_index++;


				// Set Option for Debug Draw Cube for AABB
				if (entity.HasComponent<MeshFilterComponent>())
				{
					auto& component = entity.GetComponent<MeshFilterComponent>();
					if (component.GetShouldDisplayDebugLines())
						Debug_RenderAABBs.emplace_back(component.TransformedAABB.GetGlobalBoundsMat4());
				}

				if (entity.HasComponent<SkinnedMeshComponent>())
				{
					auto& component = entity.GetComponent<SkinnedMeshComponent>();
					if (component.DisplayDebugAABB)
						Debug_RenderAABBs.emplace_back(component.TransformedAABB.GetGlobalBoundsMat4());
				}
			}

			// Back-to-Front Sorting - Transparent Objects
			std::sort(Transparent_RenderQueue.begin(), Transparent_RenderQueue.end(), [](const auto& a, const auto& b)
				{
					// Check if either material should be rendered first (before sorting by distance)
					bool aRenderFirst = std::get<1>(a).material->IsTransparencyWriteDepth();
					bool bRenderFirst = std::get<1>(b).material->IsTransparencyWriteDepth();

					if (aRenderFirst && !bRenderFirst) {
						// If 'a' should be rendered first, put it in front
						return true;
					}
					else if (!aRenderFirst && bRenderFirst) {
						// If 'b' should be rendered first, put it in front
						return false;
					}

					// If neither or both materials should be rendered first, perform the standard back-to-front sorting by distance
					return std::get<0>(a) > std::get<0>(b); // Sort by distance, back-to-front
				});
		}
	}

	#pragma endregion

	//
	//	--- SCENE CONTEXT ---
	//

	#pragma region Scene Context

	ForwardPlusPipeline::SceneContext::SceneContext(Scene* scene)
	{
		
		glGenBuffers(1, &PointLight_Data_Buffer);			// Light
		glGenBuffers(1, &SpotLight_Data_Buffer);			// Light
		glGenBuffers(1, &DirectionalLight_Data_Buffer);		// Light
		glGenBuffers(1, &SpotLight_Shadow_Matrix_Buffer);	// Shadow
		glGenBuffers(1, &BoneTransform_Data_Buffer);		// Bones
		
		// Point Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLight_Data_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);
		
		// Spot Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Data_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);
		
		// Directional Lights
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, DirectionalLight_Data_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(SSBOLightStructs::DL_SSBO_DATA_LAYOUT), nullptr, GL_DYNAMIC_DRAW);
		
		// Light Space Matrix SSBO
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Shadow_Matrix_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHT_SHADOW_MAPS * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
		
		// Bone Transformations
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BoneTransform_Data_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_BONE_TRANSFORMATIONS * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
		
		BoneTransform_Offset.reserve(1024);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		// Point Shadow Maps
		{
			// Cube Map Array
			glGenTextures(1, &PointLight_Shadow_CubeMapArray);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, PointLight_Shadow_CubeMapArray);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT24,
				SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION, MAX_POINT_LIGHT_SHADOW_MAPS * 6);
			
			// Frame Buffer
			glGenFramebuffers(1, &PointLight_Shadow_FrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, PointLight_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, PointLight_Shadow_CubeMapArray, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Spot Shadow Maps
		{
			// SPOT SHADOWS - Texture Array
			glGenTextures(1, &SpotLight_Shadow_TextureArray);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, SpotLight_Shadow_TextureArray);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION, MAX_SPOT_LIGHT_SHADOW_MAPS);
			
			// SPOT SHADOWS - Frame Buffer
			glGenFramebuffers(1, &SpotLight_Shadow_FrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, SpotLight_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, SpotLight_Shadow_TextureArray, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		auto camera_view = scene->GetAllEntitiesWith<CameraComponent>();
		for (const auto& entity_handle : camera_view)
		{
			Entity camera_entity = { entity_handle, scene };
			CameraContexts[camera_entity.GetUUID()] = std::make_unique<CameraContext>(scene, &camera_entity, *camera_entity.GetComponent<CameraComponent>().CameraFramebuffer);
		}

	}

	ForwardPlusPipeline::SceneContext::~SceneContext()
	{
		if (OctreeUpdate_Thread.joinable())
			OctreeUpdate_Thread.join();

		CameraContexts.clear();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		glDeleteBuffers(1, &PointLight_Data_Buffer); // Light
		glDeleteBuffers(1, &SpotLight_Data_Buffer); // Light
		glDeleteBuffers(1, &DirectionalLight_Data_Buffer); // Light
		glDeleteBuffers(1, &SpotLight_Shadow_Matrix_Buffer); // Shadow
		glDeleteBuffers(1, &BoneTransform_Data_Buffer); // Bones

		glDeleteFramebuffers(1, &PointLight_Shadow_FrameBuffer);
		glDeleteTextures(1, &PointLight_Shadow_CubeMapArray);
		
		glDeleteFramebuffers(1, &SpotLight_Shadow_FrameBuffer);
		glDeleteTextures(1, &SpotLight_Shadow_TextureArray);
	}

	//
	//	Scene Functions
	//

	void ForwardPlusPipeline::SceneContext::SortRenderQueue(Scene* scene, const std::vector<CameraContext*>& active_cameras)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::SortRenderQueue - Overall");

		if (!scene)
			return;

		Entities_OverallVisible.clear();
		PointLight_OverallVisible.clear();
		SpotLight_OverallVisible.clear();

		for (const auto& camera_context : active_cameras)
		{
			if (!camera_context)
				continue;

			camera_context->Light_FrustumBasedCull(scene, this);
			camera_context->Renderable_FrustumCull(scene, this);

			camera_context->PrepareRenderPass_Depth(scene, this);

			camera_context->Renderable_OcclusionCull(scene, this);

			camera_context->PrepareRenderPass_Colour(scene, this);

			for (const auto& entity : camera_context->Entities_Visible)
			{
				Entities_OverallVisible.insert(entity.GetUUID());
			}
		}

	}

	void ForwardPlusPipeline::SceneContext::UpdateComputeData(Scene* scene)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::UpdateComputeData - Updating Compute Data");

		if (!scene)
			return;

		for (auto& [camera_uuid, camera_context] : s_SceneContext[scene]->CameraContexts)
		{
			if (!camera_context)
				continue;

			// Calculate Workgroups and Generate SSBOs from Screen Size
			camera_context->TileWorkGroups.x = static_cast<unsigned int>(std::ceil(static_cast<float>(camera_context->FrameBufferSize.x / 16.0f)));
			camera_context->TileWorkGroups.y = static_cast<unsigned int>(std::ceil(static_cast<float>(camera_context->FrameBufferSize.y / 16.0f)));

			size_t num_of_tiles = static_cast<size_t>(camera_context->TileWorkGroups.x * camera_context->TileWorkGroups.y);

			// Update Light Indice Buffers

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, camera_context->PointLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_POINT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, camera_context->SpotLight_Indices_Buffer);
			glBufferData(GL_SHADER_STORAGE_BUFFER, num_of_tiles * sizeof(uint32_t) * MAX_SPOT_LIGHTS, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void ForwardPlusPipeline::SceneContext::UpdateGlobalLightSSBOs(Scene* scene)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::UpdateGlobalLightSSBOs - Updating Global Light Data");

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, PointLight_Data_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SpotLight_Data_Buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, DirectionalLight_Data_Buffer);

		// Point Lights
		{
			// Update Light Objects
			static std::vector<SSBOLightStructs::PL_SSBO_DATA_LAYOUT> s_PointLightVector(MAX_POINT_LIGHTS);
			s_PointLightVector.clear();

			// Add lights to vector that are contained within the scene up to a maximum of 1024
			for (auto& entity_uuid : PointLight_OverallVisible)
			{
				Entity entity = scene->FindEntityByUUID(entity_uuid);
				if (!entity)
					continue;

				if (s_PointLightVector.size() >= MAX_POINT_LIGHTS)
					break;

				auto& point_light = entity.GetComponent<PointLightComponent>();
				if (point_light.Active)
				{
					s_PointLightVector.push_back({ point_light, entity.GetTransform() });
					s_PointLightVector.back().radius *= 2.0f;

					if (PointLight_Shadow_LightIndices.find(entity.GetUUID()) != PointLight_Shadow_LightIndices.end())
					{
						s_PointLightVector.back().shadowLayerIndex = PointLight_Shadow_LightIndices.at(entity.GetUUID());
					}
				}
			}

			// Create Buffer Light at End of Vector if not full
			if (s_PointLightVector.size() < MAX_POINT_LIGHTS)
			{
				PointLightComponent temp_last_light;
				s_PointLightVector.push_back(temp_last_light);
				s_PointLightVector.back().lastLight = true;
			}

			// Update SSBO data with light data
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointLight_Data_Buffer);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_PointLightVector.size() * sizeof(SSBOLightStructs::PL_SSBO_DATA_LAYOUT), s_PointLightVector.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		// Spot Lights
		{
			// Update Light Objects
			static std::vector<SSBOLightStructs::SL_SSBO_DATA_LAYOUT> s_SpotLightVector(MAX_SPOT_LIGHTS);
			s_SpotLightVector.clear();

			// Add lights to vector that are contained within the scene up to a maximum of 1024
			for (auto& entity_uuid : SpotLight_OverallVisible)
			{
				Entity entity = scene->FindEntityByUUID(entity_uuid);
				if (!entity)
					continue;

				if (s_SpotLightVector.size() >= MAX_POINT_LIGHTS)
					break;

				auto& spot_light = entity.GetComponent<SpotLightComponent>();
				if (spot_light.Active)
				{
					s_SpotLightVector.push_back({ spot_light, entity.GetTransform() });

					if (SpotLight_Shadow_LightIndices.find(entity.GetUUID()) != SpotLight_Shadow_LightIndices.end())
					{
						s_SpotLightVector.back().shadowLightIndex = SpotLight_Shadow_LightIndices.at(entity.GetUUID());
					}
				}
			}

			// Create Buffer Light at End of Vector if not full
			if (s_SpotLightVector.size() < MAX_SPOT_LIGHTS) {
				SpotLightComponent temp_last_light;
				s_SpotLightVector.push_back(temp_last_light);
				s_SpotLightVector.back().lastLight = true;
			}

			// Update SSBO data with light data
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Data_Buffer);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_SpotLightVector.size() * sizeof(SSBOLightStructs::SL_SSBO_DATA_LAYOUT), s_SpotLightVector.data());
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	void ForwardPlusPipeline::SceneContext::UpdateGlobalBoneSSBO(Scene* scene)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::UpdateGlobalBoneSSBO - Updating Global Bone Data");

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, BoneTransform_Data_Buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BoneTransform_Data_Buffer);

		static std::vector<glm::mat4> s_FinalBoneTransformations = {};
		if (s_FinalBoneTransformations.capacity() == 0)
			s_FinalBoneTransformations.reserve(MAX_BONE_TRANSFORMATIONS);

		s_FinalBoneTransformations.clear();
		BoneTransform_Offset.clear();

		size_t offset = 0;

		for (const auto& entity_uuid : Entities_OverallVisible)
		{
			Entity entity = scene->FindEntityByUUID(entity_uuid);
			if (!entity || !entity.HasComponent<SkinnedMeshComponent>())
				continue;

			auto& skinned_mesh_component = entity.GetComponent<SkinnedMeshComponent>();

			if (offset + skinned_mesh_component.FinalBoneTransformations.size() >= MAX_BONE_TRANSFORMATIONS)
				break;

			s_FinalBoneTransformations.insert(s_FinalBoneTransformations.begin() + offset, skinned_mesh_component.FinalBoneTransformations.begin(), skinned_mesh_component.FinalBoneTransformations.end());
			BoneTransform_Offset[entity.GetUUID()] = static_cast<uint32_t>(offset);

			offset += skinned_mesh_component.FinalBoneTransformations.size();
		}

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, s_FinalBoneTransformations.size() * sizeof(glm::mat4), s_FinalBoneTransformations.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ForwardPlusPipeline::SceneContext::UpdateGlobalShadowMaps(Scene* scene, const std::vector<CameraContext*>& active_cameras)
	{
		L_PROFILE_SCOPE_ACCUMULATIVE("F+Pipeline::SceneContext::UpdateGlobalShadowMaps - Updating Global Shadow Maps");

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, SpotLight_Shadow_Matrix_Buffer);

#pragma region Point Lights

		PointLight_Shadow_LightIndices.clear();

		std::vector<Entity> pl_shadow_casting_vec;
		std::unordered_map<UUID, std::vector<Entity>> pl_shadow_casting_meshes_map; // What meshes are inside this point light?
		constexpr int numShadowCastingLights = MAX_POINT_LIGHT_SHADOW_MAPS; // Number of shadow-casting lights

		// 1. Gather and Sort Point Lights that have shadow mapping enabled
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Point Shadow Mapping 1. Sorting");
			pl_shadow_casting_vec.reserve(PointLight_OverallVisible.size());
			for (auto& entity_uuid : PointLight_OverallVisible)
			{
				Entity entity = scene->FindEntityByUUID(entity_uuid);
				if (!entity)
					continue;

				if (entity.GetComponent<PointLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
					pl_shadow_casting_vec.push_back(entity);
			}

			// Sort array based on distance to highest priority camera if exists
			if (!active_cameras.empty())
			{
				// Get highest priority camera position
				glm::vec3 camera_position = active_cameras.front()->Position;
				std::sort(pl_shadow_casting_vec.begin(), pl_shadow_casting_vec.end(),
					[&camera_position](Entity& a, Entity& b)
					{
						glm::vec3 posA = a.GetTransform().GetGlobalPosition();
						glm::vec3 posB = b.GetTransform().GetGlobalPosition();
						return glm::length(posA - camera_position) < glm::length(posB - camera_position);
					});
			}

			// Keep only the closest 5 point lights
			// TODO: Increase this so there is like a shadow map atlas with lower resolutions? 
			// E.g., One cube map in the array could hold 4 more point light textures if the resolution is halved?
			// Maybe we implement an algorithm to determine which are the most important point lights, 
			//		- Create a cube map array with 25 x 2k textures
			//		- assign a hard limit of maybe 5 x 2K cube maps for the most important point lights, 
			//		- then have another 5 cube maps that are made up of 20 1K point lights, and so on
			if (pl_shadow_casting_vec.size() > numShadowCastingLights)
				pl_shadow_casting_vec.erase(pl_shadow_casting_vec.begin() + numShadowCastingLights, pl_shadow_casting_vec.end());

			for (auto& point_light : pl_shadow_casting_vec) {

				Bounds_Sphere sphere{};
				sphere.BoundsCentre = point_light.GetTransform().GetGlobalPosition();
				sphere.BoundsRadius = point_light.GetComponent<PointLightComponent>().Radius * 2.0f;

				if (auto oct_ref = scene->GetOctree().lock(); oct_ref) {

					std::unique_lock lock(oct_ref->GetOctreeMutex());

					const auto& query_vec = oct_ref->Query(sphere);

					std::vector<Entity>& entities_in_light = pl_shadow_casting_meshes_map[point_light.GetUUID()];
					entities_in_light.reserve(query_vec.size());

					for (const auto& data : query_vec)
					{
						if (!data->Data || !data->Data.HasComponent<MeshRendererComponent>())
							continue;

						auto& component = data->Data.GetComponent<MeshRendererComponent>();
						if (component.Active && component.CastShadows)
							entities_in_light.push_back(data->Data);
					}
				}

			}

		}

		// 2. Initialise and Draw Shadow CubeMap Array
		if (!pl_shadow_casting_vec.empty())
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Point Shadow Mapping 2. Drawing");

			glBindFramebuffer(GL_FRAMEBUFFER, PointLight_Shadow_FrameBuffer);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, PointLight_Shadow_CubeMapArray, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			glCullFace(GL_FRONT);

			glViewport(0, 0, SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION);

			glClear(GL_DEPTH_BUFFER_BIT);

			auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Point");
			shader->Bind();


			for (int lightIndex = 0; lightIndex < pl_shadow_casting_vec.size(); ++lightIndex) {

				glm::vec3 light_pos = pl_shadow_casting_vec[lightIndex].GetTransform().GetGlobalPosition();

				float near_plane = 0.1f;
				float far_plane = pl_shadow_casting_vec[lightIndex].GetComponent<PointLightComponent>().Radius * 2.0f;
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
				std::vector<glm::mat4> shadowTransforms{};
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
				shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

				for (unsigned int i = 0; i < 6; ++i)
				{
					shader->SetMat4(std::string("u_ShadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);
				}

				int layerOffset = lightIndex * 6;
				shader->SetInt("u_LayerOffset", layerOffset);
				shader->SetFloatVec3("u_LightPosition", light_pos);
				shader->SetFloat("u_FarPlane", far_plane);

				// Link up the offset to the light map so we can 
				// update the data into the SSBO for shader access
				PointLight_Shadow_LightIndices[pl_shadow_casting_vec[lightIndex].GetUUID()] = lightIndex;

				// Render all entities THAT ARE IN RANGE of this light in one pass.
				for (auto& entity : pl_shadow_casting_meshes_map[pl_shadow_casting_vec[lightIndex].GetUUID()]) {

					if (!entity)
						continue;

					glm::mat4 transform = entity.GetTransform().GetGlobalTransform();
					shader->SetMat4("u_Model", transform);

					std::shared_ptr<StaticMesh> asset_mesh = AssetManager::GetAsset<StaticMesh>(entity.GetComponent<MeshFilterComponent>().StaticMeshHandle);
					for (auto& sub_mesh : asset_mesh->SubMeshes)
					{
						Renderer::DrawSubMesh(sub_mesh);

					}
				}
			}

			shader->UnBind();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glCullFace(GL_BACK);
		}

#pragma endregion

#pragma region Spot Lights

		std::vector<Entity> sl_shadow_casting_vec;
		std::vector<glm::mat4> sl_shadow_light_space_matricies;
		sl_shadow_light_space_matricies.reserve(30);
		std::unordered_map<UUID, std::vector<Entity>> sl_shadow_renderable_entities;

		SpotLight_Shadow_LightIndices.clear();

		// 1. Gather Spot Lights
		{
			L_PROFILE_SCOPE_ACCUMULATIVE("Spot Shadow Mapping 1. Gathering");
			sl_shadow_casting_vec.reserve(SpotLight_OverallVisible.size());
			for (auto& entity_uuid : SpotLight_OverallVisible)
			{
				Entity entity = scene->FindEntityByUUID(entity_uuid);
				if (!entity)
					continue;

				if (entity.GetComponent<SpotLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
				{
					if (sl_shadow_casting_vec.size() >= MAX_SPOT_LIGHT_SHADOW_MAPS)
						break;

					sl_shadow_casting_vec.push_back(entity);
				}
			}
		}

		if (!sl_shadow_casting_vec.empty())
		{
			// 2. Calculate ViewProj and Get Meshes in Frustum
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("Spot Shadow Mapping 2. Calculate ViewProj and Get Meshes in Frustum");
				for (auto& entity : sl_shadow_casting_vec) {

					if (!entity)
						continue;

					auto& transform = entity.GetTransform();

					glm::mat4 light_proj = glm::perspective(glm::radians(entity.GetComponent<SpotLightComponent>().Angle), 1.0f, 0.1f, entity.GetComponent<SpotLightComponent>().Range);
					glm::mat4 light_view = glm::lookAt(transform.GetGlobalPosition(), transform.GetGlobalPosition() + transform.GetForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f));

					sl_shadow_light_space_matricies.push_back(light_proj * light_view);

					Frustum spot_frustum = { sl_shadow_light_space_matricies.back() };

					if (auto oct_ref = scene->GetOctree().lock(); oct_ref) {

						std::unique_lock lock(oct_ref->GetOctreeMutex());

						const auto& query_vec = oct_ref->Query(spot_frustum);

						auto& sl_renderable_entities = sl_shadow_renderable_entities[entity.GetUUID()];
						sl_renderable_entities.reserve(query_vec.size());

						for (const auto& data : query_vec)
						{
							if (!data->Data || !data->Data.HasComponent<MeshRendererComponent>())
								continue;

							auto& component = data->Data.GetComponent<MeshRendererComponent>();
							if (component.Active && component.CastShadows)
								sl_renderable_entities.push_back(data->Data);
						}
					}

				}

			}

			// 3. Update Light Space Matrix SSBO Data
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("Spot Shadow Mapping 3. Updating Light Matrix SSBO Data");
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, SpotLight_Shadow_Matrix_Buffer);

				// Update SSBO data with light data
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, SpotLight_Shadow_Matrix_Buffer);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sl_shadow_light_space_matricies.size() * sizeof(glm::mat4), sl_shadow_light_space_matricies.data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// 4. Render Shadow Map from Light Space Matrix ViewProj
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("Spot Shadow Mapping 4. Rendering Spot Shadow Maps");

				glBindFramebuffer(GL_FRAMEBUFFER, SpotLight_Shadow_FrameBuffer);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, SpotLight_Shadow_TextureArray, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);

				glCullFace(GL_FRONT);

				glViewport(0, 0, SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION);
				glClear(GL_DEPTH_BUFFER_BIT);

				auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Spot");
				shader->Bind();

				for (int light_index = 0; light_index < sl_shadow_casting_vec.size(); ++light_index) {

					Entity entity = sl_shadow_casting_vec[light_index];
					if (!entity)
						continue;

					SpotLight_Shadow_LightIndices[entity.GetUUID()] = light_index;
					shader->SetMat4("u_LightSpaceMatrix", sl_shadow_light_space_matricies[light_index]);
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, SpotLight_Shadow_TextureArray, 0, light_index);

					for (auto& mesh_entity : sl_shadow_renderable_entities[entity.GetUUID()])
					{
						if (!mesh_entity)
							continue;

						glm::mat4 transform = mesh_entity.GetTransform().GetGlobalTransform();
						shader->SetMat4("u_Model", transform);

						std::shared_ptr<StaticMesh> asset_mesh = AssetManager::GetAsset<StaticMesh>(mesh_entity.GetComponent<MeshFilterComponent>().StaticMeshHandle);
						for (auto& sub_mesh : asset_mesh->SubMeshes)
						{
							Renderer::DrawSubMesh(sub_mesh);
						}
					}

				}

				shader->UnBind();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glCullFace(GL_BACK);
			}
		}

#pragma endregion

#pragma region Directional Lights

		for (const auto& camera_context : active_cameras)
		{
			if (!camera_context)
				continue;

			std::vector<Entity> dl_shadow_casting_vec; dl_shadow_casting_vec.reserve(MAX_DIRE_LIGHT_SHADOW_MAPS);
			std::vector<Entity> dl_shadow_renderable_entities; dl_shadow_renderable_entities.reserve(1024);
			std::vector<glm::mat4> dl_shadow_light_space_matricies; dl_shadow_light_space_matricies.reserve(MAX_DIRE_LIGHT_SHADOW_MAPS * 5);

			camera_context->DirectionalLight_Shadow_LightIndices.clear();
			camera_context->DirectionalLight_Shadow_CascadeIndices.clear();

			const glm::mat4& projection_matrix = camera_context->Projection_Matrix;
			float fov = 2.0f * atan(1.0f / projection_matrix[1][1]);
			float aspect = projection_matrix[1][1] / projection_matrix[0][0];

			glm::mat4 view_matrix = camera_context->View_Matrix;

			// 1. Gather Directional Lights
			{
				L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 1. Gathering");
				dl_shadow_casting_vec.reserve(camera_context->DirectionalLights.size());
				for (auto& entity : camera_context->DirectionalLights)
					if (entity.GetComponent<DirectionalLightComponent>().ShadowFlag != ShadowTypeFlag::NoShadows)
					{
						if (dl_shadow_casting_vec.size() >= MAX_DIRE_LIGHT_SHADOW_MAPS)
							break;

						dl_shadow_casting_vec.emplace_back(entity);
					}
			}

			if (!dl_shadow_casting_vec.empty())
			{
				// 2. Calculate Light Space World Space AABB - Get Meshes Intersecting with AABB from Octree

				// TODO: Need to fix this because it is not including objects that are behind camera frustum that 
				// may cast shadow into frustum. Maybe we do this after generating the cascades and use the 
				// world space AABB of the light projection to find our meshes?
				Bounds_Sphere world_light_bounds;
				{
					L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 2a. Calculate Light Space Bounds");

					world_light_bounds.BoundsCentre = camera_context->Position;
					world_light_bounds.BoundsRadius = camera_context->FarClip;
				}

				{
					L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 2b. Get Meshes");

					if (auto oct_ref = scene->GetOctree().lock(); oct_ref) {

						std::unique_lock lock(oct_ref->GetOctreeMutex());

						const auto& query_vec = oct_ref->Query(world_light_bounds);

						dl_shadow_renderable_entities.reserve(query_vec.size());

						for (const auto& data : query_vec)
						{
							if (!data->Data || !data->Data.HasComponent<MeshRendererComponent>())
								continue;

							auto& component = data->Data.GetComponent<MeshRendererComponent>();
							if (component.Active && component.CastShadows)
								dl_shadow_renderable_entities.push_back(data->Data);
						}
					}
				}

				// 3. Calculate Light Space Matricies Per Light Per Cascade - 40 x glm::mat4's is the max = MAX_DIRECTIONAL_LIGHTS * 4 cascades (per directional light)

				{
					L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 3. Calculate Cascade Light Matricies");
					dl_shadow_light_space_matricies.reserve(dl_shadow_casting_vec.size() * 5);

					for (int light_index = 0; light_index < dl_shadow_casting_vec.size(); light_index++)
					{
						Entity entity = dl_shadow_casting_vec[light_index];
						if (!entity)
							continue;

						auto& component = entity.GetComponent<DirectionalLightComponent>();
						std::array<float, 5>& shadow_cascade_distances = camera_context->DirectionalLight_Shadow_CascadeIndices[entity.GetUUID()];
						std::array<glm::mat4, 5> light_space_matrices = Frustum::CalculateCascadeLightSpaceMatrices(fov, aspect, camera_context->NearClip, glm::max(camera_context->FarClip * 1.1f, camera_context->FarClip * component.MaxShadowVisibleDistance), view_matrix, entity.GetTransform().GetForwardDirection(), shadow_cascade_distances);

						dl_shadow_light_space_matricies.insert(dl_shadow_light_space_matricies.end(), light_space_matrices.begin(), light_space_matrices.end());
						camera_context->DirectionalLight_Shadow_LightIndices[entity.GetUUID()] = light_index;
					}
				}

				// 4. Update Light Space Matrix SSBO Data

				{
					L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 4. Updating Light Matrix SSBO Data");
					// Update SSBO data with light data
					glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, camera_context->DirectionalLight_Shadow_Matrix_Buffer);
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, camera_context->DirectionalLight_Shadow_Matrix_Buffer);
					glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dl_shadow_light_space_matricies.size() * sizeof(glm::mat4), dl_shadow_light_space_matricies.data());
					glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
				}

				// 5. Render Shadow Map from Light Space Matrix View & Proj

				{
					L_PROFILE_SCOPE_ACCUMULATIVE("Directional Shadow Mapping 5. Rendering Cascaded Shadow Maps");

					glBindFramebuffer(GL_FRAMEBUFFER, camera_context->DirectionalLight_Shadow_FrameBuffer);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera_context->DirectionalLight_Shadow_Texture_Array, 0);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);

					glCullFace(GL_FRONT);

					glViewport(0, 0, SHADOW_MAP_TEXTURE_RESOLUTION, SHADOW_MAP_TEXTURE_RESOLUTION);
					glClear(GL_DEPTH_BUFFER_BIT);

					auto shader = AssetManager::GetInbuiltShader("FP_Shadow_Directional");
					shader->Bind();

					for (int light_index = 0; light_index < dl_shadow_casting_vec.size(); ++light_index) {

						Entity entity = dl_shadow_casting_vec[light_index];
						if (!entity)
							continue;

						shader->SetUInt("u_LightIndex", light_index);

						for (auto& mesh_entity : dl_shadow_renderable_entities) {

							if (!mesh_entity)
								continue;

							glm::mat4 transform = mesh_entity.GetTransform().GetGlobalTransform();
							shader->SetMat4("u_Model", transform);

							// Retrieve Cached Mesh Asset
							auto mesh_asset = CachedMeshes[mesh_entity.GetComponent<MeshFilterComponent>().StaticMeshHandle].lock();

							// Check if Loaded
							if (!mesh_asset)
							{
								// If Not Loaded, Call GetAsset to Load
								CachedMeshes[mesh_entity.GetComponent<MeshFilterComponent>().StaticMeshHandle] = AssetManager::GetAsset<StaticMesh>(mesh_entity.GetComponent<MeshFilterComponent>().StaticMeshHandle);
								mesh_asset = CachedMeshes[mesh_entity.GetComponent<MeshFilterComponent>().StaticMeshHandle].lock();

								// If Failed to Load - Continue
								if (!mesh_asset)
									continue;
							}

							if (mesh_asset)
							{
								for (auto& sub_mesh : mesh_asset->SubMeshes)
								{
									Renderer::DrawSubMesh(sub_mesh);
								}
							}
						}
					}

					shader->UnBind();

					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glCullFace(GL_BACK);
				}
			}
		}

#pragma endregion

	}

	//
	//	Thread Functions
	//

	void ForwardPlusPipeline::SceneContext::OnOctreeUpdate(Scene* scene)
	{
		L_PROFILE_SCOPE("Forward Plus (Octree Thread) - Octree Worker Thread Update");

		if (!scene)
			return;

		auto octree_ref = scene->GetOctree().lock();
		if (!octree_ref)
			return;

		// Lock the Octree Mutex
		std::unique_lock lock(octree_ref->GetOctreeMutex());

		// 1. Check which objects are no longer in the scene
		const auto& octree_data_sources = octree_ref->GetAllOctreeDataSources();
		std::vector<Entity> remove_entities{};
		for (const auto& data_source : octree_data_sources)
		{
			// Check if Scene has Entity
			if (scene->HasEntity(data_source->Data))
			{
				Entity entity = data_source->Data;

				if (!entity)
					continue;

				// Check if Entity still has a MeshFilterComponent && MeshRendererComponent
				if (entity.HasComponent<MeshFilterComponent>() && entity.HasComponent<MeshRendererComponent>())
				{
					// Check if MeshRendererComponent Is Active
					if (entity.GetComponent<MeshRendererComponent>().Active)
					{
						continue;
					}
				}

				// Check if Entiy has a SkinnedMeshComponent and Is Active
				if (entity.HasComponent<SkinnedMeshComponent>() && entity.GetComponent<SkinnedMeshComponent>().Active)
				{
					continue;
				}

			}

			// All Other Cases Fall Into Here
			remove_entities.push_back(data_source->Data);
		}

		// 2. Remove all references from Octree that no longer exist in the scene
		for (const auto& entity : remove_entities)
			octree_ref->Remove(entity);

		// 3. Try to Shrink the Octree
		octree_ref->TryShrinkOctree();

		// 4. Update All AABB's that have changed since last processing - for example, if the transform was altered, it will flag to be updated
		auto static_mesh_view = scene->GetAllEntitiesWith<MeshFilterComponent, MeshRendererComponent>();
		for (const auto& entity_handle : static_mesh_view)
		{
			auto& mesh_filter_component = static_mesh_view.get<MeshFilterComponent>(entity_handle);

			if (mesh_filter_component.StaticMeshHandle == NULL_UUID || !AssetManager::IsAssetLoaded(mesh_filter_component.StaticMeshHandle))
			{
				continue;
			}

			// Check if the StaticMeshAsset bounds are valid
			if (const auto& mesh_asset = AssetManager::GetAsset<StaticMesh>(mesh_filter_component.StaticMeshHandle); mesh_asset)
			{
				if (mesh_asset->MeshBounds.BoundsMin.x ==  FLT_MAX || 
					mesh_asset->MeshBounds.BoundsMin.y ==  FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMin.z ==  FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.x == -FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.y == -FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.z == -FLT_MAX)
				{
					continue;
				}
			}

			// Check if the AABB of this MeshFilter needs to be updated
			bool update_AABB = mesh_filter_component.AABBNeedsUpdate;

			// Check if the underlying mesh has been updated, therefore requiring an update to this MeshFilter's AABB - for example, a mesh is altered by C# code, we will then need to update the AABB for this MeshFilter
			if (!update_AABB)
			{
				if (AssetManager::IsAssetLoaded(mesh_filter_component.StaticMeshHandle))
				{
					if (auto asset_mesh = AssetManager::GetAsset<StaticMesh>(mesh_filter_component.StaticMeshHandle); asset_mesh)
					{
						if (asset_mesh->ModifiedAABB)
						{
							// Set Flags for Updates
							mesh_filter_component.AABBNeedsUpdate = true;
							mesh_filter_component.OctreeNeedsUpdate = true;

							// Ensure processed in this frame
							update_AABB = mesh_filter_component.AABBNeedsUpdate;

							// Reset flag on AssetMesh
							asset_mesh->ModifiedAABB = false;
						}
					}
				}
			}

			// Update the MeshFilter AABB if required
			if (update_AABB && AssetManager::IsAssetLoaded(mesh_filter_component.StaticMeshHandle))
			{
				mesh_filter_component.UpdateTransformedAABB();
			}

			// If MeshFilterComponent requires the Octree to be Updated, we will do this here
			if (mesh_filter_component.OctreeNeedsUpdate)
			{
				// Try to update the data source in the Octree
				if (octree_ref->Update({ entity_handle, scene }, mesh_filter_component.TransformedAABB))
				{
					mesh_filter_component.OctreeNeedsUpdate = false;
				}
			}
		}

		auto skinned_mesh_view = scene->GetAllEntitiesWith<SkinnedMeshComponent>();
		for (const auto& entity_handle : skinned_mesh_view)
		{
			auto& skinned_mesh_component = skinned_mesh_view.get<SkinnedMeshComponent>(entity_handle);

			if (skinned_mesh_component.StaticMeshHandle == NULL_UUID || !AssetManager::IsAssetLoaded(skinned_mesh_component.StaticMeshHandle))
			{
				continue;
			}
			 
			// Check if the StaticMeshAsset bounds are valid
			if (const auto& mesh_asset = AssetManager::GetAsset<StaticMesh>(skinned_mesh_component.StaticMeshHandle); mesh_asset)
			{
				if (mesh_asset->MeshBounds.BoundsMin.x == FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMin.y == FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMin.z == FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.x == -FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.y == -FLT_MAX ||
					mesh_asset->MeshBounds.BoundsMax.z == -FLT_MAX)
				{
					continue;
				}
			}
			// Check if the AABB of this MeshFilter needs to be updated
			bool update_AABB = skinned_mesh_component.AABBNeedsUpdate;

			// Check if the underlying mesh has been updated, therefore requiring an update to this MeshFilter's AABB - for example, a mesh is altered by C# code, we will then need to update the AABB for this MeshFilter
			if (!update_AABB)
			{
				if (AssetManager::IsAssetLoaded(skinned_mesh_component.StaticMeshHandle))
				{
					if (auto asset_mesh = AssetManager::GetAsset<StaticMesh>(skinned_mesh_component.StaticMeshHandle); asset_mesh)
					{
						if (asset_mesh->ModifiedAABB)
						{
							// Set Flags for Updates
							skinned_mesh_component.AABBNeedsUpdate = true;
							skinned_mesh_component.OctreeNeedsUpdate = true;

							// Ensure processed in this frame
							update_AABB = skinned_mesh_component.AABBNeedsUpdate;

							// Reset flag on AssetMesh
							asset_mesh->ModifiedAABB = false;
						}
					}
				}
			}

			// Update the MeshFilter AABB if required
			if (update_AABB && AssetManager::IsAssetLoaded(skinned_mesh_component.StaticMeshHandle))
			{
				skinned_mesh_component.UpdateTransformedAABB();
			}

			// If MeshFilterComponent requires the Octree to be Updated, we will do this here
			if (skinned_mesh_component.OctreeNeedsUpdate)
			{
				// Try to update the data source in the Octree
				if (octree_ref->Update({ entity_handle, scene }, skinned_mesh_component.TransformedAABB))
				{
					skinned_mesh_component.OctreeNeedsUpdate = false;
				}
			}

		}
	}

	#pragma endregion

#pragma endregion

#pragma region ForwardPipeline

	void ForwardPipeline::OnRenderScene(Scene* scene)
	{
	}

	void ForwardPipeline::OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera)
	{
	}

	void ForwardPipeline::OnStartPipeline(Scene* scene) {

	}

	void ForwardPipeline::OnStopPipeline(Scene* scene) {

		Renderer::CleanupRenderData();
	}

	void ForwardPipeline::OnViewportResize(Scene* scene, const glm::uvec2& viewport_size)
	{

	}

	std::vector<Entity> ForwardPipeline::GetActiveCameras(Scene* scene)
	{
		return std::vector<Entity>();
	}

#pragma endregion

#pragma region DeferredPipeline

	void DeferredRenderPipeline::OnRenderScene(Scene* scene) {

	}

	void DeferredRenderPipeline::OnRenderEditorScene(Scene* scene, const std::vector<Entity>& scene_cameras, EditorCamera* editor_camera)
	{
	}

	void DeferredRenderPipeline::OnStartPipeline(Scene* scene) {

	}

	void DeferredRenderPipeline::OnStopPipeline(Scene* scene) {

		Renderer::CleanupRenderData();
	}

	void DeferredRenderPipeline::OnViewportResize(Scene* scene, const glm::uvec2& viewport_size)
	{
	}

	std::vector<Entity> DeferredRenderPipeline::GetActiveCameras(Scene* scene)
	{
		return std::vector<Entity>();
	}

#pragma endregion

}
