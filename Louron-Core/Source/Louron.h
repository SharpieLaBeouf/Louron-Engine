#pragma once

#include "Animation/Animations.h"

#include "Asset/Asset.h"
#include "Asset/Asset Importer.h"
#include "Asset/Asset Manager.h"
#include "Asset/Asset Manager API.h"

#include "Core/Audio.h"
#include "Core/Engine.h"
#include "Core/GuiLayer.h"
#include "Core/Input.h"
#include "Core/Layer.h"
#include "Core/Logging.h"
#include "Core/Time.h"
#include "Core/Window.h"
#include "Core/FileSystem Utilities.h"
#include "Core/File Utilities.h"
#include "Core/Platform.h"

#include "Jobs/Jobs.h"

#include "OpenGL/Buffer.h"
#include "OpenGL/Compute Shader Asset.h"
#include "OpenGL/Material.h"
#include "OpenGL/Mesh.h"
#include "OpenGL/Shader.h"
#include "OpenGL/Texture.h"
#include "OpenGL/Vertex Array.h"
#include "OpenGL/Framebuffer.h"

#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Scene/Prefab.h"
#include "Scene/Scene Serializer.h"
#include "Scene/Spatial Partitioning/Bounds.h"
#include "Scene/Spatial Partitioning/Frustum.h"
#include "Scene/Spatial Partitioning/OctreeBounds.h"

#include "Scene/Components/Audio Components.h"
#include "Scene/Components/Camera Component.h"
#include "Scene/Components/Core Components.h"
#include "Scene/Components/Light Components.h"
#include "Scene/Components/Mesh Components.h"
#include "Scene/Components/Script Component.h"
#include "Scene/Components/Skybox Component.h"
#include "Scene/Components/SkinnedMeshComponent.h"
#include "Scene/Components/Animator Component.h"
#include "Scene/Components/Physics/Collider Components.h"
#include "Scene/Components/Physics/Rigidbody Component.h"

#include "Physics/CollisionCallback.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsWrappers.h"

#include "Project/Project.h"
#include "Project/Project Serializer.h"

#include "Renderer/Camera.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererPipeline.h"

#include "Debug/Assert.h"
#include "Debug/Profiler.h"

#include "Scripting/Script Manager.h"
#include "Scripting/Script Register.h"
#include "Scripting/Script Defines.h"