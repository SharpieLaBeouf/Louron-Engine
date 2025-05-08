file(GLOB_RECURSE ENGINE_SRC
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

add_library(LouronCore STATIC ${ENGINE_SRC})

set(CMAKE_CXX_STANDARD 20)

if(ENABLE_IPO)
    set_property(TARGET LouronCore PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_compile_definitions(LouronCore PUBLIC GLFW_INCLUDE_NONE)
target_compile_definitions(LouronCore PUBLIC PX_PHYSX_STATIC_LIB)
target_compile_definitions(LouronCore PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(LouronCore PUBLIC YAML_CPP_STATIC_DEFINE)

if(MSVC)
    target_compile_options(LouronCore PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(LouronCore PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

target_include_directories(LouronCore
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${PROJECT_SOURCE_DIR}/Louron Core/Source"
        ${PROJECT_SOURCE_DIR}/External/glfw/include
        ${PROJECT_SOURCE_DIR}/External/glad/include
        ${PROJECT_SOURCE_DIR}/External/assimp/include
        ${PROJECT_SOURCE_DIR}/External/efsw/include
        ${PROJECT_SOURCE_DIR}/External/imgui
        ${PROJECT_SOURCE_DIR}/External/imguizmo
        ${PROJECT_SOURCE_DIR}/External/miniaudio
        ${PROJECT_SOURCE_DIR}/External/spdlog/include
        ${PROJECT_SOURCE_DIR}/External/physx/physx/include
        ${PROJECT_SOURCE_DIR}/External/glm/glm
        ${PROJECT_SOURCE_DIR}/External/stb
        ${PROJECT_SOURCE_DIR}/External/yaml-cpp/include
        ${PROJECT_SOURCE_DIR}/External/tinyfiledialogs
        ${LIBCLANG_INCLUDE_DIR}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(LouronCore PUBLIC tinyfiledialogs)
endif()

target_link_libraries(LouronCore
    PUBLIC
        EnTT
        glfw
        glad
        assimp
        efsw
        imgui
        imguizmo
        miniaudio
        spdlog
        $<$<CONFIG:Debug>:${PHYSX_DEBUG_LIBS}>
        $<$<CONFIG:Release>:${PHYSX_RELEASE_LIBS}>
        ${LIBCLANG_LIB_PATH}
        glm
        yaml-cpp
)

if(UNIX)

    target_link_libraries(LouronCore 
        PUBLIC
            pthread
            dl
            m
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    
        message("Debug Configuration")
        add_compile_definitions(_DEBUG)
        target_link_libraries(LouronCore 
            PUBLIC
                ${PHYSX_DEBUG_LIBS}
        )

    else()

        message("Release Configuration")
        add_compile_definitions(NDEBUG)
        target_link_libraries(LouronCore 
            PUBLIC
                ${PHYSX_RELEASE_LIBS}
        )

    endif()

endif()

set_target_properties(LouronCore PROPERTIES FOLDER "Louron-Core")