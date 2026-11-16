<div align="center">
  <img src="logo.png" alt="RodEngine" width="280" />

# RodEngine

**C++23 game engine · OpenGL 4.6 · C#/.NET 9 scripting · Scene editor**

</div>

C++ game engine and editor for 2D/3D scenes. ECS architecture, OpenGL renderer, C# scripting, scene serialization, standalone runtime.

**Status:** In development. Physics, audio, animation, and prefabs not implemented.

![RodEngine Editor](editor.png)

## Features

**Editor**
- Dear ImGui docking, viewport, entity list, component inspector, content browser
- Entity picking, transform gizmos, asset drag-and-drop
- Scene creation, loading, saving, isolated play mode

**Rendering**
- OpenGL 4.6 backend
- 2D sprite batching, textures, tinting, tiling
- Static GLB meshes, materials, directional lighting, shadow maps
- GLSL → SPIR-V compilation, shader caching

**Scenes**
- EnTT ECS, UUID-based entities, component system
- YAML scene serialization (`.rod`)
- Perspective and orthographic cameras

**Scripting**
- C# scripting via hosted .NET 9 runtime
- C++/C# interop, entity and component APIs
- Script fields in inspector, assembly reloading

**Runtime**
- Separate runtime executable
- Scene and asset export from editor

## Project Structure

| Directory | Purpose |
|---|---|
| `RodEngine/` | Core engine, rendering, ECS, scripting |
| `Rod-Editor/` | Editor, panels, scene assets |
| `Rod-Runtime/` | Standalone scene runner |
| `Rod-ScriptCore/` | C# API and native interop |
| `cmake/` | Build configuration, dependencies |
| `tests/` | Catch2 unit tests |

**Dependencies:** GLFW, Glad, Dear ImGui, ImGuizmo, EnTT, GLM, shaderc, yaml-cpp, tinygltf, stb_image, spdlog.

## Build

### Requirements

- **Common:** CMake 3.25+, Git, Python 3, .NET 9 SDK
- **Graphics:** OpenGL 4.6-capable GPU and driver
- **Windows:** Visual Studio 2022, Desktop development with C++
- **Windows Clang:** LLVM, Ninja, x64 Visual Studio developer environment
- **Linux:** Clang, Ninja, OpenGL/X11 development packages, Zenity for file dialogs

Linux: X11 or XWayland; no native Wayland backend.

### Clone

```bash
git clone --recurse-submodules https://github.com/Plecaj/RodEngine.git
cd RodEngine
python RodEngine/vendor/shaderc/utils/git-sync-deps
```

### Configure & Compile

| Toolchain | Configure | Build (Debug) |
|---|---|---|
| Windows / MSVC | `cmake --preset vs` | `cmake --build --preset vs-debug` |
| Windows / Clang | `cmake --preset windows-clang` | `cmake --build --preset windows-clang-debug` |
| Linux / Clang | `cmake --preset linux-clang` | `cmake --build --preset linux-clang-debug` |

Build configurations: `Debug`, `Release`, `Dist`.

## Usage

### Run Editor

Executable locations:
- **MSVC:** `build/vs/bin/<configuration>/`
- **Clang:** `build/clang/bin/<configuration>/`

Linux / Clang example:

```bash
cd build/clang/bin/Debug
./Rod-Editor
```

Run from the executable directory; assets loaded relative to the working directory.

Engine resources (shaders, fonts, icons, and the scripting core) live in `RodAssets/` beside the executables. Project files live in their own `Assets/` folder. Open `Samples/Samples.rodproj` with `Rod-Launcher` to try the bundled scenes, or run `Rod-Runtime Samples/Samples.rodproj` from the executable directory.

Project manifests use the `.rodproj` extension and contain `Name`, `AssetsPath`, `StartupScenePath`, `ScriptsProjectPath`, and `ShadersPath`. Relative paths resolve from the manifest's directory. The editor and runtime accept a manifest path; a packaged game loads `Game.rodproj` beside its executable automatically.

### Export

**File > Export Game** builds the project's scripts and packages the current scene into `exports/RodGame` beside the editor. The package contains `RodGame`, `Game.rodproj`, project assets, compiled scripts and their dependencies, the selected shaders, and the managed scripting core under `RodAssets/`. Script sources, intermediate build files, and caches are excluded.

Exports target the current operating system and require an installed .NET 9 runtime; players do not need the .NET SDK. Export first assembles and validates a temporary package, then replaces an existing package after success. Symbolic links and asset references outside the packaged project are rejected.

**CMake:**

```bash
cmake --build build/clang --config Debug --target RodGame-Export
```

Use `build/vs` for MSVC. Output goes to `<build-directory>/export/RodGame`; run `RodGame` (`RodGame.exe` on Windows) without arguments. The package can be moved and launched from another working directory.

The same exporter is available from the command line:

```sh
Rod-Package path/to/Game.rodproj path/to/output
```

Replace `build/clang` with `build/vs` for MSVC.

## Tests

Catch2 + CTest. Math, scenes, serialization, rendering utilities, scripting interop.

Linux / Clang:

```bash
cmake --preset linux-clang-tests
cmake --build --preset linux-clang-tests
ctest --preset linux-clang-tests
```

Other presets: `vs-tests`, `windows-clang-tests`.

Linux sanitizer configuration: `linux-clang-sanitizers` (ASan, LSan, UBSan).

GitHub Actions: Windows MSVC, Windows Clang, Linux Clang, Linux sanitizers.

## License

[Apache-2.0](LICENSE). Third-party dependencies and assets retain their respective licenses.
