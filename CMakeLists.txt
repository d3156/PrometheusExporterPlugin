cmake_minimum_required(VERSION 3.16)
project(PrometheusExporterPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_FILES
  CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(PrometheusExporterPlugin SHARED ${SRC_FILES})

target_include_directories(PrometheusExporterPlugin
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Put resulting .so into workspace/Plugins
set_target_properties(PrometheusExporterPlugin PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins"
  OUTPUT_NAME "PrometheusExporterPlugin"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel" "${CMAKE_CURRENT_BINARY_DIR}/_deps/MetricsModel")
target_link_libraries(PrometheusExporterPlugin PRIVATE MetricsModel)

set(Boost_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(BUILD_SHARED_LIBS OFF)

find_package(OpenSSL REQUIRED)
find_package(Boost 1.75 REQUIRED COMPONENTS system thread json)

target_include_directories(PrometheusExporterPlugin PRIVATE
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(PrometheusExporterPlugin PRIVATE
  Boost::system
  Boost::thread
  Boost::json
  OpenSSL::SSL
  OpenSSL::Crypto
)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")