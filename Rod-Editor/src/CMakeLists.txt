target_sources(Rod-Editor PRIVATE
    EditorCommands.cpp
    EditorCommands.h
    EditorInput.cpp
    EditorInput.h
    EditorInteractionState.h
    EditorLayer.cpp
    EditorLayer.h
    EditorLayout.cpp
    EditorLayout.h
    EditorSceneManager.cpp
    EditorSceneManager.h
    EditorViewport.cpp
    EditorViewport.h
    RodEditorApp.cpp
    SceneState.h
)

add_subdirectory(Panels)
