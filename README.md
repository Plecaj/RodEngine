<div align="center">
  <img src="logo.png" alt="RodEngine logo" width="340" />

  # RodEngine

  Custom C++ game engine and editor focused on rendering, tooling, scripting, and engine architecture.

  [![License](https://img.shields.io/github/license/Plecaj/RodEngine?color=green)](LICENSE)
  [![Latest Commit](https://img.shields.io/github/last-commit/Plecaj/RodEngine)](https://github.com/Plecaj/RodEngine/commits/main)
  ![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)
  ![CMake](https://img.shields.io/badge/build-CMake-informational)
  ![OpenGL](https://img.shields.io/badge/rendering-OpenGL-lightgrey)
  ![.NET 9](https://img.shields.io/badge/scripting-.NET%209-purple)
</div>

RodEngine is a hands-on engine project focused on building the core systems directly: rendering, scene management, editor tools, C# scripting, and the runtime/export pipeline.

It is not intended to compete with engines like Unity or Unreal. The project is still in early development, so APIs and workflows may change.

> **Project status:** early development. The editor and runtime are usable, but the API is not stable yet.

<p align="center">
  <img src="editor.png" alt="RodEditor screenshot" width="900" />
</p>

## At A Glance

| Area | Current state |
| --- | --- |
| Editor | Dockable ImGui editor with viewport, hierarchy, properties, content browser, toolbar, debug, guide, and performance panels |
| Scene system | ECS scenes with UUID entities, serializable components, editor/runtime update paths, and scene copying for play mode |
| Rendering | Batched 2D renderer, 3D mesh renderer, directional lights, shadow depth passes, GLB loading, and material data |
| Scripting | .NET 9 C# scripting hosted from C++, with reflected script fields and runtime reload support |
| Runtime | Separate game runtime executable with editor export into `exports/RodGame` |

## Highlights

| Feature | Why it matters |
| --- | --- |
| Editor play mode copies the scene | Runtime changes do not overwrite editor data |
| Framebuffer entity picking | The editor can select entities directly from the viewport |
| YAML scene serialization | Scenes store entities, components, materials, scripts, and script field values |
| Shaderc + SPIR-V cache | Multi-stage GLSL shaders are compiled and cached instead of rebuilt every time |
| C# metadata reflection | Script classes and serializable fields show up in the editor |
| Runtime/export path | Scenes can be packaged with assets and launched outside the editor |

## Implemented Systems

### Editor

- Custom title bar and dockspace layout.
- Scene hierarchy and properties editing.
- Content browser with drag and drop for scenes, textures, and GLB meshes.
- Viewport rendering through a framebuffer.
- Entity picking from an integer framebuffer attachment.
- Transform gizmos with ImGuizmo.
- Play and stop workflow.
- Scene open, save, save as, and game export.

### Scene System

- ECS model based on `entt`.
- Stable UUIDs for entities.
- Components for transform, camera, sprite renderer, mesh, directional light, and C# script.
- YAML serialization for scene data.
- Separate editor and runtime update paths.
- Scene copying before play mode.

### Rendering

- OpenGL backend with a renderer abstraction layer.
- Batched 2D quad renderer with texture slots and draw stats.
- 3D renderer with mesh submission and material data.
- Directional lights and shadow map depth passes.
- GLB mesh loading through `tinygltf`.
- Material properties for albedo, emissive color, roughness, and metallic.
- Multi-stage GLSL files compiled with `shaderc` into SPIR-V.
- YAML-backed shader cache database.

### C# Scripting

- .NET 9 runtime hosted from C++ through `hostfxr`.
- Script projects built with `dotnet build`.
- Game scripts loaded into a collectible `AssemblyLoadContext`.
- Script classes and serializable fields reflected into editor UI.
- Native calls for logging, input, entities, components, transforms, cameras, sprites, meshes, materials, lights, and script data.
- Script source changes detected while the editor is running.

### Runtime And Export

- `Rod-Runtime` runs scenes without the editor UI.
- Editor export copies the runtime executable, assets, and startup scene.
- Runtime loads `assets/scenes/Startup.rod`.

## Current Limitations

| Area | Current limitation |
| --- | --- |
| Platform | Windows and Linux (X11/XWayland). Native Wayland window management is not supported yet. |
| Rendering backend | The renderer abstraction exists, but OpenGL is the only implemented backend. |
| Asset pipeline | Asset handling is still simple. The editor currently works with scene files, PNG textures, and GLB meshes. |
| Mesh loading | GLB loading is focused on static mesh data and basic material values. It is not a full glTF asset pipeline yet. |
| Gameplay systems | Physics, audio, animation, prefab workflows, and asset importing tools are not implemented yet. |
| Editor UX | The editor is usable, but many workflows are still early and will change as the engine grows. |

## Code Map

| Area | Start here |
| --- | --- |
| Editor shell | `Rod-Editor/src/EditorLayer.cpp` |
| Editor panels | `Rod-Editor/src/Panels/` |
| Scene and ECS | `RodEngine/src/Rod/Scene/` |
| 2D renderer | `RodEngine/src/Rod/Renderer/Renderer2D.cpp` |
| 3D renderer and shadows | `RodEngine/src/Rod/Renderer/Renderer3D.cpp` |
| Shader compilation | `RodEngine/src/Rod/Platform/OpenGL/OpenGLShader.cpp` |
| Shader cache | `RodEngine/src/Rod/Renderer/ShaderCacheDatabase.cpp` |
| C# script hosting | `RodEngine/src/Rod/Scripting/` |
| Managed script API | `Rod-ScriptCore/Source/` |
| Standalone runtime | `Rod-Runtime/src/` |

## Tech Stack

| Area | Technology |
| --- | --- |
| Language | C++23, C# |
| Build | CMake, MSVC or Clang, Ninja, .NET SDK |
| Rendering | OpenGL, shaderc, SPIR-V |
| UI | Dear ImGui, ImGuizmo |
| ECS | entt |
| Scene files | YAML |
| Mesh loading | tinygltf |
| Logging | spdlog |

## Build

### Requirements

Both platforms need **CMake 3.25+**, **Git**, **Python 3**, and **.NET 9 SDK** for C# scripting. Running the engine requires **OpenGL 4.6**.

| Platform | Build tools and dependencies |
| --- | --- |
| Windows | Visual Studio 2022 with Desktop development with C++ |
| Linux | Clang, a C++23 standard library with `std::format`, Ninja, X11 development headers (Xrandr, Xinerama, Xcursor, XInput), OpenGL development headers, and Zenity for file dialogs |

Linux uses X11, including XWayland on Wayland desktops. Native Wayland support is pending.

<details>
<summary>Install Linux dependencies on Fedora</summary>

```bash
sudo dnf install clang cmake ninja-build libstdc++-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel dotnet-sdk-9.0 zenity
```

</details>

### Get the sources

Choose HTTPS or SSH; SSH requires a GitHub SSH key.

| Protocol | Clone command |
| --- | --- |
| HTTPS | `git clone https://github.com/Plecaj/RodEngine.git` |
| SSH | `git clone git@github.com:Plecaj/RodEngine.git` |

Then initialize the submodules and sync shaderc's dependencies. Run these steps for an existing clone too:

```bash
cd RodEngine
git submodule update --init --recursive
python RodEngine/vendor/shaderc/utils/git-sync-deps
```

### Build and run

Run CMake from the repository root:

| Platform | Configure | Debug build | Output directory | Editor executable |
| --- | --- | --- | --- | --- |
| Windows | `cmake --preset vs` | `cmake --build --preset vs-debug` | `build/vs/bin/Debug` | `Rod-Editor.exe` |
| Linux | `cmake --preset linux-clang` | `cmake --build --preset linux-clang-debug` | `build/linux-clang/bin/Debug` | `Rod-Editor` |

Run the editor from its output directory so it can find the copied assets. Both builds also produce `Rod-Runtime` (`.exe` on Windows) and build the C# projects when `dotnet` is available.

For other configurations, replace `-debug` with `-release` or `-dist` in the build preset. The output directory changes to `Release` or `Dist`.

### Runtime and export

For a runtime-only build, append `-DROD_BUILD_EDITOR=OFF` to either configure command.

| Platform | Export a Debug game package |
| --- | --- |
| Windows | `cmake --build build/vs --config Debug --target RodGame-Export` |
| Linux | `cmake --build build/linux-clang --config Debug --target RodGame-Export` |

Packages go to `<build-directory>/export/RodGame`, containing assets and `RodGame.exe` on Windows or `RodGame` on Linux. Run the game from that directory. Use `--config Release` or `--config Dist` for other configurations.

## License

RodEngine is licensed under the MIT License. See [LICENSE](LICENSE).
