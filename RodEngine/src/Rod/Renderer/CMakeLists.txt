target_sources(RodEngine PRIVATE
    EditorCamera.cpp
    Mesh.cpp
    Renderer2D.cpp
    Renderer3D.cpp
    SceneRenderer.cpp
    ShaderCache.cpp
    ShaderLoader.cpp
    ShaderStage.h
    SubTexture2D.cpp
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
        RenderDevice.h
        Renderer2D.h
        Renderer3D.h
        SceneRenderer.h
        Shader.h
        ShaderCache.h
        ShaderLoader.h
        SubTexture2D.h
        Texture.h
        VertexArray.h
)
