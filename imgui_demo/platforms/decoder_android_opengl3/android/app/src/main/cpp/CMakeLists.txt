cmake_minimum_required(VERSION 3.6)

project(ImGuiDecoder)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../..)

add_library(${CMAKE_PROJECT_NAME} SHARED
  main.cpp
  ${PROJECT_ROOT}/third_party/imgui/imgui.cpp
  ${PROJECT_ROOT}/third_party/imgui/imgui_demo.cpp
  ${PROJECT_ROOT}/third_party/imgui/imgui_draw.cpp
  ${PROJECT_ROOT}/third_party/imgui/imgui_tables.cpp
  ${PROJECT_ROOT}/third_party/imgui/imgui_widgets.cpp
  ${PROJECT_ROOT}/third_party/imgui/backends/imgui_impl_android.cpp
  ${PROJECT_ROOT}/third_party/imgui/backends/imgui_impl_opengl3.cpp
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
  ${PROJECT_ROOT}/decoder/src/logger.cpp
  ${PROJECT_ROOT}/decoder/src/decoder.cpp
  ${PROJECT_ROOT}/decoder/src/decoder_android.cpp
  ${PROJECT_ROOT}/imgui_demo/ui/decoder_ui.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
  IMGUI_IMPL_OPENGL_ES3
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${PROJECT_ROOT}/third_party/imgui
  ${PROJECT_ROOT}/third_party/imgui/backends
  ${ANDROID_NDK}/sources/android/native_app_glue
  ${PROJECT_ROOT}/decoder/src
  ${PROJECT_ROOT}/imgui_demo/ui
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  android
  EGL
  GLESv3
  log
  mediandk
)
