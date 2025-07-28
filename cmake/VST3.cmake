# VST3 SDK integration helper module

# Function to add a VST3 plugin target
function(add_vst3_plugin TARGET_NAME)
    # Parse arguments
    set(options "")
    set(oneValueArgs VST3_SDK_ROOT)
    set(multiValueArgs SOURCES INCLUDE_DIRS DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Check if VST3 SDK path is provided
    if(NOT ARG_VST3_SDK_ROOT)
        message(FATAL_ERROR "VST3_SDK_ROOT is required for add_vst3_plugin")
    endif()

    # Add the plugin as a module library
    add_library(${TARGET_NAME} MODULE ${ARG_SOURCES})

    # Include directories
    target_include_directories(${TARGET_NAME} 
        PRIVATE 
            ${ARG_INCLUDE_DIRS}
            ${ARG_VST3_SDK_ROOT}
    )

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    # Platform-specific settings
    if(APPLE)
        set_target_properties(${TARGET_NAME} PROPERTIES
            BUNDLE TRUE
            BUNDLE_EXTENSION "vst3"
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES"
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PkgInfo.plist"
        )
    elseif(WIN32)
        set_target_properties(${TARGET_NAME} PROPERTIES
            SUFFIX ".vst3"
        )
    endif()

    # Install targets
    install(TARGETS ${TARGET_NAME}
        DESTINATION "VST3"
    )
endfunction()

# Function to find and setup VST3 SDK
function(find_vst3_sdk VST3_SDK_ROOT)
    # Check if the SDK exists
    if(NOT EXISTS "${VST3_SDK_ROOT}")
        message(FATAL_ERROR "VST3 SDK not found at ${VST3_SDK_ROOT}")
    endif()

    # Check for key SDK files
    if(NOT EXISTS "${VST3_SDK_ROOT}/pluginterfaces/vst/ivstaudioprocessor.h")
        message(FATAL_ERROR "VST3 SDK at ${VST3_SDK_ROOT} appears to be incomplete")
    endif()

    # Add SDK subdirectory if it exists
    if(EXISTS "${VST3_SDK_ROOT}/CMakeLists.txt")
        add_subdirectory(${VST3_SDK_ROOT} vst3sdk)
    else()
        message(WARNING "VST3 SDK CMakeLists.txt not found. SDK will not be built.")
    endif()
endfunction()