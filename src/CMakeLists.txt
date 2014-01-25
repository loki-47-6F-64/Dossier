cmake_minimum_required (VERSION 2.6)

#################################################################
#include compilers
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(Dossier)
#################################################################
#include libraries

execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS)

#remove \n to prevent crash
STRING(REPLACE "\n" "" MYSQL_LIBS ${MYSQL_LIBS})

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MYSQL_LIBS} -lcrypto -lssl -lpthread -ldl")

#################################################################
#include compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#################################################################
#include source

set(DOSSIER_SOURCE_FILES
  main.cpp proc.cpp server.cpp err.cpp _ssl.cpp config.cpp proxy.cpp
  ssl_stream.cpp file_stream.cpp log.cpp
  sql_connect.cpp database.cpp
)


##################################################################
#include headers
execute_process(COMMAND mysql_config --include OUTPUT_VARIABLE MYSQL_INCLUDE)

#remove \n
STRING(REPLACE "\n" "" MYSQL_INCLUDE ${MYSQL_INCLUDE})

#Add include to compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_INCLUDE}")

set(INCLUDE_DIR ../include/)
set(DOSSIER_INCLUDED
  sql_connect.h proc.h main.h _ssl.h database.h 
  file.h stream.h proxy.h server.h log.h config.h err.h
)

include_directories("${INCLUDE_DIR}")

foreach(included ${DOSSIER_INCLUDED})
  set(DOSSIER_HEADER_FILES ${DOSSIER_HEADER_FILES} "${INCLUDE_DIR}${included}")
endforeach(included)

###################################################################

add_executable(dossier ${DOSSIER_SOURCE_FILES} ${DOSSIER_HEADER_FILES})


###################################################################
#Installation

install(TARGETS dossier DESTINATION "/usr/bin")
install(FILES dossier.service DESTINATION "/usr/lib/systemd/system")
install(FILES dossier.conf    DESTINATION "/etc/dossier")