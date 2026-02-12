cmake_minimum_required(VERSION 3.9)

project (solax)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -Wconversion -Wall -Wextra -Wpedantic -Wno-deprecated-copy -fno-omit-frame-pointer")

find_package(cpprestsdk REQUIRED)
find_package(CppLinuxSerial REQUIRED)
find_package(OpenSSL REQUIRED)

include(FetchContent)

FetchContent_Declare(
    Backward
    GIT_REPOSITORY https://github.com/bombela/backward-cpp
    GIT_TAG master
)
FetchContent_MakeAvailable(Backward)

FetchContent_Declare(
    libconfig
    GIT_REPOSITORY https://github.com/hyperrealm/libconfig
    GIT_TAG master
)
FetchContent_MakeAvailable(libconfig)

configure_file("${CMAKE_SOURCE_DIR}/solax.service.in" "${CMAKE_BINARY_DIR}/solax.service")
configure_file("${CMAKE_SOURCE_DIR}/solax.cfg" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/solax.cfg COPYONLY)

install(FILES "${CMAKE_BINARY_DIR}/solax.service" DESTINATION /lib/systemd/system)
install(FILES "solax.cfg" DESTINATION /opt/solax PERMISSIONS WORLD_READ)

include_directories("include")

add_subdirectory(src)

if(CMAKE_TESTING_ENABLED)
    add_subdirectory(test)
endif()
