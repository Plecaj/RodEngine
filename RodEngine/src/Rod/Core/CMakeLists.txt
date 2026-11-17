target_sources(RodEngine PRIVATE
    Application.cpp
    Layer.cpp
    LayerStack.cpp
    Log.cpp
    UUID.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        Application.h
        BuildMode.h
        Core.h
        EntryPoint.h
        Input.h
        KeyCodes.h
        Layer.h
        LayerStack.h
        Log.h
        MouseButtonCodes.h
        Timestep.h
        UUID.h
        Window.h
)
