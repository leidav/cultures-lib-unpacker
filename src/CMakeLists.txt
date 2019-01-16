add_executable(unpack main.c)
#target_sources(executable_name PRIVATE 
#               "${CMAKE_CURRENT_SOURCE_DIR}/file.cpp" 
#               "${CMAKE_CURRENT_SOURCE_DIR}/file.h")
#target_link_libraries(unpack PRIVATE library)
target_compile_features(unpack PRIVATE cxx_std_17)
