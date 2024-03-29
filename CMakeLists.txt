cmake_minimum_required(VERSION 3.0)
project(sylar)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)
include_directories(/usr/local/include)  # yaml-cpp
link_directories(/usr/local/lib)  # 类库

option(TEST "ON for complile test" OFF)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIB_SRC
    sylar/address.cc
    sylar/bytearray.cc
    sylar/config.cc
    sylar/daemon.cc
    sylar/env.cc
    sylar/fd_manager.cc
    sylar/fiber.cc
    sylar/http/http.cc
    sylar/http/http_parser.cc
    sylar/http/http_session.cc
    sylar/http/http_connection.cc
    sylar/http/http_server.cc
    sylar/http/servlet.cc
    sylar/http/ws_session.cc
    sylar/http/ws_connection.cc
    sylar/http/ws_server.cc
    sylar/http/ws_servlet.cc
    sylar/hook.cc
    sylar/iomanager.cc
    sylar/library.cc
    sylar/log.cc
    sylar/module.cc
    sylar/mutex.cc
    sylar/scheduler.cc
    sylar/socket.cc
    sylar/stream.cc
    sylar/socket_stream.cc
    sylar/tcp_server.cc
    sylar/thread.cc
    sylar/timer.cc
    sylar/util.cc
    sylar/util/json_util.cc
    sylar/util/hash_util.cc
    sylar/worker.cc
    sylar/application.cc
    )

ragelmaker(sylar/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(sylar/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sylar)

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)  # redefined __FILE__ as relative path for syalr/

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
find_library(JSONCPP libjsoncpp)

set(LIBS
        sylar
        dl
        pthread
        yaml-cpp
        jsoncpp
        ${OPENSSL_LIBRARIES}
        )

# message("***", ${LIBS})

if(TEST)
sylar_add_executable(test_logger "tests/test_logger.cc" sylar "${LIBS}")
sylar_add_executable(test_config "tests/test_config.cc" sylar "${LIBS}")
sylar_add_executable(test_thread "tests/test_thread.cc" sylar "${LIBS}")
sylar_add_executable(test_utils "tests/test_utils.cc" sylar "${LIBS}")
sylar_add_executable(test_fiber "tests/test_fiber.cc" sylar "${LIBS}")
sylar_add_executable(test_scheduler "tests/test_scheduler.cc" sylar "${LIBS}")
sylar_add_executable(test_iomanager "tests/test_iomanager.cc" sylar "${LIBS}")
sylar_add_executable(test_iomanager_serv "tests/test_iomanager_serv.cc" sylar "${LIBS}")
sylar_add_executable(test_hook "tests/test_hook.cc" sylar "${LIBS}")
sylar_add_executable(test_address "tests/test_address.cc" sylar "${LIBS}")
sylar_add_executable(test_socket "tests/test_socket.cc" sylar "${LIBS}")
sylar_add_executable(test_bytearray "tests/test_bytearray.cc" sylar "${LIBS}")
sylar_add_executable(test_http "tests/test_http.cc" sylar "${LIBS}")
sylar_add_executable(test_http_parser "tests/test_http_parser.cc" sylar "${LIBS}")
sylar_add_executable(test_tcp_server "tests/test_tcp_server.cc" sylar "${LIBS}")

sylar_add_executable(echo_server "examples/echo_server.cc" sylar "${LIBS}")
sylar_add_executable(echo_server_udp "examples/echo_server_udp.cc" sylar "${LIBS}")
sylar_add_executable(echo_udp_client "examples/echo_udp_client.cc" sylar "${LIBS}")

sylar_add_executable(test_http_server "tests/test_http_server.cc" sylar "${LIBS}")
sylar_add_executable(test_http_connection "tests/test_http_connection.cc" sylar "${LIBS}")
sylar_add_executable(test_http_conn_serv "tests/test_http_conn_serv.cc" sylar "${LIBS}")
sylar_add_executable(test_uri "tests/test_uri.cc" sylar "${LIBS}")

sylar_add_executable(my_http_server "samples/my_http_server.cc" sylar "${LIBS}")

sylar_add_executable(test_daemon "tests/test_daemon.cc" sylar "${LIBS}")
sylar_add_executable(test_env "tests/test_env.cc" sylar "${LIBS}")
sylar_add_executable(test_config_load "tests/test_config_load.cc" sylar "${LIBS}")
sylar_add_executable(test_application "tests/test_application.cc" sylar "${LIBS}")
sylar_add_executable(test_websocket "tests/test_websocket.cc" sylar "${LIBS}")
sylar_add_executable(test_ws_client "tests/test_ws_client.cc" sylar "${LIBS}")
sylar_add_executable(test_ws_server "tests/test_ws_server.cc" sylar "${LIBS}")

add_library(test SHARED tests/test_module.cc)
endif()

sylar_add_executable(bin_sylar "sylar/main.cc" sylar "${LIBS}")
set_target_properties(bin_sylar PROPERTIES OUTPUT_NAME "sylar")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)   # 可执行文件路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)      # 动态库路径