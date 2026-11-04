#include "rdpch.h"

#include "Renderer2D.h"

#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Rod {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIdx;
		float TilingFactor;

		int EntityID = 0;
	};

	struct Renderer2DData
	{
		static constexpr uint32_t MAX_QUAD_COUNT = 10000;
		static constexpr uint32_t MAX_VERTICES_COUNT = MAX_QUAD_COUNT * Renderer2D::QUAD_VERTEX_COUNT;
		static constexpr uint32_t MAX_INDICES_COUNT = MAX_QUAD_COUNT * Renderer2D::QUAD_INDEX_COUNT;
		static constexpr uint32_t MAX_TEXTURE_SLOTS = 32;

		Scope<VertexArray> QuadVertexArray;
		VertexBuffer* QuadVertexBuffer = nullptr; // Owned by QuadVertexArray.
		Scope<UniformBuffer> QuadUniformBuffer;
		Scope<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		std::vector<QuadVertex> QuadVertexBufferBase;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::vector<Ref<Texture2D>> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		std::array<glm::vec4, Renderer2D::QUAD_VERTEX_COUNT> QuadVertexPositions;
		Renderer2D::Statistics Stats;
	};

	static Scope<Renderer2DData> g_Data;

	void Renderer2D::Init()
	{
		RD_PROFILE_FUNCTION();
		g_Data = CreateScope<Renderer2DData>();
		g_Data->TextureSlots.resize(g_Data->MAX_TEXTURE_SLOTS);

		InitShader();
		InitSamplers();
		InitQuadVertexArray();
		InitWhiteTexture();
		InitQuadVertexPositions();
	}

	void Renderer2D::InitShader()
	{
		Shader::ShaderOptions shaderOptions;
		shaderOptions.OptimizationLevel = Shader::OptimizationLevel::Performance;
		shaderOptions.GenerateDebugInfo = false;

		g_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl", shaderOptions);
		g_Data->QuadUniformBuffer = UniformBuffer::Create(sizeof(glm::mat4));
	}

	void Renderer2D::InitSamplers()
	{
		std::array<int, Renderer2DData::MAX_TEXTURE_SLOTS> samplers;
		for (uint32_t i = 0; i < Renderer2DData::MAX_TEXTURE_SLOTS; i++)
			samplers[i] = static_cast<int>(i);

		g_Data->TextureShader->Bind();
		g_Data->TextureShader->SetIntArray("u_Textures", samplers.data(), static_cast<uint32_t>(samplers.size()));
	}

	void Renderer2D::InitQuadVertexArray()
	{
		g_Data->QuadVertexArray = VertexArray::Create();

		auto vertexBuffer = VertexBuffer::Create(sizeof(QuadVertex) * g_Data->MAX_VERTICES_COUNT);
		vertexBuffer->SetBufferLayout(
		    {{ShaderDataType::Float3, "a_Position"},
		     {ShaderDataType::Float4, "a_Color"},
		     {ShaderDataType::Float2, "a_TexCoord"},
		     {ShaderDataType::Float, "a_TexIdx"},
		     {ShaderDataType::Float, "a_TilingFactor"},
		     {ShaderDataType::Int, "a_EntityID"}});

		g_Data->QuadVertexBuffer = vertexBuffer.get();
		g_Data->QuadVertexArray->AddVertexBuffer(std::move(vertexBuffer));
		g_Data->QuadVertexBufferBase.resize(Renderer2DData::MAX_VERTICES_COUNT);

		std::vector<uint32_t> quadIndices(Renderer2DData::MAX_INDICES_COUNT);
		constexpr std::array<uint32_t, QUAD_INDEX_COUNT> quadPattern = {0, 1, 2, 2, 3, 0};
		uint32_t offset = 0;
		for (uint32_t i = 0; i < Renderer2DData::MAX_INDICES_COUNT; i += QUAD_INDEX_COUNT)
		{
			for (uint32_t corner = 0; corner < QUAD_INDEX_COUNT; ++corner)
				quadIndices[i + corner] = offset + quadPattern[corner];
			offset += QUAD_VERTEX_COUNT;
		}

		auto quadIB = IndexBuffer::Create(quadIndices.data(), Renderer2DData::MAX_INDICES_COUNT);
		g_Data->QuadVertexArray->SetIndexBuffer(std::move(quadIB));
	}

	void Renderer2D::InitWhiteTexture()
	{
		g_Data->WhiteTexture = Texture2D::Create(1, 1);
		constexpr uint32_t whiteTextureData = 0xffffffff;
		g_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

		g_Data->TextureSlots[0] = g_Data->WhiteTexture;
	}

	void Renderer2D::InitQuadVertexPositions()
	{
		constexpr std::array<glm::vec4, QUAD_VERTEX_COUNT> positions = {
		    {{-0.5f, -0.5f, 0.0f, 1.0f},
		     {0.5f, -0.5f, 0.0f, 1.0f},
		     {0.5f, 0.5f, 0.0f, 1.0f},
		     {-0.5f, 0.5f, 0.0f, 1.0f}}};
		g_Data->QuadVertexPositions = positions;
	}

	void Renderer2D::Shutdown()
	{
		RD_PROFILE_FUNCTION();

		g_Data.reset();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		RD_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

		g_Data->TextureShader->Bind();

		g_Data->QuadUniformBuffer->BindBase(0);
		g_Data->QuadUniformBuffer->SetData(&viewProj, sizeof(viewProj));

		BeginBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		RD_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetViewProjection();

		g_Data->TextureShader->Bind();

		g_Data->QuadUniformBuffer->BindBase(0);
		g_Data->QuadUniformBuffer->SetData(&viewProj, sizeof(viewProj));

		BeginBatch();
	}

	void Renderer2D::EndScene()
	{
		RD_PROFILE_FUNCTION();

		if (g_Data->QuadIndexCount == 0)
			return;

		EndBatch();
		Flush();
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		RD_PROFILE_FUNCTION();
		if (g_Data->QuadIndexCount + QUAD_INDEX_COUNT > g_Data->MAX_INDICES_COUNT)
		{
			EndBatch();
			Flush();
			BeginBatch();
		}

		constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
		constexpr float whiteTextureIndex = 0.0f;
		constexpr float tilingFactor = 1.0f;

		for (uint32_t i = 0; i < QUAD_VERTEX_COUNT; i++)
		{
			g_Data->QuadVertexBufferPtr->Position = transform * g_Data->QuadVertexPositions[i];
			g_Data->QuadVertexBufferPtr->Color = color;
			g_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
			g_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data->QuadVertexBufferPtr->EntityID = entityID;
			g_Data->QuadVertexBufferPtr++;
		}

		g_Data->QuadIndexCount += QUAD_INDEX_COUNT;

		g_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(
	    const glm::mat4& transform,
	    const Ref<Texture2D>& texture,
	    float tilingFactor,
	    const glm::vec4& tintColor,
	    int entityID)
	{
		RD_PROFILE_FUNCTION();
		if ((g_Data->QuadIndexCount + QUAD_INDEX_COUNT > g_Data->MAX_INDICES_COUNT))
		{
			EndBatch();
			Flush();
			BeginBatch();
		}

		constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < g_Data->TextureSlotIndex; i++)
		{
			if (*g_Data->TextureSlots[i] == *texture)
			{
				textureIndex = static_cast<float>(i);
				break;
			}
		}

		if (textureIndex == 0 && g_Data->TextureSlotIndex < g_Data->MAX_TEXTURE_SLOTS - 1)
		{
			textureIndex = static_cast<float>(g_Data->TextureSlotIndex);
			g_Data->TextureSlots[g_Data->TextureSlotIndex] = texture;
			g_Data->TextureSlotIndex++;
		}
		else if (textureIndex == 0)
		{
			EndBatch();
			Flush();
			BeginBatch();
		}

		for (uint32_t i = 0; i < QUAD_VERTEX_COUNT; i++)
		{
			g_Data->QuadVertexBufferPtr->Position = transform * g_Data->QuadVertexPositions[i];
			g_Data->QuadVertexBufferPtr->Color = tintColor;
			g_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
			g_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
			g_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
			g_Data->QuadVertexBufferPtr->EntityID = entityID;
			g_Data->QuadVertexBufferPtr++;
		}

		g_Data->QuadIndexCount += QUAD_INDEX_COUNT;

		g_Data->Stats.QuadCount++;
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if (src.Texture)
			DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		else
			DrawQuad(transform, src.Color, entityID);
	}

	void Renderer2D::ResetStats()
	{
		g_Data->Stats = {};
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return g_Data->Stats;
	}

	void Renderer2D::BeginBatch()
	{
		RD_PROFILE_FUNCTION();

		g_Data->TextureSlotIndex = 1;
		g_Data->QuadIndexCount = 0;
		g_Data->QuadVertexBufferPtr = g_Data->QuadVertexBufferBase.data();
	}

	void Renderer2D::EndBatch()
	{
		RD_PROFILE_FUNCTION();

		const auto vertexCount = static_cast<size_t>(g_Data->QuadVertexBufferPtr - g_Data->QuadVertexBufferBase.data());
		const auto dataSize = static_cast<uint32_t>(vertexCount * sizeof(QuadVertex));
		g_Data->QuadVertexBuffer->SetData(g_Data->QuadVertexBufferBase.data(), dataSize);
	}

	void Renderer2D::Flush()
	{
		RD_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < g_Data->TextureSlotIndex; i++)
			g_Data->TextureSlots[i]->Bind(i);

		g_Data->QuadVertexArray->Bind();
		g_Data->QuadVertexArray->GetIndexBuffer()->Bind();

		RenderCommand::DrawIndexed(*g_Data->QuadVertexArray, g_Data->QuadIndexCount);
		g_Data->Stats.DrawCalls++;
	}
}
