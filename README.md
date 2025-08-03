# RodEngine

[![License](https://img.shields.io/github/license/Plecaj/RodEngine?color=green)](LICENSE) [![Latest Commit](https://img.shields.io/github/last-commit/Plecaj/RodEngine)](https://github.com/Plecaj/RodEngine/commits/main)

<p align="center">
  <img src="logo.png" alt="RodEngine Logo" width="400" />
</p>

**RodEngine** is a custom C++ game engine I'm building from scratch. It's currently based on [The Cherno's](https://www.youtube.com/@TheCherno) game engine series, which serves as the foundation for learning and early development. Once I complete the tutorials, I plan to expand the engine with my own architecture, features, and tools.  

**Initially conceptualized as a 3D engine, my current focus is on developing a solid 2D engine that I can develop and maintain effectively.**

## 🚧 Development Status
> ⚠️ RodEngine is in early development. Expect rapid changes, incomplete systems, and experimental features.
 
## 🔧 Building

#### 🧾 Prerequisites

- **Windows**
- **Visual Studio 2022** (or later)
- **Git** (with submodule support)

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

> ⚠️ If Premake5 is not installed globally — a compatible version is included in the project for convenience.


## 🛠 Tech Stack

- 🖥️ **Platform:** Windows  
- 💻 **Language:** C++  
- 🏗️ **Build System:** Premake5  
- 🎨 **Rendering:** OpenGL  
- 🖱️ **Windowing & Input:** GLFW - Custom Cherno's fork (for Windows no titlebar support)  
- 📐 **Math Library:** GLM  
- 🖥️ **GUI:** ImGui  
- ⚛️ **ECS Library:** entt  
- 📝 **Logging:** spdlog  
- 🖼️ **Image Loading:** stb_image  
## 🎯 Goals

### 🎨 Rendering
- High-performance 2D rendering with support for sprites, tilemaps, and vector graphics  
- Support multiple rendering APIs (OpenGL, Vulkan, DirectX, Metal)  
- Efficient batching and draw call optimization

### 🛠️ Editor & Tools
- Intuitive, built-in scene editor with drag-and-drop functionality  
- Visual UI editor for creating menus and HUDs  
- Real-time preview and live editing  
- Asset management system 

### 🖥️ Platform Support
- Cross-platform support (Windows, Linux, macOS)

### 🎵 Audio
- Flexible audio playback system supporting 2D spatial audio  
- Sound effects and music management with mixing and layering  

### 📜 Scripting
- C# scripting support 
- Hot-reload scripts 
- API designed for easy access to engine features 

### ⚙️ Physics
- 2D physics engine with rigid bodies, colliders, and joints  
- Collision detection and resolution 

### 📊 ECS & Performance
- Rod’s custom ECS
- High-performance data-driven architecture for scalability  
- Multi-threading support and job system 

### 🕹️ Gameplay & UI
- Built-in UI system for menus, buttons, and interactive elements  
- Animation system for sprites and UI components  
- Input handling tailored for keyboard, mouse, and gamepads

## 🙏 Credits

Special thanks to [The Cherno](https://github.com/thecherno) for the incredible game engine tutorial series, which helped kickstart this project and continues to be a valuable learning resource.

---

