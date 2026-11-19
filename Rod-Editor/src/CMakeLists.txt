target_sources(Rod-Editor PRIVATE
    "${PROJECT_SOURCE_DIR}/RodEngine/src/Rod/Core/EntryPoint.cpp"
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
