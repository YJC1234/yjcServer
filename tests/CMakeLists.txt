file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp)

foreach(onesrc ${srcs})
    get_filename_component(onename ${onesrc} NAME_WE)
    add_executable(${onename} ${onesrc})
    target_link_libraries(${onename} PRIVATE spdlog::spdlog Config my_thread)
endforeach(onesrc ${srcs})

