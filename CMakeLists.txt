cmake_minimum_required (VERSION 2.6)

project (bankingsystem)

ADD_LIBRARY(main 
    src/admin.cpp 
    src/employee.cpp 
    src/customer.cpp 
    src/database.cpp 
    src/session.cpp 
    src/userinterface.cpp 
    src/main.cpp
)

target_link_libraries(main -lcrypto) 
target_link_libraries(main -lsmartcols)
target_link_libraries(main -lsqlite3)

ADD_EXECUTABLE(bankingsystem src/main.cpp)
target_link_libraries(bankingsystem main)

