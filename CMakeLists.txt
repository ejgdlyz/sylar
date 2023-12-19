cmake_minimum_required(VERSION 3.0)
project(sylar)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include)  # yaml-cpp
link_directories(/usr/local/lib)  # 类库

set(LIB_SRC
    sylar/config.cc
    sylar/fd_manager.cc
    sylar/fiber.cc
    sylar/hook.cc
    sylar/iomanager.cc
    sylar/log.cc
    sylar/scheduler.cc
    sylar/thread.cc
    sylar/timer.cc
    sylar/util.cc
    sylar/address.cc
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)  # redefined __FILE__ as relative path for syalr/

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)

set(LIBS
        sylar
        dl
        pthread
        yaml-cpp)

message("***", ${LIBS})

add_executable(test tests/test.cc)
add_dependencies(test sylar)
force_redefine_file_macro_for_sources(test)  # redefined __FILE__ as relative path for test
target_link_libraries(test ${LIBS})


add_executable(test_config tests/test_config.cc)
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config)  # redefined __FILE__ as relative path for test_config
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/test_thread.cc)
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread)  
target_link_libraries(test_thread ${LIBS})

add_executable(test_utils tests/test_utils.cc)
add_dependencies(test_utils sylar)
force_redefine_file_macro_for_sources(test_utils)  
target_link_libraries(test_utils ${LIBS})

add_executable(test_fiber tests/test_fiber.cc)
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber)  
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cc)
add_dependencies(test_scheduler sylar)
force_redefine_file_macro_for_sources(test_scheduler)  
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cc)
add_dependencies(test_iomanager sylar)
force_redefine_file_macro_for_sources(test_iomanager)  
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_iomanager_serv tests/test_iomanager_serv.cc)
add_dependencies(test_iomanager_serv sylar)
force_redefine_file_macro_for_sources(test_iomanager_serv)  
target_link_libraries(test_iomanager_serv ${LIBS})

add_executable(test_hook tests/test_hook.cc)
add_dependencies(test_hook sylar)
force_redefine_file_macro_for_sources(test_hook)  
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/test_address.cc)
add_dependencies(test_address sylar)
force_redefine_file_macro_for_sources(test_address)  
target_link_libraries(test_address ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)