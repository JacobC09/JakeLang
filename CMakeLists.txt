cmake_minimum_required(VERSION 3.10)
project(JakeLang)

set(CMAKE_BUILD_TYPE Debug)

# ---------- Source Files ---------- #

add_executable(jake-lang 
    "src/main.cpp"
    "src/debug.cpp"
    "src/state.cpp"
    "src/scanner.cpp"
    "src/parser.cpp"
    "src/util.cpp"
    "src/print.cpp"
    "src/compiler.cpp"
    "src/interpreter.cpp"
)

# ---------- Linker Config ---------- #

target_include_directories(jake-lang PRIVATE "./include/" "./external/variant/include/")

if (MSVC)
    target_compile_options(jake-lang PRIVATE /std:c++17 /W3 /wd4244)
else()
    target_compile_options(jake-lang PRIVATE -std=c++17 -Wall -Wno-sometimes-uninitialized)
endif()

# ---------- Tests ---------- #

set(TEST_DIR test)

if (MSVC)
    set(TEST_DEST Debug/${TEST_DIR})
else()
    set(TEST_DEST ${TEST_DIR})
endif()

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/${TEST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_DEST}
)

add_dependencies(jake-lang copy_assets)

# ---------- Emscripten ---------- #

if (EMSCRIPTEN)
    target_include_directories(jake-lang PRIVATE "C:/Program Files/emsdk/upstream/emscripten/cache/sysroot/include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sNO_EXIT_RUNTIME=1 -sEXPORTED_RUNTIME_METHODS=\"[ccall]\" -std=c++17")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

