cmake_minimum_required(VERSION 3.0)
if(POLICY CMP0043)
	cmake_policy(SET CMP0043 NEW)
endif()
if(POLICY CMP0063)
	cmake_policy(SET CMP0063 NEW)
endif()
project(qdigidoc VERSION 3.13.5)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include( GNUInstallDirs )
include( VersionInfo )

find_package( PKCS11 )
find_package( LibDigiDocpp REQUIRED )
find_package( OpenSSL REQUIRED )
find_package( Qt5 COMPONENTS Core Widgets Network PrintSupport LinguistTools REQUIRED )
include_directories( ${LIBDIGIDOCPP_INCLUDE_DIR} )

if( WIN32 )
	add_subdirectory( extensions/EsteidShellExtension )
elseif( APPLE )
	add_subdirectory( extensions/DigiDocQL )
else()
	option(ENABLE_KDE "Install KDE service menu (default: TRUE)" TRUE)
	option(ENABLE_NAUTILUS_EXTENSION "Build Nautilus extension (default: TRUE)" TRUE)
	if (ENABLE_KDE)
		add_subdirectory( extensions/kde )
	endif()
	if (ENABLE_NAUTILUS_EXTENSION)
		add_subdirectory( extensions/nautilus )
	endif()
endif()
add_subdirectory( common )
add_subdirectory( crypto )
add_subdirectory( client )
