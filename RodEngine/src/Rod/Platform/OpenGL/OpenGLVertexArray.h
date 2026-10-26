#pragma once

#include "Rod/Renderer/VertexArray.h"

namespace Rod {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(Scope<VertexBuffer> vertexBuffer) override;
		virtual void SetIndexBuffer(Scope<IndexBuffer> indexBuffer) override;

		virtual const std::vector<Scope<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const IndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer.get(); }
	private:
		uint32_t m_RendererID;
		std::vector<Scope<VertexBuffer>> m_VertexBuffers;
		Scope<IndexBuffer> m_IndexBuffer;

		int m_VertexBufferIndex = 0;
	};

}
