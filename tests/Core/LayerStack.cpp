#include "rdpch.h"

#include "Rod/Core/LayerStack.h"

#include <catch2/catch_test_macros.hpp>

namespace {
	class TrackedLayer : public Rod::Layer
	{
	public:
		TrackedLayer(char id, std::string& destroyed) : m_Id(id), m_Destroyed(destroyed) {}
		~TrackedLayer() override { m_Destroyed += m_Id; }
		TrackedLayer(const TrackedLayer&) = delete;
		TrackedLayer& operator=(const TrackedLayer&) = delete;
		TrackedLayer(TrackedLayer&&) = delete;
		TrackedLayer& operator=(TrackedLayer&&) = delete;

	private:
		char m_Id;
		std::string& m_Destroyed;
	};
}

TEST_CASE("Layer stack destroys layers before overlays in stack order", "[core][layers]")
{
	std::string destroyed;
	{
		Rod::LayerStack stack;
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('a', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('A', destroyed));
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('b', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('B', destroyed));
	}
	REQUIRE(destroyed == "ABab");
}

TEST_CASE("Layer stack pops destroy only entries from the matching partition", "[core][layers]")
{
	std::string destroyed;
	{
		Rod::LayerStack stack;
		auto layer = Rod::CreateScope<TrackedLayer>('L', destroyed);
		auto overlay = Rod::CreateScope<TrackedLayer>('O', destroyed);
		auto& layerObserver = *layer;
		auto& overlayObserver = *overlay;
		stack.PushLayer(std::move(layer));
		stack.PushOverlay(std::move(overlay));
		stack.PopOverlay(&layerObserver);
		stack.PopLayer(&overlayObserver);
		REQUIRE(destroyed.empty());

		stack.PopLayer(&layerObserver);
		REQUIRE(destroyed == "L");
		stack.PopOverlay(&overlayObserver);
		REQUIRE(destroyed == "LO");
	}
	REQUIRE(destroyed == "LO");
}

TEST_CASE("Clearing a layer stack is ordered and allows reuse", "[core][layers]")
{
	std::string destroyed;
	{
		Rod::LayerStack stack;
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('a', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('A', destroyed));
		stack.Clear();
		REQUIRE(destroyed == "Aa");
		stack.Clear();
		stack.PushOverlay(Rod::CreateScope<TrackedLayer>('b', destroyed));
		stack.PushLayer(Rod::CreateScope<TrackedLayer>('B', destroyed));
	}
	REQUIRE(destroyed == "AaBb");
}
