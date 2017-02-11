cmake_minimum_required (VERSION 3.5)
project (shri314-myclient)
enable_testing()

add_compile_options("-std=c++1z")
add_compile_options("-Wall")
add_compile_options("-Werror")
add_compile_options("-fmax-errors=3")
add_compile_options("-g3")
add_compile_options("-O3")
add_compile_options("-flto")

include_directories("include")

set(myclient_LIBS
      boost_system
      boost_thread
      pthread
   )

set(myclient_SOURCES
      "src/MyOauth2Client.cpp"
   )

add_executable        (myclient ${myclient_SOURCES})
target_link_libraries (myclient ${myclient_LIBS}   )

add_subdirectory      (test EXCLUDE_FROM_ALL)
