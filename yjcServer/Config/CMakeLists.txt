file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

find_package(Boost REQUIRED)

add_library(Config STATIC ${srcs})

target_include_directories(Config PUBLIC include)
target_link_libraries(Config PUBLIC spdlog::spdlog yaml-cpp)