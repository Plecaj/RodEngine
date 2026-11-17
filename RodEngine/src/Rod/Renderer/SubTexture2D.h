#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <span>

namespace Rod {

	class SubTexture2D
	{
		static constexpr size_t TEX_COORD_COUNT = 4;

	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max) noexcept;

		[[nodiscard]] const Ref<Texture2D>& GetTexture() const noexcept { return m_Texture; }
		[[nodiscard]] std::span<const glm::vec2, TEX_COORD_COUNT> GetTexCoords() const noexcept { return m_TexCoords; }

		[[nodiscard]] static SubTexture2D CreateFromCoords(
		    const Ref<Texture2D>& texture,
		    glm::vec2 coords,
		    glm::vec2 cellSize,
		    glm::vec2 spriteSize = {1, 1}) noexcept;

	private:
		Ref<Texture2D> m_Texture;

		std::array<glm::vec2, TEX_COORD_COUNT> m_TexCoords;
	};

}
