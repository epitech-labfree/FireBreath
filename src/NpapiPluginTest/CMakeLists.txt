

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

include(${CMAKE_DIR}/common.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/../PluginConfig.cmake)

set (PROJNAME ${PLUGIN_PREFIX}_UnitTest_NpapiPlugin)
Project (${PROJNAME})
message ("Generating project ${PROJECT_NAME} in ${CMAKE_CURRENT_BINARY_DIR}")

include_directories(
    ${GECKOSDK_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${NPAPIPLUGIN_SOURCE_DIR}
    ${NPAPIHOST_SOURCE_DIR}
    ${SCRIPTINGCORE_SOURCE_DIR}
    ${GEN_DIR}/gen
    ${BOOST_SOURCE_DIR}
    ${UNITTEST_FW_SOURCE_DIR}/src
    )

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ./*.h
    ./*.cpp
    )

set (SOURCES
    ${GENERAL}
    )

add_library(${PROJNAME} STATIC ${SOURCES})

add_dependencies(${PROJNAME}
    ${PLUGIN_PREFIX}_NpapiPlugin
    NPAPIHost
    UnitTest++
    )

target_link_libraries (${PROJNAME}
    ${PLUGIN_PREFIX}_NpapiPlugin
    NPAPIHost
    UnitTest++

add_custom_command(TARGET ${PROJNAME} POST_BUILD
    COMMAND "${BIN_DIR}/${CMAKE_CFG_INTDIR}/${PROJNAME}")
    )