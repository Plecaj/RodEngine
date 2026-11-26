target_sources(RodEngine PRIVATE
    AppMain.cpp
    DesktopHost.cpp
    InputState.cpp
    Log.cpp
    UUID.cpp
)

target_sources(RodEngine PUBLIC
    FILE_SET HEADERS
    FILES
        AppMain.h
        BuildMode.h
        Core.h
        DesktopHost.h
        InputState.h
        KeyCodes.h
        Log.h
        MouseButtonCodes.h
        Timestep.h
        UUID.h
        Window.h
)
