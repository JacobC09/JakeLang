cmake_minimum_required(VERSION 3.25)
project(JakeLang)

set(CMAKE_BUILD_TYPE Debug)

# ---------- Source Files ---------- #
add_executable(jake-lang 
    "src/main.cpp"
    "src/debug.cpp"
    "src/timer.cpp"
    "src/environment.cpp"
    "src/varient.cpp"
)

# ---------- Linker Config ---------- #
target_include_directories(jake-lang PRIVATE "./include/")
target_compile_options(jake-lang PRIVATE -Wall -std=c++17 -Wno-reorder)

# ---------- Emscripten ---------- #
if (EMSCRIPTEN)
    target_include_directories(jake-lang PRIVATE "C:/Program Files/emsdk/upstream/emscripten/cache/sysroot/include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sNO_EXIT_RUNTIME=1 -sEXPORTED_RUNTIME_METHODS=\"[ccall]\" -std=c++17")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

