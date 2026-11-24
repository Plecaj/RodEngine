target_sources(RodEngine PRIVATE
    Entity.cpp
    Scene.cpp
    SceneCamera.cpp
    SceneSerializer.cpp
    Serialization/ComponentSerializers.h
    Serialization/YamlVectors.h
    Serialization/AssetPaths.cpp
    Serialization/TagSerializer.cpp
    Serialization/TransformSerializer.cpp
    Serialization/CameraSerializer.cpp
    Serialization/SpriteRendererSerializer.cpp
    Serialization/MeshSerializer.cpp
    Serialization/DirectionalLightSerializer.cpp
    Serialization/ScriptSerializer.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        Components.h
        Entity.h
        Scene.h
        SceneCamera.h
        SceneSerializer.h
)
