
set(SOURCES main.c)
add_executable(weka_project ${SOURCES})
target_link_libraries(weka_project soem)
install(TARGETS weka_project DESTINATION bin)
