#include "rdpch.h"

#include "OpenGLTexture.h"

#include "Rod/Platform/StbImage.h"

#include <glad/glad.h>

#include <algorithm>
#include <stdexcept>

namespace Rod {
	static constexpr int g_RgbChannelCount = 3;
	static constexpr int g_RgbaChannelCount = 4;

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
	    : m_Width(width), m_Height(height), m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
	{
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
		int width{}, height{}, channels{};
		const ImagePixels data(stbi_load(path.c_str(), &width, &height, &channels, 0));

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
			throw std::runtime_error("Unsupported texture format: " + path);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(
		    m_RendererID,
		    1,
		    internalFormat,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height));

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		const auto rowSize = static_cast<size_t>(m_Width) * static_cast<size_t>(channels);
		const std::span pixels(data.get(), rowSize * m_Height);
		for (uint32_t row = 0; row < m_Height / 2; ++row)
		{
			const auto top = pixels.subspan(row * rowSize, rowSize);
			const auto bottom = pixels.subspan((m_Height - 1 - row) * rowSize, rowSize);
			std::ranges::swap_ranges(top, bottom);
		}
		OpenGLTexture2D::SetData(std::as_bytes(pixels));
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(std::span<const std::byte> data)
	{

		RD_CORE_ASSERT(
		    data.size_bytes() == static_cast<size_t>(m_Width) * m_Height *
		                             (m_DataFormat == GL_RGBA ? g_RgbaChannelCount : g_RgbChannelCount),
		    "Data must be entire texture");

		GLint unpackAlignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage2D(
		    m_RendererID,
		    0,
		    0,
		    0,
		    static_cast<GLsizei>(m_Width),
		    static_cast<GLsizei>(m_Height),
		    m_DataFormat,
		    GL_UNSIGNED_BYTE,
		    data.data());
		glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}
