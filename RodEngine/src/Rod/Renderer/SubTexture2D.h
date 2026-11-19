#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <span>

namespace Rod {

	class SubTexture2D
	{
		static constexpr size_t s_TexCoordCount = 4;

	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max) noexcept;

		[[nodiscard]] auto GetTexture() const noexcept -> const Ref<Texture2D>& { return m_Texture; }
		[[nodiscard]] auto GetTexCoords() const noexcept -> std::span<const glm::vec2, s_TexCoordCount>
		{
			return m_TexCoords;
		}

		[[nodiscard]] static auto CreateFromCoords(
		    const Ref<Texture2D>& texture,
		    glm::vec2 coords,
		    glm::vec2 cellSize,
		    glm::vec2 spriteSize = {1, 1}) noexcept -> SubTexture2D;

	private:
		Ref<Texture2D> m_Texture;

		std::array<glm::vec2, s_TexCoordCount> m_TexCoords;
	};

}
