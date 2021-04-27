find_path(glog_SOURCE CMakeLists.txt HINTS "${CMAKE_SOURCE_DIR}/externals/glog")

if (glog_SOURCE)
    set(glog_FOUND TRUE)
    set(glog_BUILD "${CMAKE_CURRENT_BINARY_DIR}/glog_build")
    set(glog_DISTRIBUTION "${CMAKE_CURRENT_BINARY_DIR}/glog_distribution")

    # Build glog
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} ${glog_BUILD})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} ${glog_DISTRIBUTION})
    execute_process(COMMAND ${CMAKE_COMMAND} ${glog_SOURCE}
            -DCMAKE_INSTALL_PREFIX=${glog_DISTRIBUTION}
            -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
            -DBUILD_TESTING=NO
            WORKING_DIRECTORY ${glog_BUILD}
            )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install
            WORKING_DIRECTORY ${glog_BUILD}
            )

    set(glog_INCLUDE "${glog_DISTRIBUTION}/include")

    if (MSVC)
        find_library(GLOG_LIBRARY_RELEASE libglog_static
                PATHS ${glog_DISTRIBUTION}
                PATH_SUFFIXES Release)

        find_library(GLOG_LIBRARY_DEBUG libglog_static
                PATHS ${glog_DISTRIBUTION}
                PATH_SUFFIXES Debug)

        set(glog_LIBRARY optimized ${GLOG_LIBRARY_RELEASE} debug ${GLOG_LIBRARY_DEBUG})
    else ()
        # FIXME
#        find_library(glog_LIBRARY
#                NAMES glog
#                PATHS ${glog_DISTRIBUTION}
#                PATH_SUFFIXES lib lib64)
        set(glog_LIBRARY "${glog_DISTRIBUTION}/lib/libglog.a")

        message(STATUS "${Green}Found Glog library at: ${glog_LIBRARY}${Reset}")
    endif ()

    message(STATUS "${Green}Found Glog include at: ${glog_SOURCE}${Reset}")
else ()
    message(FATAL_ERROR "${Red}Failed to locate Glog dependency.${Reset}")
endif ()