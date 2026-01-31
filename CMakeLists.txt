cmake_minimum_required(VERSION 3.16)
project(PrometheusExporterPlugin VERSION 1.0.2 LANGUAGES CXX)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../tools/workspace.cmake")
create_target(plugin)

dependency("../EasyHttpLib" EasyHttpLib)
dependency("../MetricsModel" MetricsModel)

find_package(Boost 1.8 REQUIRED COMPONENTS json thread system)
target_link_libraries(PrometheusExporterPlugin PRIVATE Boost::system Boost::thread Boost::json)

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
target_link_libraries(PrometheusExporterPlugin PUBLIC OpenSSL::SSL OpenSSL::Crypto)