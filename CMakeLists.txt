# kate: replace-tabs off; tab-width 4; indent-width 4; tab-indents true; indent-mode normal
# vim: ts=4:sw=4:noexpandtab

cmake_minimum_required(VERSION 2.6)

project(rackfocusfixer)

find_package(Qt4 REQUIRED)

set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(SRCS FocusEditorWidget.cpp main.cpp)
qt4_wrap_cpp(MOCS FocusEditorWidget.h)

add_executable(rff WIN32 ${SRCS} ${MOCS})

target_link_libraries(rff ${QT_LIBRARIES})

install(TARGETS rff RUNTIME DESTINATION bin LIBRARY DESTINATION bin)