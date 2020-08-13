cmake_minimum_required(VERSION 3.14)
project(OWEN_ImpulseCounter30)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

if ("${LIBRARY_INSTALL_DESTINATION}" STREQUAL "")
   set(LIBRARY_INSTALL_DESTINATION ${CMAKE_SOURCE_DIR}/host)
endif()

find_package(Boost REQUIRED)

add_definitions(-DDEBUG_INFO=1)

include_directories(
   include
   ${Boost_INCLUDE_DIRS})

set(${PROJECT_NAME}_PUBLIC_INCLUDES include/OWEN/ImpulseCounter30.hpp)

add_library(${PROJECT_NAME}
        include/OWEN/ImpulseCounter30.hpp
        src/ImpulseCounter30.cpp
        src/SerialPort.hpp
        src/SerialPort.cpp
        src/ModBus.hpp
        src/ModBus.cpp
        src/crc16.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_INCLUDES}")

install(TARGETS ${PROJECT_NAME}
   ARCHIVE DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   LIBRARY DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   RUNTIME DESTINATION ${LIBRARY_INSTALL_DESTINATION}/bin
   PUBLIC_HEADER DESTINATION ${LIBRARY_INSTALL_DESTINATION}/include/${PROJECT_NAME})

add_subdirectory(test)