# RodEngine

**RodEngine** is a custom C++ game engine I'm building from scratch. It's currently based on [The Cherno's](https://www.youtube.com/@TheCherno) game engine series, which serves as the foundation for learning and early development. Once I complete the tutorials, I plan to expand the engine with my own architecture, features, and tools.  


## 🚧 Development Status

> ⚠️ RodEngine is in early development. Expect rapid changes, incomplete systems, and experimental features.
 
## 🔧 Building

RodEngine uses **Premake5** as its build system. A setup script is included for easy project generation on Windows.

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

> ✅ The `scripts/Win-GenProjects.bat` script runs **Premake5** and sets up everything you need to start developing with RodEngine.

> ⚠️ If Premake5 is not installed globally, don't worry — a compatible version is included in the project for convenience.


## 🛠 Tech Stack

- **Platform:** Windows
- **Language:** C++
- **Build System:** Premake5  
- **Rendering:** OpenGL 
- **Windowing & Input:** GLFW  
- **Math Library:** GLM  
- **GUI:** ImGui
- **ECS library:** entt 
- **Logging:** spdlog  
- **Image Loading:** stb_image  

## 🎯 Goals

- Both 2D and 3D rendering
- Support for other rendering API's
- Support other platforms - linux, mac, mobile
- Nice built-in scene editor
- Audio playback control
- C# Scripting
- Physics system
- Using Rod ECS library not external one



## 🙏 Credits

Special thanks to [The Cherno](https://github.com/thecherno) for the incredible game engine tutorial series, which helped kickstart this project and continues to be a valuable learning resource.

---

