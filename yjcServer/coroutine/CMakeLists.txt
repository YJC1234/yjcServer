
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(my_coroutine STATIC ${srcs})

target_include_directories(my_coroutine PUBLIC include)
target_link_libraries(my_coroutine PUBLIC Config)
