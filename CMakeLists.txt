cmake_minimum_required(VERSION 2.8)

project(mymetrics)

execute_process(COMMAND mysql_config --cxxflags
                OUTPUT_VARIABLE mysql_flags OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --plugindir
                OUTPUT_VARIABLE mysql_plugin_dir OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "-std=c++0x ${mysql_flags}")
set(CMAKE_BUILD_TYPE Release)

aux_source_directory(src/ src_files)

add_library(mymetrics SHARED ${src_files})

install(TARGETS mymetrics DESTINATION ${mysql_plugin_dir})
