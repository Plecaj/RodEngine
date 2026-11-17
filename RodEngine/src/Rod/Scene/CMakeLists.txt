target_sources(RodEngine PRIVATE
    Entity.cpp
    Scene.cpp
    SceneCamera.cpp
    SceneSerializer.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        Components.h
        Entity.h
        Scene.h
        SceneCamera.h
        SceneSerializer.h
        ScriptableEntity.h
)
