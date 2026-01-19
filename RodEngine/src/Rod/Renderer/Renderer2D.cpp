#include "rdpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "RendererLimits.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Rod {

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIdx;
        float TilingFactor;

        // TODO: EnitytID should be stripped in runtime due to pointless performance overhead
        int EntityID = 0;
    };

    struct Renderer2DData 
    {
        static constexpr int MAX_QUAD_COUNT = 10000;
        static constexpr int MAX_VERTICES_COUNT = MAX_QUAD_COUNT * 4;
        static constexpr int MAX_INDICES_COUNT = MAX_QUAD_COUNT * 6;
        static constexpr int MAX_TEXTURE_SLOTS = 32;

        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<UniformBuffer> QuadUniformBuffer;  
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::vector<Ref<Texture2D>> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];
        Renderer2D::Statistics Stats;
    };

    static Renderer2DData* s_Data;

    void Renderer2D::Init()
    {
        RD_PROFILE_FUNCTION();
        s_Data = new Renderer2DData;
        s_Data->TextureSlots.resize(s_Data->MAX_TEXTURE_SLOTS);
       
        InitShader();
        InitSamplers();
        InitQuadVertexArray();
        InitWhiteTexture();
        InitQuadVertexPositions();
    }


    void Renderer2D::InitShader()
    {
        Shader::ShaderOptions shaderOptions;
        shaderOptions.OptimizationLevel = Shader::OptimalizationLevel::Performance;
        shaderOptions.GenerateDebugInfo = false;

        s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl", shaderOptions);
        s_Data->QuadUniformBuffer = UniformBuffer::Create(sizeof(glm::mat4));
    }

    void Renderer2D::InitSamplers()
    {
        int* samplers = new int[s_Data->MAX_TEXTURE_SLOTS];
        for (int i = 0; i < s_Data->MAX_TEXTURE_SLOTS; i++)
            samplers[i] = i;

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetIntArray("u_Textures", samplers, s_Data->MAX_TEXTURE_SLOTS);

        delete[] samplers;
    }

    void Renderer2D::InitQuadVertexArray()
    {
        s_Data->QuadVertexArray = VertexArray::Create();

        s_Data->QuadVertexBuffer = VertexBuffer::Create(sizeof(QuadVertex) * s_Data->MAX_VERTICES_COUNT);
        s_Data->QuadVertexBuffer->SetBufferLayout(
            {
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float4, "a_Color"},
                { ShaderDataType::Float2, "a_TexCoord" },
                { ShaderDataType::Float, "a_TexIdx"},
                { ShaderDataType::Float, "a_TilingFactor"},
                { ShaderDataType::Int, "a_EntityID"}
            });

        s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);
        s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MAX_VERTICES_COUNT];

        // Generate indices
        uint32_t* quadIndices = new uint32_t[s_Data->MAX_INDICES_COUNT];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data->MAX_INDICES_COUNT; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> QuadIB = IndexBuffer::Create(quadIndices, s_Data->MAX_INDICES_COUNT);
        s_Data->QuadVertexArray->SetIndexBuffer(QuadIB);
        delete[] quadIndices;
    }

    void Renderer2D::InitWhiteTexture()
    {
        s_Data->WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

        s_Data->TextureSlots[0] = s_Data->WhiteTexture;
    }

    void Renderer2D::InitQuadVertexPositions()
    {
        s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
    }

    void Renderer2D::Shutdown()
    {
        RD_PROFILE_FUNCTION();

        delete s_Data;
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        RD_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

        s_Data->TextureShader->Bind();

        s_Data->QuadUniformBuffer->BindBase(0);
        s_Data->QuadUniformBuffer->SetData(&viewProj, sizeof(viewProj));

        BeginBatch();
    }

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
        RD_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.GetViewProjection();

        s_Data->TextureShader->Bind();

        s_Data->QuadUniformBuffer->BindBase(0);
        s_Data->QuadUniformBuffer->SetData(&viewProj, sizeof(viewProj));

        BeginBatch();
    }

    void Renderer2D::EndScene()
    {
        RD_PROFILE_FUNCTION();

        if (s_Data->QuadIndexCount == 0) return;

        EndBatch();
        Flush();
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
    {
        RD_PROFILE_FUNCTION();
        if (s_Data->QuadIndexCount + 6 > s_Data->MAX_INDICES_COUNT) {
            EndBatch();
            Flush();
            BeginBatch();
        }

        constexpr uint32_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f }
        };
        constexpr float whiteTextureIndex = 0.0f;
        constexpr float tilingFactor = 1.0f;

        for (int i = 0; i < quadVertexCount; i++) {
            s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->Color = color;
            s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
            s_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr->EntityID = entityID;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;

        s_Data->Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        RD_PROFILE_FUNCTION();
        if ((s_Data->QuadIndexCount + 6 > s_Data->MAX_INDICES_COUNT)) {
            EndBatch();
            Flush();
            BeginBatch();
        }

        constexpr uint32_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f }
        };

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0 && s_Data->TextureSlotIndex < s_Data->MAX_TEXTURE_SLOTS - 1)
        {
            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }
        else if (textureIndex == 0) {
            EndBatch();
            Flush();
            BeginBatch();
        }

        for (int i = 0; i < quadVertexCount; i++) {
            s_Data->QuadVertexBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->Color = tintColor;
            s_Data->QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
            s_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr->EntityID = entityID;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;

        s_Data->Stats.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
    {
        if (src.Texture)
            DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);

        DrawQuad(transform, src.Color, entityID);
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data->Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data->Stats;
    }

    void Renderer2D::BeginBatch()
    {
        RD_PROFILE_FUNCTION();

        s_Data->TextureSlotIndex = 1;
        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;
    }

    void Renderer2D::EndBatch()
    {
        RD_PROFILE_FUNCTION();

        uint32_t dataSize = (uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase;
        s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);
    }

    void Renderer2D::Flush()
    {
        RD_PROFILE_FUNCTION();

        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++) 
            s_Data->TextureSlots[i]->Bind(i);

        RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
        s_Data->Stats.DrawCalls++;
    }
}