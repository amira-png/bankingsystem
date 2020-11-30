cmake_minimum_required (VERSION 2.6)

project (bankingsystem)

ADD_LIBRARY(prog 
    src/admin.cpp 
    src/employee.cpp 
    src/customer.cpp 
    src/database.cpp 
    src/session.cpp 
    src/userinterface.cpp 
)

target_link_libraries(prog -lcrypto) 
target_link_libraries(prog -lsmartcols)
target_link_libraries(prog -lsqlite3)
add_executable(bankingsystem src/main.cpp)
target_link_libraries(bankingsystem prog)


project(sessiontest)

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(sessiontest src/tests/session_test.cpp )
target_link_libraries(sessiontest prog gtest gtest_main -lsqlite3 -lsmartcols
	-lcrypto -lpthread)
add_test( sessiontest sessiontest )
