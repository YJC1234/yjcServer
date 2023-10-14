find_package(Threads REQUIRED)

file(GLOB_RECURSE srcs CMAKE_CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(my_thread STATIC ${srcs})

target_include_directories(my_thread PUBLIC include)
target_link_libraries(my_thread PUBLIC Threads::Threads logConfig)
