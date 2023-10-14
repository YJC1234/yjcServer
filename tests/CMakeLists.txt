add_executable(test test.cpp)

target_link_libraries(test PRIVATE spdlog::spdlog logConfig my_thread)