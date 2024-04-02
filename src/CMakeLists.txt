
project( SecurityCam.App )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Multimedia REQUIRED)

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
	resolution.ui
	license_dialog.hpp
	license_dialog.cpp
	license_dialog.ui
	${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp )

qt6_add_resources( SRC resources.qrc )

if( WIN32 )
	list( APPEND SRC SecurityCam.rc )
endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cfgfile
	${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/args-parser
	${CMAKE_CURRENT_BINARY_DIR} )
	
set_property( SOURCE ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp PROPERTY SKIP_AUTOGEN ON )

add_custom_command( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	PRE_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../3rdparty/cfgfile/generator/${CMAKE_CFG_INTDIR}/cfgfile.generator${CMAKE_EXECUTABLE_SUFFIX} -i cfg.qtconf -o ${CMAKE_CURRENT_BINARY_DIR}/cfg.hpp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable( SecurityCam.App WIN32 ${SRC} )

add_dependencies( SecurityCam.App cfgfile.generator )

target_link_libraries( SecurityCam.App PUBLIC Qt6::Multimedia Qt6::Widgets
	Qt6::Gui Qt6::Core ${OpenCV_LIBRARIES} )
