cmake_minimum_required(VERSION 3.2)
project(JakeLang)

set(CMAKE_BUILD_TYPE Debug)

# ---------- Source Files ---------- #

add_executable(jake-lang 
    "src/main.cpp"
    "src/debug.cpp"
    "src/timer.cpp"
    "src/environment.cpp"
    "src/scanner.cpp"
    "src/parser.cpp"
    "src/print.cpp"
)

# ---------- Linker Config ---------- #

target_include_directories(jake-lang PRIVATE "./include/" "./external/variant/include/")

if (MSVC)
    target_compile_options(jake-lang PRIVATE /std:c++17 /W3 /wd4244)
else()
    target_compile_options(jake-lang PRIVATE -std=c++17 -Wall)
endif()

# ---------- Emscripten ---------- #

# Set the source directory
set(TEST_DIR test)

add_custom_target(
    copy_tests
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${TEST_DIR}
        ${CMAKE_BINARY_DIR}/${TEST_DIR}
)

add_dependencies(jake-lang copy_tests)

# ---------- Emscripten ---------- #

if (EMSCRIPTEN)
    target_include_directories(jake-lang PRIVATE "C:/Program Files/emsdk/upstream/emscripten/cache/sysroot/include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sNO_EXIT_RUNTIME=1 -sEXPORTED_RUNTIME_METHODS=\"[ccall]\" -std=c++17")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

