
project( SecurityCam.App )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Multimedia REQUIRED)
find_package(Qt6MultimediaWidgets REQUIRED)

add_definitions( -DCFGFILE_QT_SUPPORT )
add_definitions( -DARGS_QSTRING_BUILD )

set( SRC main.cpp
    mainwindow.cpp
	mainwindow.hpp
	options.cpp
	options.hpp
	options.ui
	frames.cpp
	frames.hpp
	view.hpp
	view.cpp
	resolution.cpp
	resolution.hpp
	resolution.ui )

qt5_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC SecurityCam.rc )
endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cfgfile
	${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/Args
	${CMAKE_CURRENT_BINARY_DIR} )

add_custom_target( cfgGen
   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../3rdparty/cfgfile/generator/cfgfile.generator${CMAKE_EXECUTABLE_SUFFIX} -i ${CMAKE_CURRENT_SOURCE_DIR}/cfg.qtconf -o ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	)

add_executable( SecurityCam.App WIN32 ${SRC} )

add_dependencies( cfgGen cfgfile.generator )

add_dependencies( SecurityCam.App cfgGen )

target_link_libraries( SecurityCam.App Qt5::MultimediaWidgets Qt5::Multimedia Qt5::Widgets
	Qt5::Gui Qt5::Core ${OpenCV_LIBRARIES} )
