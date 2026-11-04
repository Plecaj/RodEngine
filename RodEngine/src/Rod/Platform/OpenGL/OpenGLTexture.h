#pragma once

#include "Rod/Renderer/Texture.h"

#include <glad/glad.h>

namespace Rod {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		explicit OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D() override;
		OpenGLTexture2D(const OpenGLTexture2D&) = delete;
		OpenGLTexture2D& operator=(const OpenGLTexture2D&) = delete;
		OpenGLTexture2D(OpenGLTexture2D&&) = delete;
		OpenGLTexture2D& operator=(OpenGLTexture2D&&) = delete;

		[[nodiscard]] virtual uint32_t GetWidth() const noexcept override { return m_Width; }
		[[nodiscard]] virtual uint32_t GetHeight() const noexcept override { return m_Height; }
		[[nodiscard]] virtual uint32_t GetRendererID() const noexcept override { return m_RendererID; };

		virtual void SetData(const void* data, uint32_t size) override;

		[[nodiscard]] virtual std::string GetPath() const override { return m_Path; };

		virtual void Bind(uint32_t slot = 0) const override;

		[[nodiscard]] virtual bool operator==(const Texture& other) const noexcept override
		{
			const auto* texture = dynamic_cast<const OpenGLTexture2D*>(&other);
			if (!texture)
				return false;
			return m_RendererID == texture->m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_RendererID = 0;
		GLenum m_InternalFormat = 0, m_DataFormat = 0;
	};
}
