

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

include(${CMAKE_DIR}/common.cmake)

set (PROJNAME UnitTest_ActiveXPlugin)
Project (${PROJNAME})
message ("Generating project ${PROJECT_NAME} in ${CMAKE_CURRENT_BINARY_DIR}")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${ACTIVEXPLUGIN_SOURCE_DIR}
    ${SCRIPTINGCORE_SOURCE_DIR}
    ${WINDOWSCOMMON_SOURCE_DIR}
    ${GEN_DIR}/gen
    ${UNITTEST_FW_SOURCE_DIR}/src
    ${BOOST_SOURCE_DIR}
    )

add_definitions(
    /D "UNICODE"
    /D "_UNICODE"
    /D "_ATL_STATIC_REGISTRY"
)

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ./*.h
    ./*.cpp
    )

set (SOURCES
    ${GENERAL}
    )

add_executable(${PROJNAME} ${SOURCES})

add_dependencies(${PROJNAME}
    ${PLUGIN_PREFIX}_ActiveXPlugin
    ScriptingCore
    UnitTest++
    )

target_link_libraries (${PROJNAME}
    ${PLUGIN_PREFIX}_ActiveXPlugin
    ScriptingCore
    UnitTest++
    )

set_target_properties(${PROJNAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)

add_custom_command(TARGET ${PROJNAME} POST_BUILD
    COMMAND "${BIN_DIR}/${CMAKE_CFG_INTDIR}/${PROJNAME}")