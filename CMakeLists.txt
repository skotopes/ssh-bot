cmake_minimum_required(VERSION 2.4)
project(robot CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Cmake)

find_package(Botan)
IF (NOT BOTAN_FOUND)
    MESSAGE(FATAL_ERROR "Could not find botan library")
ENDIF(NOT BOTAN_FOUND)
INCLUDE_DIRECTORIES(${BOTAN_INCLUDE_DIR})

find_package(LibSSH)
IF (NOT LIBSSH_FOUND)
    MESSAGE(FATAL_ERROR "Could not find ssh library")
ENDIF(NOT LIBSSH_FOUND)
INCLUDE_DIRECTORIES(${LIBSSH_INCLUDE_DIRS})

set(robot_src
	src/r_robot.cpp
	src/r_core.cpp
	src/r_executor.cpp
	src/r_worker.cpp
	src/r_passwd.cpp
	src/r_parser.cpp
	src/r_parser_include.cpp
	src/r_parser_host.cpp
	src/r_parser_group.cpp
	src/r_parser_script.cpp
	src/r_crypt.cpp
        src/ssh.cpp
)

set(passfile_src
	src/r_passfile.cpp
	src/r_core.cpp
	src/r_passwd.cpp
	src/r_crypt.cpp
)

add_executable (robot ${robot_src})
add_executable (passfile ${passfile_src})

target_link_libraries(robot ${BOTAN_LIBRARIES} ${LIBSSH_LIBRARIES})
set_target_properties(robot PROPERTIES VERSION 0.2.7 SOVERSION 0)

target_link_libraries(passfile ${BOTAN_LIBRARIES})
set_target_properties(passfile PROPERTIES VERSION 0.1.1 SOVERSION 0)

########### install files ###############

install(TARGETS robot RUNTIME DESTINATION bin)
install(TARGETS passfile RUNTIME DESTINATION bin)
