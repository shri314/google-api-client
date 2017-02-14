cmake_minimum_required (VERSION 3.5)
project (shri314-myclient)

SET(CMAKE_BUILD_TYPE "custom")
SET(CMAKE_CXX_FLAGS_CUSTOM "-std=c++1z -Wall -Werror -fmax-errors=3 -g3 -flto")

include_directories("include")

set(myclient_LIBS
      boost_system
      cpprest
      pthread
      ssl
      crypto
   )

set(myclient_SOURCES
      "src/MyOauth2Client.cpp"
   )

add_executable        (myclient ${myclient_SOURCES})
target_link_libraries (myclient ${myclient_LIBS}   )

enable_testing()
add_subdirectory      (test EXCLUDE_FROM_ALL)
