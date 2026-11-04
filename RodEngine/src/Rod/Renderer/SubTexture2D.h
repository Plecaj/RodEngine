#pragma once

#include "Texture.h"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>

namespace Rod {

	class SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

		[[nodiscard]] const Ref<Texture2D>& GetTexture() const noexcept { return m_Texture; }
		[[nodiscard]] const glm::vec2* GetTexCoords() const noexcept { return m_TexCoords.data(); }

		[[nodiscard]] static SubTexture2D CreateFromCoords(
		    const Ref<Texture2D>& texture,
		    glm::vec2 coords,
		    glm::vec2 cellSize,
		    glm::vec2 spriteSize = {1, 1});

	private:
		Ref<Texture2D> m_Texture;

		static constexpr size_t TEX_COORD_COUNT = 4;
		std::array<glm::vec2, TEX_COORD_COUNT> m_TexCoords;
	};

}