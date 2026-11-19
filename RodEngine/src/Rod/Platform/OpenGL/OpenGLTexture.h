#pragma once

#include "Rod/Renderer/Texture.h"

#include <glad/glad.h>

namespace Rod {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		explicit OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D() override;
		OpenGLTexture2D(const OpenGLTexture2D&) = delete;
		auto operator=(const OpenGLTexture2D&) -> OpenGLTexture2D& = delete;
		OpenGLTexture2D(OpenGLTexture2D&&) = delete;
		auto operator=(OpenGLTexture2D&&) -> OpenGLTexture2D& = delete;

		[[nodiscard]] auto GetWidth() const noexcept -> uint32_t override { return m_Width; }
		[[nodiscard]] auto GetHeight() const noexcept -> uint32_t override { return m_Height; }
		[[nodiscard]] auto GetRendererID() const noexcept -> uint32_t override { return m_RendererID; };

		void SetData(std::span<const std::byte> data) override;

		[[nodiscard]] auto GetPath() const -> std::string override { return m_Path; };

		void Bind(uint32_t slot = 0) const override;

		[[nodiscard]] auto operator==(const Texture& other) const noexcept -> bool override
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
