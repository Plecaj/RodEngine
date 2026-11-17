target_sources(RodEngine PRIVATE
    Buffer.cpp
    EditorCamera.cpp
    FrameBuffer.cpp
    Mesh.cpp
    RenderCommand.cpp
    Renderer.cpp
    Renderer2D.cpp
    Renderer3D.cpp
    RendererAPI.cpp
    SceneRenderer.cpp
    Shader.cpp
    ShaderCacheDatabase.cpp
    ShaderCacheDatabase.h
    SubTexture2D.cpp
    Texture.cpp
    VertexArray.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        Buffer.h
        Camera.h
        EditorCamera.h
        FrameBuffer.h
        GraphicsContext.h
        Material.h
        Mesh.h
        RenderCommand.h
        Renderer.h
        Renderer2D.h
        Renderer3D.h
        RendererAPI.h
        RendererLimits.h
        SceneRenderer.h
        Shader.h
        SubTexture2D.h
        Texture.h
        VertexArray.h
)
