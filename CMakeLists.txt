cmake_minimum_required(VERSION 3.28.3)

project(LouronEngine)

set(CMAKE_CXX_STANDARD 20)

# Determine platform
if(WIN32)
    set(PLATFORM windows)
elseif(UNIX)
    set(PLATFORM linux)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Optimisation Flags
if(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
elseif(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/s")
    set(CMAKE_C_FLAGS_RELEASE "/O2 -DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 -DNDEBUG" CACHE STRING "" FORCE)
endif()

# Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# INTERPROCEDURAL_OPTIMIZATION
cmake_policy(SET CMP0069 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

if(ipo_supported)
    set(ENABLE_IPO TRUE)
else()
    message(WARNING "IPO / LTO not supported: ${ipo_error}")
endif()

# Default to Debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# ---------------------------------------------------------------------------
# External: GLFW
# ---------------------------------------------------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(External/glfw)
set_target_properties(glfw PROPERTIES FOLDER "External/glfw")

# ---------------------------------------------------------------------------
# External: ASSIMP
# ---------------------------------------------------------------------------
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(External/assimp)
set_target_properties(assimp PROPERTIES FOLDER "External/assimp")

# ---------------------------------------------------------------------------
# External: GLAD
# ---------------------------------------------------------------------------
add_library(glad STATIC External/glad/src/glad.c)
target_include_directories(glad PUBLIC External/glad/include)
set_target_properties(glad PROPERTIES FOLDER "External/glad")

# ---------------------------------------------------------------------------
# External: EFSW
# ---------------------------------------------------------------------------
add_subdirectory(External/efsw)
set_target_properties(efsw PROPERTIES FOLDER "External/efsw")
set_target_properties(efsw-static PROPERTIES FOLDER "External/efsw")

# ---------------------------------------------------------------------------
# External: EnTT (Header-only)
# ---------------------------------------------------------------------------
add_subdirectory(External/entt)
set_target_properties(EnTT PROPERTIES FOLDER "External/entt")

# ---------------------------------------------------------------------------
# External: ImGui
# ---------------------------------------------------------------------------
set(IMGUI_SOURCES
    External/imgui/imgui.cpp
    External/imgui/imgui_draw.cpp
    External/imgui/imgui_widgets.cpp
    External/imgui/imgui_tables.cpp
    External/imgui/imgui_demo.cpp
    External/imgui/backends/imgui_impl_glfw.cpp
    External/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
set_target_properties(imgui PROPERTIES FOLDER "External/imgui")
target_include_directories(imgui PUBLIC
    External/imgui/
    External/imgui/backends
    External/glfw/include
    External/glad/include
)

# ---------------------------------------------------------------------------
# External: ImGuizmo
# ---------------------------------------------------------------------------
add_library(imguizmo STATIC External/imguizmo/ImGuizmo.cpp)
target_include_directories(imguizmo PUBLIC
    External/imguizmo
    External/imgui
)
target_compile_definitions(imguizmo PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(imguizmo PRIVATE imgui)
set_target_properties(imguizmo PROPERTIES FOLDER "External/imguizmo")

# ---------------------------------------------------------------------------
# External: MiniAudio
# ---------------------------------------------------------------------------
add_subdirectory(External/miniaudio)
set_target_properties(miniaudio PROPERTIES FOLDER "External/miniaudio")
set_target_properties(miniaudio_channel_combiner_node PROPERTIES FOLDER "External/miniaudio")
set_target_properties(miniaudio_channel_separator_node PROPERTIES FOLDER "External/miniaudio")
set_target_properties(miniaudio_ltrim_node PROPERTIES FOLDER "External/miniaudio")
set_target_properties(miniaudio_reverb_node PROPERTIES FOLDER "External/miniaudio")
set_target_properties(miniaudio_vocoder_node PROPERTIES FOLDER "External/miniaudio")

# ---------------------------------------------------------------------------
# External: PhysX
# ---------------------------------------------------------------------------
set(PHYSX_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/External/physx")
set(PHYSX_LIB_DIR_BASE "${PROJECT_SOURCE_DIR}/External/bin/physx")

set(PHYSX_DEBUG_LIB_DIR "${PHYSX_LIB_DIR_BASE}/${PLATFORM}/debug")
set(PHYSX_RELEASE_LIB_DIR "${PHYSX_LIB_DIR_BASE}/${PLATFORM}/release")

# List of PhysX module libraries
set(PHYSX_MODULES
    PhysX
    PhysXCharacterKinematic
    PhysXCommon
    PhysXCooking
    PhysXExtensions
    PhysXFoundation
    PhysXPvdSDK
    PhysXVehicle2
)

# Construct PhysX lib full paths
set(PHYSX_DEBUG_LIBS "")
set(PHYSX_RELEASE_LIBS "")

foreach(mod IN LISTS PHYSX_MODULES)
    if(WIN32)
        list(APPEND PHYSX_DEBUG_LIBS "${PHYSX_DEBUG_LIB_DIR}/${mod}_static_64.lib")
        list(APPEND PHYSX_RELEASE_LIBS "${PHYSX_RELEASE_LIB_DIR}/${mod}_static_64.lib")
    else()
        list(APPEND PHYSX_DEBUG_LIBS "${PHYSX_DEBUG_LIB_DIR}/lib${mod}_static_64.a")
        list(APPEND PHYSX_RELEASE_LIBS "${PHYSX_RELEASE_LIB_DIR}/lib${mod}_static_64.a")
    endif()
endforeach()

# ---------------------------------------------------------------------------
# External: spdlog
# ---------------------------------------------------------------------------
set(SPDLOG_DIR External/spdlog)

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${SPDLOG_DIR}/include)
target_compile_features(spdlog INTERFACE cxx_std_20)

# ---------------------------------------------------------------------------
# External: glm
# ---------------------------------------------------------------------------
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)

add_subdirectory(External/glm)
set_target_properties(glm PROPERTIES FOLDER "External/glm")

# ---------------------------------------------------------------------------
# External: yaml-cpp
# ---------------------------------------------------------------------------
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(External/yaml-cpp)
set_target_properties(yaml-cpp PROPERTIES FOLDER "External/yaml-cpp")

# ---------------------------------------------------------------------------
# External: libclang
# ---------------------------------------------------------------------------
set(LIBCLANG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/External/clang-c")

if(WIN32)
    set(LIBCLANG_LIB_DIR_BASE "${PROJECT_SOURCE_DIR}/External/bin/clang-c")

    set(LIBCLANG_LIB_DIR "${LIBCLANG_LIB_DIR_BASE}/${PLATFORM}")
    message("-- libclang Binary Location: ${LIBCLANG_LIB_DIR}")
    set(LIBCLANG_LIB_PATH "${LIBCLANG_LIB_DIR}/libclang.lib")
else()
    set(LIBCLANG_LIB_PATH "${PROJECT_SOURCE_DIR}/Louron-Editor/libclang.so.18.1")
endif()

# ---------------------------------------------------------------------------
# External: tinyfiledialogs (Linux-only)
# ---------------------------------------------------------------------------
if(UNIX AND NOT APPLE) # Linux only
    add_library(tinyfiledialogs STATIC External/tinyfiledialogs/tinyfiledialogs.c)
    target_include_directories(tinyfiledialogs PUBLIC External/tinyfiledialogs)
    set_target_properties(tinyfiledialogs PROPERTIES FOLDER "External/tinyfiledialogs")
endif()

# Projects
add_subdirectory(Louron-Core)
add_subdirectory(Louron-Editor)