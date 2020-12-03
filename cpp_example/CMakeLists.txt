cmake_minimum_required(VERSION 3.10)

# set the project name
project("paho-mqtt-cpp" 
    LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add dynamic libraries for paho lib
file(GLOB LIBRARIES "/usr/local/lib/libpaho*.dylib")
message("LIBRARIES = ${LIBRARIES}")
link_directories(/usr/local/lib)

# add include directories 
include_directories(/usr/local/include)

# add the executable
add_executable(mqtt_example_client src/mqtt_example_client.cpp)
target_link_libraries(mqtt_example_client ${LIBRARIES})