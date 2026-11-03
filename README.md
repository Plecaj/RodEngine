<div align="center">
  <img src="logo.png" alt="RodEngine logo" width="340" />

  # RodEngine

  [![License](https://img.shields.io/github/license/Plecaj/RodEngine?color=green)](LICENSE)
  [![Latest Commit](https://img.shields.io/github/last-commit/Plecaj/RodEngine)](https://github.com/Plecaj/RodEngine/commits/main)
  ![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)
  ![CMake](https://img.shields.io/badge/build-CMake-informational)
  ![OpenGL](https://img.shields.io/badge/rendering-OpenGL-lightgrey)
  ![.NET 9](https://img.shields.io/badge/scripting-.NET%209-purple)
</div>

RodEngine is a C++23 game engine and editor with OpenGL rendering and C# scripting. It supports 2D and 3D scenes, editor play mode, and exporting scenes to a standalone runtime.

The engine is in early development; physics, audio, animation, and prefabs are not implemented yet.

<p align="center">
  <img src="editor.png" alt="RodEngine scene editor" width="900" />
</p>

## Implemented systems

### Editor and scenes

- Dockable editor with a viewport, entity hierarchy, property inspector, and content browser.
- Viewport entity picking through framebuffer IDs, transform gizmos, and drag-and-drop support for scenes, textures, and models.
- ECS scenes with UUIDs and YAML serialization. Play mode runs a copy of the scene so runtime changes do not affect the editor scene.

### Rendering

- Batched 2D sprite rendering.
- Static GLB mesh rendering with material controls, directional lights, and shadow maps.
- GLSL shader compilation and SPIR-V caching.

### Scripting and runtime

- .NET 9 hosted from C++ for C# entity scripts, with fields editable in the inspector and assembly reloading while the editor is running.
- Script access to entities, transforms, cameras, rendering components, input, and logging.
- A standalone runtime and editor export for packaging a scene with its assets.

Windows and Linux are supported. Linux uses X11 or XWayland; native Wayland support is not available. Rendering requires OpenGL 4.6.

## Tech stack

| Area | Technology |
| --- | --- |
| Languages | C++23, C# |
| Build | CMake, MSVC or Clang, Ninja |
| Rendering | OpenGL 4.6, shaderc, SPIR-V |
| Editor UI | Dear ImGui, ImGuizmo |
| ECS | EnTT |
| Scripting | .NET 9 |
| Scene serialization | yaml-cpp |
| Asset loading | tinygltf, stb_image |
| Logging | spdlog |

## Build

### Requirements

Common requirements: **CMake 3.25+**, **Git**, **Python 3**, and the **.NET 9 SDK**.

| Platform | Build tools and dependencies |
| --- | --- |
| Windows (MSVC) | Visual Studio 2022 with the **Desktop development with C++** workload |
| Windows (Clang) | The same Visual Studio workload, plus LLVM and Ninja on `PATH` |
| Linux (Clang) | Clang with a C++23 standard library, Ninja, X11 development headers (Xrandr, Xinerama, Xcursor, XInput), OpenGL development headers, and Zenity for file dialogs |

### Sources

```sh
git clone --recursive https://github.com/Plecaj/RodEngine.git
cd RodEngine
python RodEngine/vendor/shaderc/utils/git-sync-deps
```

Shaderc requires the dependency sync above in addition to the recursive submodule checkout. After updating an existing clone, run `git submodule update --init --recursive` and repeat the sync.

### Configure and build

Run CMake from the repository root:

| Toolchain | Configure | Debug build |
| --- | --- | --- |
| Windows MSVC | `cmake --preset vs` | `cmake --build --preset vs-debug` |
| Windows Clang | `cmake --preset windows-clang` | `cmake --build --preset windows-clang-debug` |
| Linux Clang | `cmake --preset linux-clang` | `cmake --build --preset linux-clang-debug` |

Windows Clang requires an x64 Visual Studio developer shell with LLVM and Ninja on `PATH`.

Build presets provide `Debug`, `Release`, and `Dist` configurations through the `-debug`, `-release`, and `-dist` suffixes.

For a runtime-only build, configure with `-DROD_BUILD_EDITOR=OFF` and build the `Rod-Runtime` target explicitly.

### Run

Executables are written to `build/vs/bin/<configuration>` for MSVC and `build/clang/bin/<configuration>` for Clang. Run `Rod-Editor` (`Rod-Editor.exe` on Windows) from that directory so it can find its assets.

The bundled `assets/scenes/Example.rod` and `Example3D.rod` scenes can be opened through **File > Open** and run in play mode. `Rod-Runtime` loads `assets/scenes/Startup.rod`, falling back to the 3D example when no startup scene is present.

### Export

**File > Export Game** writes the current scene and its assets to `exports/RodGame` relative to the editor's working directory.

The CMake export target packages the bundled 3D example instead of the current editor scene:

```sh
cmake --build build/clang --config Debug --target RodGame-Export
```

Use `build/vs` for MSVC. Output goes to `<build-directory>/export/RodGame`; run `RodGame` (`RodGame.exe` on Windows) from that directory.

## Tests

The test suite covers math, scene operations, and scene serialization. Dedicated presets use separate build directories and build only the test executable and its dependencies:

```powershell
cmake --preset windows-clang-tests
cmake --build --preset windows-clang-tests
ctest --preset windows-clang-tests
```

Use `vs-tests` for MSVC or `linux-clang-tests` for Linux. Catch2 is fetched during the first test configuration.

GitHub Actions runs the same Debug test presets for all three toolchains on pull requests and pushes to `main`. Test reports and CTest logs are available as workflow artifacts.

## License

RodEngine is licensed under [Apache-2.0](LICENSE). Third-party dependencies and bundled assets retain their own licenses.
