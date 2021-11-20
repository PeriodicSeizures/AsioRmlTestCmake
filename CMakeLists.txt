cmake_minimum_required(VERSION 3.14)
project(RMLCMake)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(get_cpm)

if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()

option(USE_STANDALONE_ASIO "set ON to use standalone Asio instead of Boost.Asio" ON)

############
# LIBRARIES
############




# ASIO
find_package(Threads REQUIRED)

CPMAddPackage("gh:chriskohlhoff/asio#asio-1-18-1@1.18.1")

# ASIO doesn't use CMake, we have to configure it manually. Extra notes for using on Windows:
#
# 1) If _WIN32_WINNT is not set, ASIO assumes _WIN32_WINNT=0x0501, i.e. Windows XP target, which is
# definitely not the platform which most users target.
#
# 2) WIN32_LEAN_AND_MEAN is defined to make Winsock2 work.
if(asio_ADDED)
  add_library(asio INTERFACE)

  target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)

  target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)

  target_link_libraries(asio INTERFACE Threads::Threads)

  if(WIN32)
    # macro see @ https://stackoverflow.com/a/40217291/1746503
    macro(get_win32_winnt version)
      if(CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
        string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if("${verMajor}" MATCHES "10")
          set(verMajor "A")
          string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
        endif("${verMajor}" MATCHES "10")
        # Remove all remaining '.' characters.
        string(REPLACE "." "" ver ${ver})
        # Prepend each digit with a zero.
        string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
        set(${version} "0x${ver}")
      endif()
    endmacro()

    if(NOT DEFINED _WIN32_WINNT)
      get_win32_winnt(ver)
      set(_WIN32_WINNT ${ver})
    endif()

    message(STATUS "Set _WIN32_WINNT=${_WIN32_WINNT}")

    target_compile_definitions(asio INTERFACE _WIN32_WINNT=${_WIN32_WINNT} WIN32_LEAN_AND_MEAN)
  endif()
endif()

# ---- Executable ----


find_package(OpenGL REQUIRED)


CPMAddPackage("gh:g-truc/glm@0.9.9.8#0.9.9.8")


#CPMAddPackage("gh:bombomby/optick@1.3.1.0#1.3.1.0")


CPMAddPackage(
    GITHUB_REPOSITORY Perlmint/glew-cmake
    VERSION 2.2.0
    GIT_TAG glew-cmake-2.2.0
    OPTIONS
    "glew-cmake_BUILD_SHARED OFF"
)


CPMAddPackage(
    NAME json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.9.1
    OPTIONS
    "JSON_BuildTests OFF"
)


CPMAddPackage(
    NAME SDL2
    GITHUB_REPOSITORY libsdl-org/SDL
    VERSION 2.0.14
    GIT_TAG release-2.0.14
    OPTIONS
    "SDL_SHARED OFF"
    "SDL_STATIC ON"
    "LIBC ON"
)


CPMAddPackage(
    NAME easyloggingpp
    GITHUB_REPOSITORY amrayn/easyloggingpp
    VERSION 9.97.0
    OPTIONS "build_static_lib ON"
)
target_include_directories(easyloggingpp PUBLIC ${easyloggingpp_SOURCE_DIR}/src)
target_compile_definitions(easyloggingpp PUBLIC ELPP_THREAD_SAFE ELPP_FEATURE_PERFORMANCE_TRACKING)
if (LINUX)
    target_compile_definitions(easyloggingpp PUBLIC ELPP_FEATURE_CRASH_LOG ELPP_STL_LOGGING)
endif ()


CPMAddPackage(
    NAME stb
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG ea2f937a01ce39795ab02b6c6e30173b4f1ed46c
    DOWNLOAD_ONLY TRUE
)
if(stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif()


CPMAddPackage(
    NAME zlib
    GITHUB_REPOSITORY madler/zlib
    VERSION 1.2.11
)
target_include_directories(zlibstatic PUBLIC ${zlib_SOURCE_DIR})
target_include_directories(zlibstatic PUBLIC ${zlib_BINARY_DIR})


CPMAddPackage(
    NAME Lua
    GITHUB_REPOSITORY lua/lua
    VERSION 5.4.3
    DOWNLOAD_ONLY YES
)
if(Lua_ADDED)
    file(GLOB Lua_sources ${Lua_SOURCE_DIR}/*.c)
    list(REMOVE_ITEM Lua_sources "${Lua_SOURCE_DIR}/lua.c" "${Lua_SOURCE_DIR}/luac.c" "${Lua_SOURCE_DIR}/onelua.c")
    add_library(lua STATIC ${Lua_sources})
    target_include_directories(lua SYSTEM PUBLIC $<BUILD_INTERFACE:${Lua_SOURCE_DIR}>)
    install(TARGETS lua
		EXPORT lua-targets
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
    install(EXPORT lua-targets DESTINATION ${LIB_INSTALL_DIR}/lua_ac/cmake)
    export(EXPORT lua-targets)
endif()


CPMAddPackage(
    NAME sol2
    GITHUB_REPOSITORY ThePhD/sol2
    VERSION 3.2.2
    DOWNLOAD_ONLY YES
)
if(sol2_ADDED)
    add_library(sol2 INTERFACE IMPORTED)
    target_include_directories(sol2 SYSTEM INTERFACE ${sol2_SOURCE_DIR}/include)
    target_link_libraries(sol2 INTERFACE lua)
endif()


CPMAddPackage(
    NAME Freetype
    GITHUB_REPOSITORY freetype/freetype
    VERSION 2.10.4
    GIT_TAG VER-2-10-4
    OPTIONS
    "CMAKE_DISABLE_FIND_PACKAGE_ZLIB TRUE"
    "CMAKE_DISABLE_FIND_PACKAGE_BZip2 TRUE"
    "CMAKE_DISABLE_FIND_PACKAGE_PNG TRUE"
    "CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE"
    "CMAKE_DISABLE_FIND_PACKAGE_BrotliDec TRUE"
)
export(EXPORT freetype-targets)

CPMAddPackage(
    NAME RmlUi
    GITHUB_REPOSITORY mikke89/RmlUi
    VERSION 4.1
    GIT_TAG 4.1
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "BUILD_TESTING OFF"
    "BUILD_SAMPLES OFF"
    "BUILD_LUA_BINDINGS ON"
)
target_link_libraries(RmlCore freetype lua)



###########
# RMLCMake
###########

file(GLOB ac_manifest "./src/*.manifest")
file(GLOB ac_sources "./src/*.cpp")

add_executable(RMLCMake ${ac_sources} ${ac_manifest})

target_compile_features(RMLCMake PRIVATE cxx_std_20)

target_link_libraries(RMLCMake  # PROJECT
    Threads::Threads
    asio
    OpenGL::GL
    glm
    stb
    easyloggingpp
    libglew_static
    nlohmann_json::nlohmann_json
    SDL2-static
    SDL2main
    zlibstatic
    sol2
    RmlLua
    RmlDebugger
)

#set_target_properties(RMLCMake OptickCore PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#)

set_target_properties(RMLCMake PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/cwd/
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RMLCMake)

if (MSVC)
    target_compile_options(RMLCMake PRIVATE /bigobj)
endif()
if(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)
    set_target_properties(RMLCMake PROPERTIES WIN32_EXECUTABLE ON)
endif()

# this doesnt work
#set_target_properties(RMLCMake PROPERTIES 
#    VS_DEBUGGER_COMMAND_ARGUMENTS "C:/Users/Rico/Documents/VisualStudio2019/Projects/RMLCMake/cwd/")

file(DOWNLOAD https://github.com/google/fonts/raw/main/apache/opensans/OpenSans%5Bwdth%2Cwght%5D.ttf ${CMAKE_SOURCE_DIR}/assets/fonts/OpenSans-Regular.ttf)
