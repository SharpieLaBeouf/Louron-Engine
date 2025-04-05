#pragma once

// Louron Core Headers
#include "Vertex Array.h"
#include "../Asset/Asset.h"
#include "../Scene/Spatial Partitioning/Bounds.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron
{

	struct SubMesh {

	public:

		SubMesh() = default;
		SubMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
		~SubMesh() = default;

		SubMesh(const SubMesh& other);
		SubMesh& operator=(const SubMesh& other);

		SubMesh(SubMesh&& other) noexcept;
		SubMesh& operator=(SubMesh&& other) noexcept;

		/// <summary>
		/// This will set the triangles of the mesh 
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTriangles(const uint32_t* triangles, GLuint element_count);

		const GLuint* GetTriangles(size_t* triangle_count) const;

		/// <summary>
		/// This will set the vertices of the mesh 
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetVertices(const float* vertices, GLuint element_count);

		/// <summary>
		/// This will set the normals of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetNormals(const float* normals, GLuint element_count);

		/// <summary>
		/// This will set the texture coords of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTextureCoords(const float* texture_coords, GLuint element_count);

		/// <summary>
		/// This will set the tangents of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetTangents(const float* tangents, GLuint element_count);

		/// <summary>
		/// This will set the bi-tangents of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of FLOAT's in the data, NOT total count of vertices.</param>
		void SetBiTangents(const float* bitangents, GLuint element_count);

		/// <summary>
		/// This will set the bone IDs of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of ID's in the data, not the total count of vertices.</param>
		void SetBoneIDs(const glm::ivec4* bone_ids, GLuint element_count);

		/// <summary>
		/// This will set the bone weights of the mesh
		/// locally before uploading to GPU.
		/// </summary>
		/// <param name="element_count">This is the total count of weight's in the data, not the total count of vertices.</param>
		void SetBoneWeights(const glm::vec4* bone_weights, GLuint element_count);

		const float* GetVertices(size_t* vertice_count) const;
		const float* GetNormals(size_t* normals_count) const;
		const float* GetTextureCoords(size_t* tex_coords_count) const;
		const float* GetTangents(size_t* tangents_count) const;
		const float* GetBiTangents(size_t* bitangents_count) const;
		const glm::ivec4* GetBoneIDs(size_t* bone_ids_count) const;
		const glm::vec4* GetBoneWeights(size_t* bone_weights_count) const;

		void RecalculateNormals();

		/// <summary>
		/// This will retrieve all data from the GPU for 
		/// each buffer and store on CPU for modification.
		/// </summary>
		void CopyGPUData();

		/// <summary>
		/// This will clear all CPU memory associated with 
		/// each buffer attachment.
		/// </summary>
		void ClearCPUData();

		/// <summary>
		/// This will submit all changes to GPU of buffers
		/// that have been marked as modified.
		/// </summary>
		void SubmitChangesToGPU(bool clean_up_cpu_data);

		/// <summary>
		/// Set the Vertex Array by copying an existing VAO.
		/// </summary>
		void InitVAO() { m_VAO = std::make_unique<VertexArray>(); }

		/// <summary>
		/// Set the Vertex Array by copying an existing VAO.
		/// </summary>
		void SetVAO(std::unique_ptr<VertexArray> vao) { m_VAO = std::make_unique<VertexArray>(*vao); }

		/// <summary>
		/// Get a const ref to the existing VAO.
		/// </summary>
		const std::unique_ptr<VertexArray>& GetVAO() const { return m_VAO; }

	private:

		std::unique_ptr<VertexArray> m_VAO = nullptr;
		bool m_Modified = false;

	};

	struct StaticMesh : public Asset {

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		std::vector<std::shared_ptr<SubMesh>> SubMeshes{};

		StaticMesh() = default;

		StaticMesh(const StaticMesh& other);
		StaticMesh& operator=(const StaticMesh& other);

		StaticMesh(StaticMesh&& other) noexcept;
		StaticMesh& operator=(StaticMesh&& other) noexcept;

		~StaticMesh() = default;

		Bounds_AABB MeshBounds{};
		bool ModifiedAABB = false;

	};

}