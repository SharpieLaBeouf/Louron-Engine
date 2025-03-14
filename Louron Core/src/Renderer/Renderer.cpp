#include "Renderer.h"

// Louron Core Headers
#include "../OpenGL/Buffer.h"
#include "../OpenGL/Vertex Array.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	static std::unique_ptr<VertexArray> s_DebugCubeVAO;
	static std::unique_ptr<VertexArray> s_DebugSphereVAO; // TODO: Implement this
	static std::unique_ptr<VertexArray> s_SkyboxVAO;

	RenderPassStats Renderer::s_RenderStats = {};

	void Renderer::Init() 
	{
		if (!s_DebugCubeVAO) 
		{

			// Define the vertices of the cube
			GLfloat cubeVertices[] = {
				// Vertices of a unit cube
				-0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				-0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f
			};

			// Define the indices for the edges of the cube
			GLuint cubeIndices[] = {
				0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
				4, 5, 5, 6, 6, 7, 7, 4, // Top face
				0, 4, 1, 5, 2, 6, 3, 7  // Side edges
			};

			s_DebugCubeVAO = std::make_unique<VertexArray>();
			s_DebugCubeVAO->Bind();

			VertexBuffer* vbo = new VertexBuffer(cubeVertices, sizeof(cubeVertices));
			BufferLayout layout = {
				{ ShaderDataType::Float3, "aPos" }
			};
			vbo->SetLayout(layout);

			IndexBuffer* ebo = new IndexBuffer(cubeIndices, sizeof(cubeIndices) / sizeof(GLuint));

			s_DebugCubeVAO->AddVertexBuffer(vbo);
			s_DebugCubeVAO->SetIndexBuffer(ebo);

			s_DebugCubeVAO->UnBind();
		}

		if (!s_SkyboxVAO)
		{
			static float skyboxVertices[] = {
				// positions          
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};

			// Create Buffer
			VertexBuffer* vbo = new VertexBuffer(skyboxVertices, sizeof(skyboxVertices) / sizeof(float));
			vbo->SetLayout({ { ShaderDataType::Float3, "aPos" } });

			// Create Vertex Array and Assign Buffer
			s_SkyboxVAO = std::make_unique<VertexArray>();
			s_SkyboxVAO->AddVertexBuffer(vbo);
		}
	}

	void Renderer::ClearColour(const glm::vec4 colour) 
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void Renderer::ClearBuffer(GLbitfield mask) 
	{
		glClear(mask);
	}

	void Renderer::DrawDebugCubeTriangles(bool is_depth_pass)
	{
		s_DebugCubeVAO->Bind();
		glDrawElements(GL_TRIANGLES, s_DebugCubeVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.Debug_Individual_DrawCalls++;
		s_RenderStats.Debug_Geometry_TriangleCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() / 3;
		s_RenderStats.Debug_Geometry_VerticeCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount();

		if (is_depth_pass)
			s_RenderStats.Debug_Geometry_Depth_Rendered++;
		else
			s_RenderStats.Debug_Geometry_Colour_Rendered++;
	}

	void Renderer::DrawDebugCubeLines()
	{
		s_DebugCubeVAO->Bind();
		glDrawElements(GL_LINES, s_DebugCubeVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.Debug_Individual_DrawCalls++;
		s_RenderStats.Debug_Geometry_Colour_Rendered++;
		s_RenderStats.Debug_Geometry_LineCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() / 2;
		s_RenderStats.Debug_Geometry_VerticeCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount();
	}

	void Renderer::DrawDebugSphereTriangles(bool is_depth_pass)
	{
		s_DebugSphereVAO->Bind();

		glDrawElements(GL_TRIANGLES, s_DebugSphereVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.Debug_Individual_DrawCalls++;
		s_RenderStats.Debug_Geometry_TriangleCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount() / 3;
		s_RenderStats.Debug_Geometry_VerticeCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount();

		if (is_depth_pass)
			s_RenderStats.Debug_Geometry_Depth_Rendered++;
		else
			s_RenderStats.Debug_Geometry_Colour_Rendered++;
	}

	void Renderer::DrawDebugSphereLines()
	{
		s_DebugSphereVAO->Bind();

		glDrawElements(GL_LINES, s_DebugSphereVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
		
		s_RenderStats.Debug_Individual_DrawCalls++;
		s_RenderStats.Debug_Geometry_Colour_Rendered++;
		s_RenderStats.Debug_Geometry_LineCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount() / 2;
		s_RenderStats.Debug_Geometry_VerticeCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount();
	}

	static GLuint s_DebugCubeInstanceBuffers = -1;
	void Renderer::DrawInstancedDebugCube(std::vector<glm::mat4> transforms) 
	{
		if (transforms.empty())
			return;

		if (s_DebugCubeInstanceBuffers == -1) {

			glGenBuffers(1, &s_DebugCubeInstanceBuffers);
			glBindBuffer(GL_ARRAY_BUFFER, s_DebugCubeInstanceBuffers);
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);
		}
		else {

			glBindBuffer(GL_ARRAY_BUFFER, s_DebugCubeInstanceBuffers);

			GLint bufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			if (bufferSize < transforms.size() * sizeof(glm::mat4)) {
				// Reallocate buffer with the new size
				glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_DYNAMIC_DRAW);
			}
			else {
				// Update existing buffer with new data
				glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), &transforms[0]);
			}
		}

		// Additional validation
		if (glIsBuffer(s_DebugCubeInstanceBuffers) == GL_FALSE) {
			L_CORE_ERROR("Buffer is not valid after binding or updating data.");
			return;
		}

		s_DebugCubeVAO->Bind();

		// Set vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)0);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(1 * vec4Size));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(2 * vec4Size));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(3 * vec4Size));

		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);

		// DRAW CALL
		glDrawElementsInstanced(GL_LINES, s_DebugCubeVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, static_cast<GLuint>(transforms.size()));

		// Reset state after drawing
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		// Optionally reset divisor values if needed
		glVertexAttribDivisor(1, 0);
		glVertexAttribDivisor(2, 0);
		glVertexAttribDivisor(3, 0);
		glVertexAttribDivisor(4, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		s_RenderStats.Debug_Instanced_DrawCalls++;
		s_RenderStats.Debug_Geometry_Colour_Instanced++;
		s_RenderStats.Debug_Geometry_LineCount += (s_DebugCubeVAO->GetIndexBuffer()->GetCount() / 2) * static_cast<GLuint>(transforms.size());
		s_RenderStats.Debug_Geometry_VerticeCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() * static_cast<GLuint>(transforms.size());
	}

	void Renderer::DrawSkybox() 
	{
		if (!s_SkyboxVAO)
			return;

		s_SkyboxVAO->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		s_SkyboxVAO->UnBind();

		s_RenderStats.Individual_DrawCalls++;
		s_RenderStats.Geometry_Colour_Rendered++;
		s_RenderStats.Geometry_Colour_TriangleCount += 12;
		s_RenderStats.Geometry_Colour_VerticeCount += 36;
	}

	void Renderer::DrawSubMesh(const VertexArray& sub_mesh, bool is_depth_pass)
	{
		sub_mesh.Bind();
		glDrawElements(GL_TRIANGLES, sub_mesh.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.Individual_DrawCalls++;

		if (is_depth_pass)
		{
			s_RenderStats.Geometry_Depth_Rendered++;
			s_RenderStats.Geometry_Depth_TriangleCount += sub_mesh.GetIndexBuffer()->GetCount() / 3;
			s_RenderStats.Geometry_Depth_VerticeCount += sub_mesh.GetIndexBuffer()->GetCount();
		}
		else
		{
			s_RenderStats.Geometry_Colour_Rendered++;
			s_RenderStats.Geometry_Colour_TriangleCount += sub_mesh.GetIndexBuffer()->GetCount() / 3;
			s_RenderStats.Geometry_Colour_VerticeCount += sub_mesh.GetIndexBuffer()->GetCount();
		}

	}

	void Renderer::DrawSubMesh(std::shared_ptr<SubMesh> sub_mesh, bool is_depth_pass)
	{
		DrawSubMesh(*sub_mesh->GetVAO(), is_depth_pass);
	}

	static GLuint s_MeshInstanceBuffers = -1;

	void Renderer::DrawInstancedSubMesh(const VertexArray& sub_mesh, std::vector<glm::mat4> transforms)
	{
		if (transforms.empty())
			return;

		if (s_MeshInstanceBuffers == -1) {

			glGenBuffers(1, &s_MeshInstanceBuffers);
			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);
		}
		else {

			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);

			GLint bufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			if (bufferSize < transforms.size() * sizeof(glm::mat4)) {
				// Reallocate buffer with the new size
				glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_DYNAMIC_DRAW);
			}
			else {
				// Update existing buffer with new data
				glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), &transforms[0]);
			}
		}

		// Additional validation
		if (glIsBuffer(s_MeshInstanceBuffers) == GL_FALSE) {
			L_CORE_ERROR("Buffer is not valid after binding or updating data.");
			return;
		}

		sub_mesh.Bind();

		// Set vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)0);

		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(1 * vec4Size));

		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(2 * vec4Size));

		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(3 * vec4Size));

		glVertexAttribDivisor(7,	1);
		glVertexAttribDivisor(8,	1);
		glVertexAttribDivisor(9,	1);
		glVertexAttribDivisor(10,	1);

		// DRAW CALL
		glDrawElementsInstanced(GL_TRIANGLES, sub_mesh.GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, static_cast<GLuint>(transforms.size()));

		// Reset state after drawing
		glDisableVertexAttribArray(7);
		glDisableVertexAttribArray(8);
		glDisableVertexAttribArray(9);
		glDisableVertexAttribArray(10);

		// Optionally reset divisor values if needed
		glVertexAttribDivisor(7,	0);
		glVertexAttribDivisor(8,	0);
		glVertexAttribDivisor(9,	0);
		glVertexAttribDivisor(10,	0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		s_RenderStats.Instanced_DrawCalls++;

		s_RenderStats.Geometry_Colour_Instanced += static_cast<GLuint>(transforms.size());
		s_RenderStats.Geometry_Colour_TriangleCount += (sub_mesh.GetIndexBuffer()->GetCount() / 3) * static_cast<GLuint>(transforms.size());
		s_RenderStats.Geometry_Colour_VerticeCount += sub_mesh.GetIndexBuffer()->GetCount() * static_cast<GLuint>(transforms.size());
	}

	void Renderer::DrawInstancedSubMesh(std::shared_ptr<SubMesh> sub_mesh, std::vector<glm::mat4> transforms)
	{
		DrawInstancedSubMesh(*sub_mesh->GetVAO(), transforms);
	}

	void Renderer::CleanupRenderData() 
	{

		if (s_MeshInstanceBuffers != -1) {
			glDeleteBuffers(1, &s_MeshInstanceBuffers);
			s_MeshInstanceBuffers = -1;
		}
	}

	void Renderer::ClearRenderStats() { s_RenderStats = {}; }

	const RenderPassStats& Renderer::GetFrameRenderStats() { return s_RenderStats; }

	void Renderer::CheckOpenGLError()
	{
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::string error_message;
			switch (error)
			{
			case GL_INVALID_ENUM:
				error_message = "GL_INVALID_ENUM: An unacceptable value has been specified for an enumerated argument.";
				break;
			case GL_INVALID_VALUE:
				error_message = "GL_INVALID_VALUE: A numeric argument is out of range.";
				break;
			case GL_INVALID_OPERATION:
				error_message = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
				break;
			case GL_STACK_OVERFLOW:
				error_message = "GL_STACK_OVERFLOW: Command caused a stack overflow.";
				break;
			case GL_STACK_UNDERFLOW:
				error_message = "GL_STACK_UNDERFLOW: Command caused a stack underflow.";
				break;
			case GL_OUT_OF_MEMORY:
				error_message = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error_message = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
				break;
			case GL_CONTEXT_LOST:
				error_message = "GL_CONTEXT_LOST: The OpenGL context has been lost, due to a graphics card reset.";
				break;
			default:
				error_message = "Unknown OpenGL error.";
				break;
			}

			L_CORE_ERROR("OpenGL Error ({}): {}", std::to_string(error), error_message);
		}
	}

}