include_directories(include)

add_definitions(-DDEBUG_INFO=0)

add_executable(test_${PROJECT_NAME}
        OWEN_ImpulseCounter30.cpp)

target_link_libraries(test_${PROJECT_NAME}
   ${PROJECT_NAME}
   ${Boost_LIBRARIES})

install(TARGETS test_${PROJECT_NAME}
   RUNTIME DESTINATION ${LIBRARY_INSTALL_DESTINATION}/bin)