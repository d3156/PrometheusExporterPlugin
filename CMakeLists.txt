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

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel/CMakeLists.txt")
    message(STATUS "MetricsModel not found, downloading...")
    
    include(FetchContent)
    FetchContent_Declare(
        MetricsModel
        GIT_REPOSITORY https://github.com/d3156/MetricsModel.git
        GIT_TAG master  # или конкретный тег/коммит
    )
    FetchContent_MakeAvailable(MetricsModel)
else()
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../MetricsModel" 
                     "${CMAKE_BINARY_DIR}/_deps/MetricsModel")
endif()

target_link_libraries(PrometheusExporterPlugin PRIVATE MetricsModel)

find_package(OpenSSL REQUIRED)
find_package(Boost 1.75 REQUIRED COMPONENTS system thread json)

set(OPENSSL_USE_STATIC_LIBS TRUE)

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
