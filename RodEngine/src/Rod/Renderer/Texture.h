#pragma once

#include "Rod/Core/Core.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace Rod {

	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() = default;
		Texture(const Texture&) = delete;
		auto operator=(const Texture&) -> Texture& = delete;
		Texture(Texture&&) = delete;
		auto operator=(Texture&&) -> Texture& = delete;

		[[nodiscard]] virtual auto GetWidth() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto GetHeight() const noexcept -> uint32_t = 0;
		[[nodiscard]] virtual auto GetRendererID() const noexcept -> uint32_t = 0;

		virtual void SetData(std::span<const std::byte> data) = 0;

		[[nodiscard]] virtual auto GetPath() const -> std::string = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		[[nodiscard]] virtual auto operator==(const Texture& other) const noexcept -> bool = 0;
	};

	class Texture2D : public Texture
	{
	public:
		[[nodiscard]] static auto Create(uint32_t width, uint32_t height) -> Scope<Texture2D>;
		[[nodiscard]] static auto Create(const std::string& path) -> Scope<Texture2D>;
	};

}
