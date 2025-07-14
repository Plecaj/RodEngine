#include "rdpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Rod {

    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIdx;
    };

    struct Renderer2DData 
    {
        static constexpr int MAX_QUAD_COUNT = 10000;
        static constexpr int MAX_VERTICES_COUNT = MAX_QUAD_COUNT * 4;
        static constexpr int MAX_INDICES_COUNT = MAX_QUAD_COUNT * 6;
        int MaxTexturesSlots;

        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::vector<Ref<Texture2D>> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        Renderer2DData()    : MaxTexturesSlots(Renderer::GetMaxTextureSlots()), TextureSlots(MaxTexturesSlots){}
    };

    static Renderer2DData* s_Data;

    void Renderer2D::Init()
    {
        RD_PROFILE_FUNCTION();
        s_Data = new Renderer2DData;
        
        std::string vertexShaderSrc = R"(
        #version 330 core

        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;
        layout(location = 2) in vec2 a_TexCoord;
        layout(location = 3) in float a_TexIdx;

        uniform mat4 u_ViewProjection;

        out vec4 v_Color;
        out vec2 v_TexCoord;
        out float v_TexIdx;

        void main()
        {
            v_Color = a_Color;
            v_TexCoord = a_TexCoord;
            v_TexIdx = a_TexIdx;
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
        }
        )";

        std::string fragmentShaderSrc = R"(
        #version 330 core

        layout(location = 0) out vec4 color;

        in vec4 v_Color;
        in vec2 v_TexCoord;
        in float v_TexIdx;

        uniform sampler2D u_Textures[)" + std::to_string(s_Data->MaxTexturesSlots) + R"(];

        void main()
        {
            color = texture(u_Textures[int(v_TexIdx)], v_TexCoord) * v_Color;
        }
        )";

        s_Data->TextureShader = Shader::Create("Texture", vertexShaderSrc, fragmentShaderSrc);
        int* samplers = new int[s_Data->MaxTexturesSlots];
        for (int i = 0; i < s_Data->MaxTexturesSlots; i++) {
            samplers[i] = i;
        }
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetIntArray("u_Textures", samplers, s_Data->MaxTexturesSlots);
        delete[] samplers;

        s_Data->QuadVertexArray = VertexArray::Create();

        s_Data->QuadVertexBuffer = VertexBuffer::Create(sizeof(QuadVertex) * s_Data->MAX_VERTICES_COUNT);
        s_Data->QuadVertexBuffer->SetBufferLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color"},
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIdx"},
            });
        s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

        s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MAX_VERTICES_COUNT];

        uint32_t* quadIndices = new uint32_t[s_Data->MAX_INDICES_COUNT];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data->MAX_INDICES_COUNT; i += 6) {
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

        s_Data->WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));

        s_Data->TextureSlots[0] = s_Data->WhiteTexture;
    }

    void Renderer2D::Shutdown()
    {
        RD_PROFILE_FUNCTION();

        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        RD_PROFILE_FUNCTION();

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        BeginBatch();
    }

    void Renderer2D::EndScene()
    {
        RD_PROFILE_FUNCTION();

        EndBatch();
        Flush();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        if (s_Data->QuadIndexCount + 6 > s_Data->MAX_INDICES_COUNT) {
            EndBatch();
            Flush();
            BeginBatch();
        }

        float whiteTextureIndex = 0.0f;

        s_Data->QuadVertexBufferPtr->Position = position;
        s_Data->QuadVertexBufferPtr->Color = color;
        s_Data->QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x + size.x, position.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = color;
        s_Data->QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = color;
        s_Data->QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x, position.y + size.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = color;
        s_Data->QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = whiteTextureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        if ((s_Data->QuadIndexCount + 6 > s_Data->MAX_INDICES_COUNT) || s_Data->TextureSlotIndex + 1 >= s_Data->MaxTexturesSlots) {
            EndBatch();
            Flush();
            BeginBatch();
        }
    
        float textureIndex = 0;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++) 
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0) 
        {
            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        s_Data->QuadVertexBufferPtr->Position = position;
        s_Data->QuadVertexBufferPtr->Color = tintColor;
        s_Data->QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x + size.x, position.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = tintColor;
        s_Data->QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = tintColor;
        s_Data->QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadVertexBufferPtr->Position = { position.x, position.y + size.y, position.z };
        s_Data->QuadVertexBufferPtr->Color = tintColor;
        s_Data->QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
        s_Data->QuadVertexBufferPtr->TexIdx = textureIndex;
        s_Data->QuadVertexBufferPtr++;

        s_Data->QuadIndexCount += 6;
    }

    // Rotated ones arent supported for now!
    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float radiansRotation, const glm::vec4& color)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, radiansRotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float radiansRotation, const glm::vec4& color)
    {
        RD_PROFILE_FUNCTION();

        s_Data->TextureShader->SetFloat4("u_Color", color);
        s_Data->TextureShader->SetFloat("u_TilingFactor", 1.0f);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), radiansRotation, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_Data->WhiteTexture->Bind();

        s_Data->TextureShader->SetMat4("u_Transform", transform);

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float radiansRotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, radiansRotation, texture, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float radiansRotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        RD_PROFILE_FUNCTION();

        s_Data->TextureShader->SetFloat4("u_Color", tintColor);
        s_Data->TextureShader->SetFloat("u_TilingFactor", tilingFactor);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), radiansRotation, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        s_Data->TextureShader->SetMat4("u_Transform", transform);

        texture->Bind();

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
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

        //s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
    }
}