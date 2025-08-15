#pragma once

namespace Rod {

    class BuildMode {
    public:
        enum class Mode { Editor, Sandbox };

        static Mode& GetMode() {
            static Mode s_Mode = Mode::Sandbox; 
            return s_Mode;
        }

        static void SetMode(Mode mode) {
            GetMode() = mode;
        }
    };

}