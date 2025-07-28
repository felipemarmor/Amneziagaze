cmake_minimum_required(VERSION 3.14)
project(AMNEZIAGAZE VERSION 0.2.7)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define VST3 SDK path
set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/vst3sdk")

# Include our custom VST3 module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(VST3)

# Add the VST3 SDK as a subdirectory
find_vst3_sdk(${VST3_SDK_ROOT})

# Define plugin information
set(PLUGIN_SOURCES
    src/vst/pluginentry.cpp
    src/vst/pluginprocessor.cpp
    src/vst/plugincontroller.cpp
    src/vst/plugineditor.cpp
)

# Add the VST3 plugin
add_vst3_plugin(AMNEZIAGAZE
    VST3_SDK_ROOT ${VST3_SDK_ROOT}
    SOURCES ${PLUGIN_SOURCES}
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDENCIES
        base
        pluginterfaces
        sdk
)

# Define preprocessor macros
target_compile_definitions(AMNEZIAGAZE
    PRIVATE
        DEVELOPMENT=1
        SMTG_RENAME_ASSERT=1
        _CRT_SECURE_NO_WARNINGS=1
)