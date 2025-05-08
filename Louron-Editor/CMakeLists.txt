file(GLOB_RECURSE APP_SRC
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

add_executable(LouronEditor ${APP_SRC})

set(CMAKE_CXX_STANDARD 20)

target_compile_definitions(LouronEditor PUBLIC PX_PHYSX_STATIC_LIB)

if(UNIX)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_DEBUG)
    else()
        add_compile_definitions(NDEBUG)
    endif()
endif()

if(MSVC)
    target_compile_options(LouronEditor PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(LouronEditor PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

set_target_properties(LouronEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    FOLDER "Louron-Editor"
)

target_include_directories(LouronEditor PRIVATE "${PROJECT_SOURCE_DIR}/Louron-Core/Source")
target_link_libraries(LouronEditor PRIVATE LouronCore)

# ---------------------------------------------------------------------------
# Always copy Native Script Core source folder before building LouronEditor
# ---------------------------------------------------------------------------

set(SCRIPT_CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../Louron-Native-Script-Core/Source")
set(SCRIPT_CORE_DST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Script Core API")

add_custom_target(copy_script_core ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SCRIPT_CORE_SRC}" "${SCRIPT_CORE_DST}"
    COMMENT "Copying Native Script Core into Resources/Script Core API..."
)

add_dependencies(LouronEditor copy_script_core)