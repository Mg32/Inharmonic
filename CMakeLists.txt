cmake_minimum_required(VERSION 3.14.0)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "./vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(Inharmonic
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0
    DESCRIPTION "Inharmonic VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(Inharmonic
    source/version.h
    source/cids.h
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/entry.cpp
)

# VSTGUI ------------
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(Inharmonic PRIVATE resource/editor.uidesc)
    target_link_libraries(Inharmonic PRIVATE vstgui_support)
    target_compile_definitions(Inharmonic PRIVATE VSTGUI_LIVE_EDITING=1)  # for Debugging
    smtg_target_add_plugin_resources(Inharmonic RESOURCES "resource/editor.uidesc")
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (Inharmonic
    RESOURCES
        resource/CE2677E662F35BE2AC186EEC987CC171_snapshot.png
        resource/CE2677E662F35BE2AC186EEC987CC171_snapshot_2.0x.png
)

target_link_libraries(Inharmonic
    PRIVATE
        sdk
)

smtg_target_configure_version_file(Inharmonic)

if(SMTG_MAC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -Wno-pragma-pack")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -Wno-pragma-pack")
    smtg_target_set_bundle(Inharmonic
        BUNDLE_IDENTIFIER net.mogesystem.inharmonic
        COMPANY_NAME "mogesystem"
    )
    smtg_target_set_debug_executable(Inharmonic
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(Inharmonic PRIVATE
        resource/win32resource.rc
    )
    if(MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Os /fp:fast /w04103 -Wno-pragma-pack")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Os /fp:fast /w04103 -Wno-pragma-pack")
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Inharmonic)
        smtg_target_set_debug_executable(Inharmonic
            "$(ProgramFiles)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
