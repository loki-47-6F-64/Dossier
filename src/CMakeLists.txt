cmake_minimum_required (VERSION 2.6)

#################################################################
#define macros
function(sub_dir IN DIR FILES)
  foreach(_file ${FILES})
    set(result ${result} "${DIR}${_file}")
  endforeach(_file)
  set(${IN} ${result} PARENT_SCOPE)
  message("${result}")
endfunction(sub_dir)


#################################################################
#include compilers
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(Dossier)
#################################################################
#include libraries

#execute_process(COMMAND mysql_config --libs OUTPUT_VARIABLE MYSQL_LIBS)

#remove \n to prevent crash
#STRING(REPLACE "\n" "" MYSQL_LIBS ${MYSQL_LIBS})

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmysqlclient -lz -lm -lcrypto -lssl -lpthread -ldl -lgtest")

#################################################################
#include compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#################################################################
#include source

set(DB_SOURCE_FILES database.cpp sql_connect.cpp)
set(DB_SOURCE_DIR database/)
set(FILE_SOURCE_FILES file_stream.cpp log.cpp _ssl.cpp ssl_stream.cpp)
set(FILE_SOURCE_DIR file/)
set(SERVER_SOURCE_FILES config.cpp proxy.cpp server.cpp)
set(SERVER_SOURCE_DIR server/)

set(CLIENT_DOC_SOURCE_FILES download.cpp remove.cpp search.cpp upload.cpp)
set(CLIENT_COMPANY_SOURCE_FILES add.cpp del.cpp list.cpp)

sub_dir(CLIENT_DOC_SOURCE client/doc/ "${CLIENT_DOC_SOURCE_FILES}")
sub_dir(CLIENT_COMPANY_SOURCE client/company/ "${CLIENT_COMPANY_SOURCE_FILES}")
sub_dir(DB_SOURCE ${DB_SOURCE_DIR} "${DB_SOURCE_FILES}")
sub_dir(SERVER_SOURCE ${SERVER_SOURCE_DIR} "${SERVER_SOURCE_FILES}") 
sub_dir(FILE_SOURCE ${FILE_SOURCE_DIR} "${FILE_SOURCE_FILES}")

set(CLIENT_SOURCE client/client.cpp ${CLIENT_DOC_SOURCE} ${CLIENT_COMPANY_SOURCE})

set(DOSSIER_SOURCE_FILES
  ${DB_SOURCE}
  ${FILE_SOURCE}
  ${SERVER_SOURCE}
  ${CLIENT_SOURCE}
  err.cpp proc.cpp
)

##################################################################
#include headers
#execute_process(COMMAND mysql_config --include OUTPUT_VARIABLE MYSQL_INCLUDE)

#remove \n
#STRING(REPLACE "\n" "" MYSQL_INCLUDE ${MYSQL_INCLUDE})

#Add include to compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_FIND_ROOT_PATH}/usr/include/mysql")

set(DB_HEADER_FILES database.h sql_connect.h)
set(DB_HEADER_DIR database/)
set(SERVER_HEADER_FILES config.h main.h proxy.h server.h)
set(SERVER_HEADER_DIR server/)
set(CLIENT_HEADER_DIR client/)
set(CLIENT_HEADER_FILES args.h main.h)
set(FILE_HEADER_FILES file.h log.h _ssl.h stream.h)
set(FILE_HEADER_DIR file/)

sub_dir(DB_HEADER ${DB_HEADER_DIR} ${DB_HEADER_FILES})
sub_dir(CLIENT_HEADER ${CLIENT_HEADER_DIR} ${CLIENT_HEADER_FILES})
sub_dir(FILE_HEADER ${FILE_HEADER_DIR} ${FILE_HEADER_FILES})
sub_dir(SERVER_HEADER ${SERVER_HEADER_DIR} ${SERVER_HEADER_FILES})

set(INCLUDE_DIR ../include/)
set(DOSSIER_INCLUDED
  ${DB_HEADER}
  ${FILE_HEADER}
  ${SERVER_HEADER}
  err.h proc.h thread_pool.h thread_t.h
)

include_directories("${INCLUDE_DIR}")

foreach(included ${DOSSIER_INCLUDED})
  set(DOSSIER_HEADER_FILES ${DOSSIER_HEADER_FILES} "${INCLUDE_DIR}${included}")
endforeach(included)

###################################################################

add_library(dossier_lib ${DOSSIER_SOURCE_FILES} ${DOSSIER_HEADER_FILES})
add_executable(dossier server/main.cpp ${DOSSIER_HEADER_FILES})
add_executable(dossier_client client/main.cpp ${DOSSIER_HEADER_FILES})
add_executable(test.out test/test.cpp ${DOSSIER_HEADER_FILES})

target_link_libraries(dossier dossier_lib)
target_link_libraries(dossier_client dossier_lib)
target_link_libraries(test.out dossier_lib)

###################################################################
#Testing
enable_testing()
add_test(Server test.out)


###################################################################
#Installation

install(TARGETS dossier dossier_client DESTINATION "/usr/bin")
install(FILES dossier.service DESTINATION "/usr/lib/systemd/system")
install(FILES dossier.conf    DESTINATION "/etc/dossier")
