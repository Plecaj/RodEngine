#include "rdpch.h"

#include "OpenGLTexture.h"

#include "stb_image.h"

#include <glad/glad.h>

#include <stdexcept>

namespace Rod {
	static constexpr int g_RgbChannelCount = 3;
	static constexpr int g_RgbaChannelCount = 4;

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
	    : m_Width(width), m_Height(height), m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
	{
		RD_PROFILE_FUNCTION();

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

		glTextureStorage2D(
		    m_RendererID,
		    1,
		    m_InternalFormat,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height));

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path)
	{
		RD_PROFILE_FUNCTION();

		int width{}, height{}, channels{};
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;

		{
			RD_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string& path)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		if (!data)
			throw std::runtime_error("Failed to load image: " + path);

		m_Width = static_cast<uint32_t>(width);
		m_Height = static_cast<uint32_t>(height);

		GLenum internalFormat = 0, dataFormat = 0;

		if (channels == g_RgbaChannelCount)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == g_RgbChannelCount)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		if (!internalFormat || !dataFormat)
		{
			stbi_image_free(data);
			throw std::runtime_error("Unsupported texture format: " + path);
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(
		    m_RendererID,
		    1,
		    internalFormat,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height));

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(
		    m_RendererID,
		    0,
		    0,
		    0,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height),
		    dataFormat,
		    GL_UNSIGNED_BYTE,
		    data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		RD_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(const void* data, uint32_t size)
	{

		const uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? g_RgbaChannelCount : g_RgbChannelCount;
		const uint32_t dataSize = m_Width * m_Height * bytesPerPixel;
		const bool isCompleteTexture = size == dataSize;
		RD_CORE_ASSERT(isCompleteTexture, "Data must be entire texture");
		if (!isCompleteTexture)
		{
			RD_CORE_ERROR("Texture data size mismatch. Expected {0} bytes, got {1}.", dataSize, size);
			return;
		}

		glTextureSubImage2D(
		    m_RendererID,
		    0,
		    0,
		    0,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height),
		    m_DataFormat,
		    GL_UNSIGNED_BYTE,
		    data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		RD_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}
}
