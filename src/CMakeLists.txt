AUX_SOURCE_DIRECTORY(. src_files)

ADD_EXECUTABLE(cypress-drv ${src_files})

find_package(Threads)

target_link_libraries(cypress-drv ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS cypress-drv
	RUNTIME DESTINATION bin
)


