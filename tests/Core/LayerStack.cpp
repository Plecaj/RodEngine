#include "rdpch.h"

#include "Rod/Core/LayerStack.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <string_view>

namespace {
	class DestructionOrder
	{
	public:
		void Append(char id) noexcept
		{
			if (m_Size < m_Entries.size())
				m_Entries[m_Size] = id;
			++m_Size;
		}

		[[nodiscard]] auto View() const noexcept -> std::string_view
		{
			if (m_Size > m_Entries.size())
				return "<overflow>";
			return {m_Entries.data(), m_Size};
		}

	private:
		std::array<char, 4> m_Entries{};
		std::size_t m_Size = 0;
	};

	class TrackedLayer : public Rod::Layer
	{
	public:
		TrackedLayer(char id, DestructionOrder& destroyed) : m_Id(id), m_Destroyed(destroyed) {}
		~TrackedLayer() noexcept override { m_Destroyed.Append(m_Id); }
		TrackedLayer(const TrackedLayer&) = delete;
		auto operator=(const TrackedLayer&) -> TrackedLayer& = delete;
		TrackedLayer(TrackedLayer&&) = delete;
		auto operator=(TrackedLayer&&) -> TrackedLayer& = delete;

	private:
		char m_Id;
		DestructionOrder& m_Destroyed;
	};
}

TEST_CASE("Layer stack destroys layers before overlays in stack order", "[core][layers]")
{
	DestructionOrder destroyed;
	{
		Rod::LayerStack stack;
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('a', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('A', destroyed));
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('b', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('B', destroyed));
	}
	REQUIRE(destroyed.View() == "ABab");
}

TEST_CASE("Layer stack pops destroy only entries from the matching partition", "[core][layers]")
{
	DestructionOrder destroyed;
	{
		Rod::LayerStack stack;
		auto layer = Rod::CreateScope<TrackedLayer>('L', destroyed);
		auto overlay = Rod::CreateScope<TrackedLayer>('O', destroyed);
		auto& layerObserver = *layer;
		auto& overlayObserver = *overlay;
		stack.PushLayer(std::move(layer));
		stack.PushOverlay(std::move(overlay));
		stack.PopOverlay(layerObserver);
		stack.PopLayer(overlayObserver);
		REQUIRE(destroyed.View().empty());

		stack.PopLayer(layerObserver);
		REQUIRE(destroyed.View() == "L");
		stack.PopOverlay(overlayObserver);
		REQUIRE(destroyed.View() == "LO");
	}
	REQUIRE(destroyed.View() == "LO");
}

TEST_CASE("Clearing a layer stack is ordered and allows reuse", "[core][layers]")
{
	DestructionOrder destroyed;
	{
		Rod::LayerStack stack;
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('a', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('A', destroyed));
		stack.Clear();
		REQUIRE(destroyed.View() == "Aa");
		stack.Clear();
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('b', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('B', destroyed));
	}
	REQUIRE(destroyed.View() == "AaBb");
}
