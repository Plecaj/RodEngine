# RodEngine

[![License](https://img.shields.io/github/license/Plecaj/RodEngine?color=green)](LICENSE) [![Latest Commit](https://img.shields.io/github/last-commit/Plecaj/RodEngine)](https://github.com/Plecaj/RodEngine/commits/main)

<p align="center">
  <img src="logo.png" alt="RodEngine Logo" width="400" />
</p>

**RodEngine** is a custom C++ game engine I'm building from scratch. It's currently based on [The Cherno's](https://www.youtube.com/@TheCherno) game engine series, which serves as the foundation for learning and early development. Once I complete the tutorials, I plan to expand the engine with my own architecture, features, and tools.  

## 🚧 Development Status
> ⚠️ RodEngine is in early development. Expect rapid changes, incomplete systems, and experimental features.
 
## 🔧 Building

#### 🧾 Prerequisites

- **Windows**
- **Visual Studio 2022** 
- **Git**
- **Python 3.13+**
- **CMake 3.30+**
- **Premake5**

#### 🧪 Steps to Build

1. **Clone the repository (with submodules):**
   
   ```bash
   git clone --recurse-submodules https://github.com/Plecaj/RodEngine.git
   ```

2. **Navigate to the project root folder:**

   ```bash
   cd RodEngine
   ```

3. **Navigate into the `scripts/` directory:**

   ```bash
   cd scripts
   ```

4. **Run the Windows setup script to generate project files:**

   ```bash
   Win-GenProjects.bat
   ```

5. **Open the generated solution file:**

   After running the script, a `.sln` file will be created in the root directory. Open it using **Visual Studio**.

6. **Build the solution:**

   Once the solution is open, set your desired configuration (**Debug**, **Release**, etc.) and build the project.


## 🛠 Tech Stack

- 🖥️ **Platform:** Windows  
- 💻 **Language:** C++  
- 🏗️ **Build System:** Premake5  
- 🎨 **Rendering:** OpenGL  
- 🖱️ **Windowing & Input:** GLFW (with minor changes for no windows titlebar support)
- 📐 **Math Library:** GLM  
- 🖥️ **GUI:** ImGui, ImGuizmo  
- ⚛️ **ECS Library:** entt  
- 📝 **Logging:** spdlog  
- 🖼️ **Image Loading:** stb_image  
- 🔧 **Shader Compilation:** Shaderc

## 🎯 Goals

### 🎨 Rendering
- High-performance 3D rendering with support for meshes, models, materials, and textures  
- Physically-Based Rendering (PBR) pipeline with real-time lighting and shadows  
- Support for multiple rendering APIs (OpenGL, Vulkan, DirectX, Metal)  
- Efficient batching, frustum culling, and draw call optimization  
- Post-processing effects: bloom, HDR, motion blur, and SSAO  

### 🛠️ Editor & Tools
- Built-in 3D scene editor with drag-and-drop functionality  
- Visual UI editor for interfaces, menus, and HUDs  
- Real-time preview and live editing of scenes and assets  
- Asset management for models, textures, shaders, animations, and audio  

### 🖥️ Platform Support
- Cross-platform support: Windows, Linux, macOS

### 🎵 Audio
- 3D spatial audio engine with sound positioning  
- Sound effects and music management with mixing, layering, and attenuation  

### 📜 Scripting
- C# scripting support with hot-reload capability  
- Easy-to-use API for engine features and gameplay logic  

### ⚙️ Physics
- 3D physics engine with rigid bodies, colliders, and joints  
- Collision detection, resolution, and physics simulation  

### 📊 ECS & Performance
- Rod’s custom ECS architecture for scalable, data-driven performance  
- Multi-threading support and job system for parallelized tasks  


## 🙏 Credits

Special thanks to [The Cherno](https://github.com/thecherno) for the incredible game engine tutorial series, which helped kickstart this project and continues to be a valuable learning resource.

---

