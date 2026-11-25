#include "rdpch.h"

#include "Renderer2D.h"

#include "RenderDevice.h"
#include "ShaderLoader.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <span>

namespace Rod {

	Renderer2D::Renderer2D(const RenderDevice& device, const ShaderLoader& shaders)
	    : m_Device(device), m_VertexArray(device.CreateVertexArray()),
	      m_CameraBuffer(device.CreateUniformBuffer(sizeof(glm::mat4))), m_Shader(shaders.Load("Texture.glsl"))
	{
		constexpr uint32_t vertexCapacity = s_QuadCapacity * s_VerticesPerQuad;
		constexpr uint32_t indexCapacity = s_QuadCapacity * s_IndicesPerQuad;
		const BufferLayout layout{
		    {ShaderDataType::Float3, "a_Position"},
		    {ShaderDataType::Float4, "a_Color"},
		    {ShaderDataType::Float2, "a_TexCoord"},
		    {ShaderDataType::Float, "a_TexIdx"},
		    {ShaderDataType::Float, "a_TilingFactor"},
		    {ShaderDataType::Int, "a_EntityID"}};
		RD_CORE_ASSERT(
		    layout.GetStride() == sizeof(Vertex) && layout.GetElements().back().Offset == offsetof(Vertex, EntityID),
		    "Sprite vertex storage does not match the GPU layout");

		auto vertexBuffer = device.CreateVertexBuffer(sizeof(Vertex) * vertexCapacity);
		vertexBuffer->SetBufferLayout(layout);
		m_VertexArray->AddVertexBuffer(std::move(vertexBuffer));

		constexpr std::array<uint32_t, s_IndicesPerQuad> pattern{0, 1, 2, 2, 3, 0};
		std::vector<uint32_t> indices;
		indices.reserve(indexCapacity);
		for (uint32_t offset = 0; offset < vertexCapacity; offset += s_VerticesPerQuad)
		{
			for (const auto index : pattern)
				indices.push_back(offset + index);
		}
		m_VertexArray->SetIndexBuffer(device.CreateIndexBuffer(indices));

		constexpr auto white = std::array<uint8_t, 4>{255, 255, 255, 255};
		auto whiteTexture = device.CreateTexture2D(1, 1);
		whiteTexture->SetData(std::as_bytes(std::span{white}));
		std::array<int, s_TextureCapacity> samplers{};
		std::iota(samplers.begin(), samplers.end(), 0);
		m_Shader->SetIntArray("u_Textures", samplers);
		m_Vertices.reserve(vertexCapacity);
		m_Textures.reserve(s_TextureCapacity);
		m_Textures.emplace_back(std::move(whiteTexture));
	}

	void Renderer2D::Begin(const glm::mat4& viewProjection)
	{
		m_CameraBuffer->SetData(std::as_bytes(std::span{&viewProjection, 1}));
		m_Vertices.clear();
		m_Textures.resize(1);
		m_Stats = {};
	}

	void Renderer2D::SubmitQuad(
	    const glm::mat4& transform,
	    const Ref<Texture2D>& texture,
	    float tiling,
	    const glm::vec4& tint,
	    int entityID)
	{
		if (m_Vertices.size() == static_cast<size_t>(s_QuadCapacity) * s_VerticesPerQuad)
			Flush();

		const auto slot = static_cast<float>(AssignTextureSlot(texture));
		constexpr std::array<glm::vec2, s_VerticesPerQuad> uvs{
		    {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}};
		for (const auto uv : uvs)
		{
			const auto corner = glm::vec4(uv - glm::vec2{0.5f}, 0.0f, 1.0f);
			m_Vertices.push_back(
			    {.Position = glm::vec3(transform * corner),
				 .Tint = tint,
				 .UV = uv,
				 .TextureSlot = slot,
				 .Tiling = tiling,
				 .EntityID = entityID});
		}

		++m_Stats.QuadCount;
	}

	auto Renderer2D::AssignTextureSlot(const Ref<Texture2D>& texture) -> uint32_t
	{
		if (!texture)
			return 0;

		const auto found = std::ranges::find_if(m_Textures, [&](const auto& slot) { return *slot == *texture; });
		if (found != m_Textures.end())
			return static_cast<uint32_t>(found - m_Textures.begin());

		if (m_Textures.size() == s_TextureCapacity)
			Flush();

		const auto slot = static_cast<uint32_t>(m_Textures.size());
		m_Textures.push_back(texture);
		return slot;
	}

	void Renderer2D::End()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		if (m_Vertices.empty())
			return;

		m_Shader->Bind();
		m_CameraBuffer->BindBase(0);
		for (uint32_t slot = 0; slot < m_Textures.size(); ++slot)
			m_Textures[slot]->Bind(slot);

		m_VertexArray->GetVertexBuffers().front()->SetData(std::as_bytes(std::span{m_Vertices}));
		const auto indexCount = static_cast<uint32_t>(m_Vertices.size() / s_VerticesPerQuad) * s_IndicesPerQuad;
		m_Device.DrawIndexed(*m_VertexArray, indexCount);
		++m_Stats.DrawCalls;
		m_Vertices.clear();
		m_Textures.resize(1);
	}

}
