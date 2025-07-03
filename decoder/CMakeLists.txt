cmake_minimum_required( VERSION 3.6 )

project( DecoderDemo )

add_library(
        ${PROJECT_NAME} SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/src/decoder.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/decoder_windows.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
)

target_compile_definitions(
        ${PROJECT_NAME} PRIVATE
        NO_CRYPTO
)

target_include_directories(
        ${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/api
)

if(WIN32)
    target_sources(
        ${PROJECT_NAME} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/api/decoder_export_windows.cpp
    )

    target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        d3d11
        mfplat
        mfreadwrite
        mfuuid
        wmcodecdspuuid
    )
endif()