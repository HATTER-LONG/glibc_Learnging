message(STATUS "Config unittest current dir is ${CMAKE_CURRENT_SOURCE_DIR}")

#find_package(GTest QUIET CONFIG)
find_package(Catch2 QUIET CONFIG)
if(Catch2_FOUND)
    list(APPEND LOCAL_LINK_LIB Catch2::Catch2)
else(Catch2_FOUND)
    include_directories(${CMAKE_SOURCE_DIR}/Lib/catch2)
endif(Catch2_FOUND)

FILE(GLOB AUTESTSRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(PROJECTNAME automatedtools_unittests)

add_executable(${PROJECTNAME} ${AUTESTSRC})
target_link_libraries(${PROJECTNAME} PRIVATE ${LOCAL_LINK_LIB})
