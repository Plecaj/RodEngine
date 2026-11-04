option(ROD_ENABLE_SANITIZERS "Enable AddressSanitizer, LeakSanitizer, and UBSan on Linux." OFF)

if(NOT ROD_ENABLE_SANITIZERS)
    return()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
   NOT CMAKE_C_COMPILER_ID MATCHES "^(Clang|GNU)$" OR
   NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    message(FATAL_ERROR "ROD_ENABLE_SANITIZERS requires Linux with Clang or GCC. Use WSL2 on Windows.")
endif()

include(CMakePushCheckState)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined -fno-sanitize-recover=undefined")
set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address,undefined")
check_c_source_compiles("int main(void) { return 0; }" ROD_C_SANITIZERS_AVAILABLE)
check_cxx_source_compiles("int main() { return 0; }" ROD_CXX_SANITIZERS_AVAILABLE)
cmake_pop_check_state()

if(NOT ROD_C_SANITIZERS_AVAILABLE OR NOT ROD_CXX_SANITIZERS_AVAILABLE)
    message(FATAL_ERROR "Sanitizer compile/link check failed. Install your compiler's ASan and UBSan runtimes.")
endif()

# Set these before adding subdirectories so native dependencies are instrumented too.
# ASan includes LeakSanitizer on Linux; detect_leaks is explicit in the presets.
add_compile_options(
    "$<$<COMPILE_LANGUAGE:C,CXX>:-fsanitize=address,undefined;-fno-sanitize-recover=undefined;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;-g>"
)
add_link_options(-fsanitize=address,undefined)
