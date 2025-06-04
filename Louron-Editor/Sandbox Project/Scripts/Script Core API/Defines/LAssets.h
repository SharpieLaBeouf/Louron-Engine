#pragma once

#include <vector>

#include "../Defines/LTypes.h"
#include "../Engine Callbacks.h"

namespace Louron
{

	namespace Components 
	{
		class MeshFilterComponent;
		class MeshRendererComponent;
		class SkinnedMeshComponent;
		class SkyboxComponent;
	}

	namespace Assets
	{

		class Asset
		{
		public:

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
				AnimationStateMachine,

				Audio,

				Material_Standard,
				Material_Skybox,

				Compute_Shader,
				Shader,
		
				Humanoid,
				HumanoidMask,

			};

			Asset() = default;
			virtual ~Asset() = default;

			virtual AssetType GetAssetType() const { return AssetType::Unknown; }

			virtual operator uint32_t() const { return m_AssetHandle; }

		protected:

			void SetAssetHandle(uint32_t asset_handle) { m_AssetHandle = asset_handle; }

			uint32_t m_AssetHandle = NULL_UUID;
		};

		class Prefab : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::Prefab; }

		};

		class Texture2D : public Asset
		{
			friend class MaterialUniformBlock;

		private:

			Texture2D(bool create_new) {};

		public:

			enum TextureFormat : uint8_t
			{
				RED_8 = 0,
				RED_GREEN_BLUE_8,
				RED_GREEN_BLUE_ALPHA_8,

				BLUE_GREEN_RED_ALPHA_8 // Embedded Assimp Textures
			};

			enum TextureProperty : uint8_t
			{
				Wrap_Repeat = 1U << 0,
				Wrap_Mirror = 1U << 1,
				Wrap_ClampEdge = 1U << 2,
				Wrap_ClampBorder = 1U << 3,

				Filter_Linear = 1U << 4,
				Filter_Nearest = 1U << 5,

				Anisotropy_Enabled = 1U << 6,

				Default = Wrap_Repeat | Filter_Linear | Anisotropy_Enabled,
			};

			Texture2D() = default;
			Texture2D(const Vectors::UVector2& size, TextureFormat internal_format) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t, uint32_t, TextureFormat), Texture2D_Create, size.x, size.y, internal_format); }
			Texture2D(unsigned char* data, const Vectors::UVector2& size, TextureFormat internal_format, TextureFormat data_format) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const unsigned char*, uint32_t, uint32_t, TextureFormat, TextureFormat), Texture2D_CreateWithData, data, size.x, size.y, internal_format, data_format); }
			Texture2D(const std::vector<Vectors::UVector4>& data, const Vectors::UVector2& size) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const unsigned char*, uint32_t, uint32_t, TextureFormat, TextureFormat), Texture2D_CreateWithData, reinterpret_cast<const unsigned char*>(data.data()), size.x, size.y, TextureFormat::RED_GREEN_BLUE_ALPHA_8, TextureFormat::RED_GREEN_BLUE_ALPHA_8); }

			void SetPixel(const Vectors::Vector4& colour, const Vectors::IVector2& pixel_coord) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*, const Vectors::IVector2*), Texture2D_SetPixel, m_AssetHandle, &colour, &pixel_coord); }
			void SetPixelData(unsigned char* pixel_data, size_t pixel_data_size, TextureFormat pixel_data_format) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const unsigned char*, size_t, TextureFormat), Texture2D_SetPixelData, m_AssetHandle, pixel_data, pixel_data_size, pixel_data_format); }
			void SetPixelData(const std::vector<Vectors::UVector4>& pixel_data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const unsigned char*, size_t, TextureFormat), Texture2D_SetPixelData, m_AssetHandle, reinterpret_cast<const unsigned char*>(pixel_data.data()), pixel_data.size() * 4, TextureFormat::RED_GREEN_BLUE_ALPHA_8); }

			void SubmitTextureChanges() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Texture2D_SubmitTextureChanges, m_AssetHandle); }

			void Destroy() { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Texture2D_Destroy, m_AssetHandle); m_AssetHandle = NULL_UUID; }

			AssetType GetAssetType() const override { return AssetType::Texture2D; }

		};

		class TextureCubeMap : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::TextureCubeMap; }

		};

		class StaticMesh : public Asset
		{

		private:

			friend class Components::MeshFilterComponent;
			friend class Components::SkinnedMeshComponent;

			StaticMesh(bool create_new) {};

		public:

			StaticMesh() { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(), Mesh_CreateNewMesh); }

			void Destroy() { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Mesh_Destroy, m_AssetHandle); m_AssetHandle = NULL_UUID; }

			void SubmitChanges(bool clear_cpu_data) { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, bool), Mesh_, m_AssetHandle, clear_cpu_data); }
			void CopyBufferDataToCPU() { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Mesh_CopyBufferDataToCPU, m_AssetHandle); }
			void ClearBufferDataFromCPU() { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Mesh_ClearBufferDataFromCPU, m_AssetHandle); }

			void SetVertices(const float* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetVertices, m_AssetHandle, data, data_size); }
			void SetVertices(const std::vector<float>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetVertices, m_AssetHandle, data.data(), data.size()); }
			void SetVertices(const std::vector<Vectors::Vector3>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetVertices, m_AssetHandle, reinterpret_cast<const float*>(data.data()), data.size() * 3); }

			void SetNormals(const float* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetNormals, m_AssetHandle, data, data_size); }
			void SetNormals(const std::vector<float>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetNormals, m_AssetHandle, data.data(), data.size()); }
			void SetNormals(const std::vector<Vectors::Vector3>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetNormals, m_AssetHandle, reinterpret_cast<const float*>(data.data()), data.size() * 3); }

			void SetTextureCoords(const float* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTextureCoords, m_AssetHandle, data, data_size); }
			void SetTextureCoords(const std::vector<float>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTextureCoords, m_AssetHandle, data.data(), data.size()); }
			void SetTextureCoords(const std::vector<Vectors::Vector2>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTextureCoords, m_AssetHandle, reinterpret_cast<const float*>(data.data()), data.size() * 2); }

			void SetTangents(const float* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTangents, m_AssetHandle, data, data_size); }
			void SetTangents(const std::vector<float>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTangents, m_AssetHandle, data.data(), data.size()); }
			void SetTangents(const std::vector<Vectors::Vector3>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetTangents, m_AssetHandle, reinterpret_cast<const float*>(data.data()), data.size() * 3); }

			void SetBitangents(const float* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetBitangents, m_AssetHandle, data, data_size); }
			void SetBitangents(const std::vector<float>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetBitangents, m_AssetHandle, data.data(), data.size()); }
			void SetBitangents(const std::vector<Vectors::Vector3>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float*, size_t), Mesh_SetBitangents, m_AssetHandle, reinterpret_cast<const float*>(data.data()), data.size() * 3); }

			void SetBoneIDs(const Vectors::IVector4* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::IVector4*, size_t), Mesh_SetBoneIDs, m_AssetHandle, data, data_size); }
			void SetBoneIDs(const std::vector<Vectors::IVector4>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::IVector4*, size_t), Mesh_SetBoneIDs, m_AssetHandle, data.data(), data.size()); }

			void SetBoneWeights(const Vectors::Vector4* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*, size_t), Mesh_SetBoneWeights, m_AssetHandle, data, data_size); }
			void SetBoneWeights(const std::vector<Vectors::Vector4>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4*, size_t), Mesh_SetBoneWeights, m_AssetHandle, data.data(), data.size()); }

			void SetVertexes(const std::vector<Vertex>& vertexes) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vertex*, size_t), Mesh_SetVertexes, m_AssetHandle, vertexes.data(), vertexes.size()); }

			const float* GetVertices(size_t* data_size) const { const float* data = nullptr;		ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float**, size_t*), Mesh_GetVertices, m_AssetHandle, &data, data_size);		return data; }
			const float* GetNormals(size_t* data_size) const { const float* data = nullptr;		ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float**, size_t*), Mesh_GetNormals, m_AssetHandle, &data, data_size);			return data; }
			const float* GetTextureCoords(size_t* data_size) const { const float* data = nullptr;		ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float**, size_t*), Mesh_GetTextureCoords, m_AssetHandle, &data, data_size);	return data; }
			const float* GetTangents(size_t* data_size) const { const float* data = nullptr;		ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float**, size_t*), Mesh_GetTangents, m_AssetHandle, &data, data_size);		return data; }
			const float* GetBitangents(size_t* data_size) const { const float* data = nullptr;		ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const float**, size_t*), Mesh_GetBitangents, m_AssetHandle, &data, data_size);		return data; }
			const Vectors::IVector4* GetBoneIDs(size_t* data_size) const { const Vectors::IVector4* data = nullptr;	ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::IVector4**, size_t*), Mesh_GetBoneIDs, m_AssetHandle, &data, data_size);		return data; }
			const Vectors::Vector4* GetBoneWeights(size_t* data_size) const { const Vectors::Vector4* data = nullptr;	ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const Vectors::Vector4**, size_t*), Mesh_GetBoneWeights, m_AssetHandle, &data, data_size);	return data; }

			void SetTriangles(const uint32_t* data, size_t data_size) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t*, size_t), Mesh_SetTriangles, m_AssetHandle, data, data_size); }
			void SetTriangles(const std::vector<uint32_t>& data) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t*, size_t), Mesh_SetTriangles, m_AssetHandle, data.data(), data.size()); }
			const uint32_t* GetTriangles(size_t* data_size) const { const uint32_t* data = nullptr;	ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const uint32_t**, size_t*), Mesh_GetTriangles, m_AssetHandle, &data, data_size);	return data; }

			void RecalculateNormals() const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Mesh_RecalculateNormals, m_AssetHandle); }

			AssetType GetAssetType() const override { return AssetType::Mesh; }
		};

		class Skeleton : public Asset
		{
			friend class Components::SkinnedMeshComponent;

		public:

			AssetType GetAssetType() const override { return AssetType::Skeleton; }

		};

		class AnimationClip : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::AnimationClip; }

		};

		class AnimatorMachine : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::AnimationStateMachine; }

		};

		class Humanoid : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::Humanoid; }

		};

		class HumanoidMask : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::HumanoidMask; }

		};

		class Audio : public Asset
		{

		public:

			AssetType GetAssetType() const override { return AssetType::Audio; }

		};

		class SkyboxMaterial : public Asset
		{
			friend class Components::SkyboxComponent;

		public:

			AssetType GetAssetType() const override { return AssetType::Material_Skybox; }

		};

		class ComputeBuffer
		{

		public:

			ComputeBuffer() = default;
			ComputeBuffer(size_t element_count, size_t element_size) { Create(element_count, element_size); };
			~ComputeBuffer() { if (m_Handle) Release(); }


			void Create(size_t element_count, size_t element_size) { if (m_Handle) Release(); m_Handle = ENGINE_SAFE_CALL_RET_PTR(void*, void* (*)(size_t, size_t), ComputeBuffer_Create, element_count, element_size); }
			void Release() { if (m_Handle) ENGINE_SAFE_CALL_VOID(void* (*)(void*), ComputeBuffer_Release, m_Handle); m_Handle = nullptr; }

			void SetData(const void* data, size_t element_count, size_t size_of_elements) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const void*, size_t, size_t), ComputeBuffer_SetData, m_Handle, data, element_count, size_of_elements); }
			void GetData(void* output, size_t element_count, size_t size_of_elements) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, void*, size_t, size_t), ComputeBuffer_GetData, m_Handle, output, element_count, size_of_elements); }

		private:

			void* m_Handle = nullptr;

			friend class ComputeShader;
		};

		class ComputeShader : public Asset
		{

		public:

			ComputeShader() = default;
			ComputeShader(const std::string& compute_shader_name) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), ComputeShader_GetShaderByName, compute_shader_name.c_str()); }
			void SetComputeShader(const std::string& compute_shader_name) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), ComputeShader_GetShaderByName, compute_shader_name.c_str()); }

			void SetBuffer(const ComputeBuffer& buffer, uint32_t gl_binding_index) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, void*, uint32_t), ComputeShader_SetBuffer, m_AssetHandle, buffer.m_Handle, gl_binding_index); }

			void Dispatch(const Vectors::UVector3& work_groups = Vectors::UVector3(1, 1, 1)) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t, uint32_t, uint32_t), ComputeShader_Dispatch, m_AssetHandle, work_groups.x, work_groups.y, work_groups.z); }

			void SetBool(const std::string& uniform_name, bool value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, bool), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetInt(const std::string& uniform_name, int32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, int32_t), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetUInt(const std::string& uniform_name, uint32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, uint32_t), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetFloat(const std::string& uniform_name, float value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, float), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetVector2(const std::string& uniform_name, Vectors::Vector2 value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, Vectors::Vector2), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetVector3(const std::string& uniform_name, Vectors::Vector3 value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, Vectors::Vector3), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetVector4(const std::string& uniform_name, Vectors::Vector4 value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, Vectors::Vector4), ComputeShader_SetBool, m_AssetHandle, uniform_name.c_str(), value); }
			void SetMat3(const std::string& uniform_name, const Matrices::Mat3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, const float*), ComputeShader_SetMat3, m_AssetHandle, uniform_name.c_str(), value.Data()); }
			void SetMat4(const std::string& uniform_name, const Matrices::Mat4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, const char*, const float*), ComputeShader_SetMat4, m_AssetHandle, uniform_name.c_str(), value.Data()); }

			AssetType GetAssetType() const override { return AssetType::Compute_Shader; }

		};

		class Shader : public Asset
		{
			friend class Material;

			Shader(uint32_t asset_handle) { m_AssetHandle = asset_handle; }

		public:

			Shader() = default;

			AssetType GetAssetType() const override { return AssetType::Shader; }

		};

		class MaterialUniformBlock
		{

		public:

			MaterialUniformBlock() = delete;
			MaterialUniformBlock(const MaterialUniformBlock&) = default;
			MaterialUniformBlock(MaterialUniformBlock&&) = delete;
			MaterialUniformBlock& operator=(const MaterialUniformBlock&) = default;
			MaterialUniformBlock& operator=(MaterialUniformBlock&&) = delete;

			// Bool types
			void SetBool(const char* name, bool value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Bool, (void*)&value); }
			void SetBVec2(const char* name, const Vectors::BVector2& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::BVec2, (void*)&value); }
			void SetBVec3(const char* name, const Vectors::BVector3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::BVec3, (void*)&value); }
			void SetBVec4(const char* name, const Vectors::BVector4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::BVec4, (void*)&value); }

			// Int types
			void SetInt(const char* name, int value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Int, (void*)&value); }
			void SetIVec2(const char* name, const Vectors::IVector2& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::IVec2, (void*)&value); }
			void SetIVec3(const char* name, const Vectors::IVector3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::IVec3, (void*)&value); }
			void SetIVec4(const char* name, const Vectors::IVector4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::IVec4, (void*)&value); }

			// Unsigned Int types
			void SetUInt(const char* name, uint32_t value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Uint, (void*)&value); }
			void SetUVec2(const char* name, const Vectors::UVector2& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::UVec2, (void*)&value); }
			void SetUVec3(const char* name, const Vectors::UVector3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::UVec3, (void*)&value); }
			void SetUVec4(const char* name, const Vectors::UVector4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::UVec4, (void*)&value); }

			// Float types
			void SetFloat(const char* name, float value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Float, (void*)&value); }
			void SetVec2(const char* name, const Vectors::Vector2& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Vec2, (void*)&value); }
			void SetVec3(const char* name, const Vectors::Vector3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Vec3, (void*)&value); }
			void SetVec4(const char* name, const Vectors::Vector4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Vec4, (void*)&value); }

			// Double types
			void SetDouble(const char* name, double value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Double, (void*)&value); }
			void SetDVec2(const char* name, const Vectors::DVector2& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::DVec2, (void*)&value); }
			void SetDVec3(const char* name, const Vectors::DVector3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::DVec3, (void*)&value); }
			void SetDVec4(const char* name, const Vectors::DVector4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::DVec4, (void*)&value); }

			// Matrix types
			void SetMat3(const char* name, const Matrices::Mat3& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Mat3, (void*)&value); }
			void SetMat4(const char* name, const Matrices::Mat4& value) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const char*, GLSLType, void*), MaterialUniformBlock_SetUniform, m_Handle, name, GLSLType::Mat4, (void*)&value); }

			void SetTexture(const char* name, GLSLType sampler_type, const Texture2D& texture) const;

			void OverrideAlbedoMap(Texture2D albedo_texture) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, uint32_t), MaterialUniformBlock_OverrideAlbedoMap, m_Handle, albedo_texture.m_AssetHandle); }
			void OverrideMetallicMap(Texture2D metallic_texture) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, uint32_t), MaterialUniformBlock_OverrideMetallicMap, m_Handle, metallic_texture.m_AssetHandle); }
			void OverrideNormalMap(Texture2D normal_texture) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, uint32_t), MaterialUniformBlock_OverrideNormalMap, m_Handle, normal_texture.m_AssetHandle); }
			void OverrideAlbedoTint(const Vectors::Vector4& colour) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, const Vectors::Vector4*), MaterialUniformBlock_OverrideAlbedoTint, m_Handle, &colour); }
			void OverrideMetallic(float metallic_factor) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, float), MaterialUniformBlock_OverrideRoughness, m_Handle, metallic_factor); }
			void OverrideRoughness(float roughness_factor) const { ENGINE_SAFE_CALL_VOID(void(*)(void*, float), MaterialUniformBlock_OverrideRoughness, m_Handle, roughness_factor); }

		private:

			MaterialUniformBlock(void* handle) : m_Handle(handle) {}

			void* m_Handle = nullptr;

			friend class Material;
			friend class Components::MeshRendererComponent;
			friend class Components::SkinnedMeshComponent;
		};

		class Material : public Asset
		{

		private:

			friend class Components::MeshRendererComponent;
			friend class Components::SkinnedMeshComponent;

			Material(bool create_new) {};

		public:

			Material() { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), Material_Create, "New Material"); }
			Material(const std::string& material_name) { m_AssetHandle = ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(const char*), Material_Create, material_name.c_str()); }

			void SetShader(const Shader& shader) const { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t, uint32_t), Material_SetShader, m_AssetHandle, shader.m_AssetHandle); }
			Shader GetShader() const { return Shader(ENGINE_SAFE_CALL_RET(uint32_t, uint32_t(*)(uint32_t), Material_GetShader, m_AssetHandle)); }

			void Destroy() { ENGINE_SAFE_CALL_VOID(void(*)(uint32_t), Material_Destroy, m_AssetHandle); m_AssetHandle = NULL_UUID; }

			AssetType GetAssetType() const override { return AssetType::Material_Standard; }

		};
	}

}