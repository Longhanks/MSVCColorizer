cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(MSVCColorizer LANGUAGES CXX)

add_executable(MSVCColorizer
  ${CMAKE_CURRENT_LIST_DIR}/src/cmdline.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/consolecp.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/printlasterror.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/processline.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/syslibs.cpp
  ${CMAKE_CURRENT_LIST_DIR}/MSVCColorizer.manifest
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES
      FILES
        ${CMAKE_CURRENT_LIST_DIR}/src/cmdline.cppm
        ${CMAKE_CURRENT_LIST_DIR}/src/consolecp.cppm
        ${CMAKE_CURRENT_LIST_DIR}/src/printlasterror.cppm
        ${CMAKE_CURRENT_LIST_DIR}/src/processline.cppm
        ${CMAKE_CURRENT_LIST_DIR}/src/syslibs.cppm
  )
endif()

set_target_properties(MSVCColorizer
  PROPERTIES
    CXX_STANDARD              ${CMAKE_CXX_STANDARD_LATEST}
    CXX_EXTENSIONS                                      ON
    CXX_STANDARD_REQUIRED                               ON
    POSITION_INDEPENDENT_CODE                           ON
    EXPORT_COMPILE_COMMANDS                             ON
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_target_properties(MSVCColorizer
    PROPERTIES
      CXX_MODULE_STD ON
  )
endif()

target_compile_definitions(MSVCColorizer
  PUBLIC
    -D_WIN32_IE=0x0A00          # _WIN32_IE_WIN10
    -D_WIN32_WINNT=0x0A00       # _WIN32_WINNT_WIN10
    -DNTDDI_VERSION=0x0A000010  # NTDDI_WIN11_GE
    -DWINVER=0x0A00             # _WIN32_WINNT_WIN10
    -DWIN32_LEAN_AND_MEAN
    -DVC_EXTRALEAN
)

target_link_options(MSVCColorizer
  PRIVATE
    $<$<CONFIG:Debug>:/nodefaultlib:libucrtd.lib /defaultlib:ucrtd.lib>
    $<$<NOT:$<CONFIG:Debug>>:/nodefaultlib:libucrt.lib /defaultlib:ucrt.lib>
)

target_compile_options(MSVCColorizer
  PRIVATE
    /Zc:__cplusplus
    /Zc:enumTypes
    /Zc:externConstexpr
    /Zc:inline
    /Zc:preprocessor
    /Zc:templateScope
    /Zc:throwingNew
    /volatile:iso
    /utf-8
    /diagnostics:caret
    /U_ISO_VOLATILE
    /U_UTF8
    /Wall
    /wd4710 # function not inlined despite inline
    /wd4711 # function inlined despite no inline
    /wd5039 # poptentially throwing function passed to extern C function
)

target_link_options(MSVCColorizer
  PRIVATE
    /DEPENDENTLOADFLAG:0x800
)
