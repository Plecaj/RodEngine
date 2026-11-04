#include "rdpch.h"

#include "SubTexture2D.h"

namespace Rod {

	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
	    : m_Texture(texture), m_TexCoords{{{min.x, min.y}, {max.x, min.y}, {max.x, max.y}, {min.x, max.y}}}
	{}

	SubTexture2D SubTexture2D::CreateFromCoords(
	    const Ref<Texture2D>& texture,
	    glm::vec2 coords,
	    const glm::vec2 cellSize,
	    const glm::vec2 spriteSize)
	{
		const auto sheetWidth = static_cast<float>(texture->GetWidth());
		const auto sheetHeight = static_cast<float>(texture->GetHeight());

		const glm::vec2 min = {(coords.x * cellSize.x) / sheetWidth, (coords.y * cellSize.y) / sheetHeight};
		const glm::vec2 max = {
		    ((coords.x + spriteSize.x) * cellSize.x) / sheetWidth,
		    ((coords.y + spriteSize.y) * cellSize.y) / sheetHeight};

		return {texture, min, max};
	}

}
