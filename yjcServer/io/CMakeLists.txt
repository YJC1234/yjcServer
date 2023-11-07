file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(my_io STATIC ${srcs})

find_library(URING uring REQUIRED)

target_include_directories(my_io PUBLIC include)
target_link_libraries(my_io ${URING} Config)
