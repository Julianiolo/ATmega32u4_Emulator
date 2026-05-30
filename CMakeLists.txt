cmake_minimum_required(VERSION 3.15)
project(Atmega32u4_Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SourceFiles 
    "src/ATmega32u4.cpp"

    "src/components/CPU.cpp"
    "src/components/DataSpace.cpp"
    "src/components/Flash.cpp"
    "src/components/InstHandler.cpp"

    "src/extras/Analytics.cpp"
    "src/extras/Debugger.cpp"
    "src/extras/Disassembler.cpp"
)

add_library(${PROJECT_NAME} ${SourceFiles})
target_include_directories(${PROJECT_NAME} PUBLIC src)

add_subdirectory(dependencies/CPP_Utils)

target_link_libraries(${PROJECT_NAME} PUBLIC CPP_Utils)

# https://stackoverflow.com/a/60890947
# /Zc:__cplusplus is required to make __cplusplus accurate
# /Zc:__cplusplus is available starting with Visual Studio 2017 version 15.7
# (according to https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
# That version is equivalent to _MSC_VER==1914
# (according to https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019)
# CMake's ${MSVC_VERSION} is equivalent to _MSC_VER
# (according to https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html#variable:MSVC_VERSION)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()