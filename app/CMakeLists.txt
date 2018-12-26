 cmake_minimum_required(VERSION 3.4.1)


 file(GLOB bzip_source src/main/cpp/bzip/*.c)

 add_library(
              native-lib

              SHARED

              src/main/cpp/native-lib.cpp
              src/main/cpp/bspatch.c
              ${bzip_source}

               )
 include_directories(src/main/cpp/bzip)


 find_library(
               log-lib
               log )


 target_link_libraries(
                        native-lib
                        ${log-lib} )