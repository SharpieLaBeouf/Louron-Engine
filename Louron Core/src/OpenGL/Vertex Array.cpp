#include "Vertex Array.h"

namespace Louron {

	VertexArray::VertexArray() {
		glCreateVertexArrays(1, &m_VAO);
	}

	VertexArray::~VertexArray() {

		if (!m_DeleteOnObjectDestroy)
			return;

		glDeleteVertexArrays(1, &m_VAO);

		delete m_IndexBuffer;

		for (auto vertex : m_VertexBuffers)
			delete vertex;
	}

	void VertexArray::Bind() const {
		glBindVertexArray(m_VAO);
	}

	void VertexArray::UnBind() const {
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(VertexBuffer* vertexBuffer) {
		glBindVertexArray(m_VAO);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout) {
			switch (element.Type) {
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex, element.GetComponentCount(), OpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				GLint count = element.GetComponentCount();
				for (int i = 0; i < count; i++) {
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex, count, OpenGLDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)(element.Offset * sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default: L_CORE_WARN("Shader Data Type Not Defined");
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
		glBindVertexArray(0);
		vertexBuffer->Unbind();
	}

	void VertexArray::SetIndexBuffer(IndexBuffer* indexBuffer) {

		glBindVertexArray(m_VAO);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}