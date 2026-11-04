#pragma once

#include "Rod/Core/Core.h"

#include <cstdint>
#include <string>

namespace Rod {

	class Texture
	{
	public:
		Texture() = default;
		virtual ~Texture() = default;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

		[[nodiscard]] virtual uint32_t GetWidth() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetHeight() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetRendererID() const noexcept = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		[[nodiscard]] virtual std::string GetPath() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		[[nodiscard]] virtual bool operator==(const Texture& other) const noexcept = 0;
	};

	class Texture2D : public Texture
	{
	public:
		[[nodiscard]] static Scope<Texture2D> Create(uint32_t width, uint32_t height);
		[[nodiscard]] static Scope<Texture2D> Create(const std::string& path);
	};

}