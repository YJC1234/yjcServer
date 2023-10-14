file(GLOB_RECURSE srcs CMAKE_CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(logConfig STATIC ${srcs})

target_include_directories(logConfig PUBLIC include)
target_link_libraries(logConfig PUBLIC spdlog::spdlog)