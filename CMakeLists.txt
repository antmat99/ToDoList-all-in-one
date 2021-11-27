cmake_minimum_required(VERSION 3.5)

project(eccoci VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(eccoci
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET eccoci APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(eccoci SHARED
            ${PROJECT_SOURCES}
            )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(eccoci
            ${PROJECT_SOURCES}
            )
    endif()
endif()

#target_link_libraries(eccoci PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(eccoci PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(eccoci)
endif()


#############################   CURL / CURLPP cmakelist #############################

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW) # MACOSX_RPATH
    set(CMAKE_MACOSX_RPATH TRUE)
endif()

# for unix platform, define install directories.
include(GNUInstallDirs)

if(WIN32)
    # cmake 3.4 is required for CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
    cmake_minimum_required(VERSION 3.4)

    # c++ 11 support from cmake 3.4 or newer
    set(CMAKE_CXX_STANDARD 11) # C++11...
    set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
    set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_LESS 3.1)
        cmake_minimum_required(VERSION 2.8)
        # c++11 support for cmake 2.8.12 - 3.0.x
        #
        # for non-windows platform we try to keep cmake 2.8 support
        # since entreprise distribution tends to have 2.8 version.
        add_compile_options(-std=c++11)
    else()
        # c++ 11 support from cmake 3.1 or newer
        set(CMAKE_CXX_STANDARD 11) # C++11...
        set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
        set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
    endif()
endif()



# Conan.io integration
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

# extra (pkg-config-related files)
add_subdirectory(curlpp-master/extras)

#######################
# Look for dependencies

# Documented at https://cmake.org/cmake/help/v3.0/module/FindCURL.html?highlight=curlpp
# Seems simple.

message(STATUS "Looking for CURL")
include(FindCURL)
find_package(CURL REQUIRED)

if(CURL_FOUND)
    message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
    message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
    message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find CURL")
endif()

# All following targets should search these directories for headers
include_directories(
    curlpp-master/include
    ${CURL_INCLUDE_DIRS}
    )

################
# Define Targets

# If building on windows, install path will be in build/winbuild
if(CMAKE_SYSTEM MATCHES "Windows")
    set(CMAKE_INSTALL_PREFIX "winbuild")
endif()

file(GLOB_RECURSE HeaderFileList "curlpp-master//include/*")
file(GLOB_RECURSE SourceFileList "curlpp-master/src/*")
add_library(${PROJECT_NAME}_lib SHARED ${HeaderFileList} ${SourceFileList})
target_link_libraries(${PROJECT_NAME}_lib ${CURL_LIBRARIES} ${CONAN_LIBS})
set_target_properties(${PROJECT_NAME}_lib PROPERTIES SOVERSION 1 VERSION 1.0.0)

add_library(${PROJECT_NAME}_static_lib STATIC ${HeaderFileList} ${SourceFileList})

# Make sure that on unix-platforms shared and static libraries have
# the same root name, but different suffixes.
#
#  (solution taken from https://cmake.org/Wiki/CMake_FAQ#How_do_I_make_my_shared_and_static_libraries_have_the_same_root_name.2C_but_different_suffixes.3F)
#
# Making shared and static libraries have the same root name, but different suffixes
SET_TARGET_PROPERTIES(${PROJECT_NAME}_static_lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_static_lib)
# Now the library target "curlpp_static" will be named "curlpp.lib" with MS tools.
# This conflicts with the "curlpp.lib" import library corresponding to "curlpp.dll",
# so we add a "lib" prefix (which is default on other platforms anyway):
SET_TARGET_PROPERTIES(${PROJECT_NAME}_static_lib PROPERTIES PREFIX "lib")
target_link_libraries(${PROJECT_NAME}_static_lib ${CURL_LIBRARIES} ${CONAN_LIBS})
# install headers
install(DIRECTORY include/utilspp/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/utilspp")
install(DIRECTORY include/curlpp/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/curlpp")

install(TARGETS ${PROJECT_NAME}_lib ${PROJECT_NAME}_static_lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})


target_link_libraries(eccoci PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${PROJECT_NAME}_lib)

#############################   OAUTHCPP cmakelist #############################
#############################   mai stata usata    #############################

IF(APPLE)
IF(IS_DIRECTORY /Developer/SDKs/MacOSX10.5.sdk)
SET(CMAKE_CXX_COMPILER g++-4.0)
SET(CMAKE_C_COMPILER gcc-4.0)
ELSE()
IF(EXISTS /usr/bin/g++-4.2)
SET(CMAKE_CXX_COMPILER g++-4.2)
SET(CMAKE_C_COMPILER gcc-4.2)
ELSE()
# Don't try to override. This is probably a newer setup
# using XCode 4.2+ which doesn't provide versioned gcc's
# since it uses llvm.
#SET(CMAKE_CXX_COMPILER g++)
#SET(CMAKE_C_COMPILER gcc)
ENDIF()
ENDIF()
ENDIF()

# Disable some annoying, unnecessary Windows warnings. Enable unwinding for exception handlers.
IF(MSVC)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -EHsc)
SET(CMAKE_CXX_FLAGS "-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS")
ENDIF()

# Force a default build type
IF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE Release CACHE STRING
    "Build types include: Default Debug Release None RelWithDebInfo." FORCE
    )
ENDIF()
MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
# And add a definition so code can tell
IF(CMAKE_BUILD_TYPE EQUAL "Debug")
ADD_DEFINITIONS(-DLIBOAUTHCPP_DEBUG)
ENDIF()

# Define a few helpful directories
IF(NOT LIBOAUTHCPP_TOP_LEVEL)
SET(LIBOAUTHCPP_TOP_LEVEL ${CMAKE_CURRENT_SOURCE_DIR}/..)
ENDIF()
GET_FILENAME_COMPONENT(LIBOAUTHCPP_TOP_LEVEL ${LIBOAUTHCPP_TOP_LEVEL} ABSOLUTE)
MESSAGE(STATUS "Base directory: ${LIBOAUTHCPP_TOP_LEVEL}")
SET(LIBOAUTHCPP_INCLUDE ${LIBOAUTHCPP_TOP_LEVEL}/eccoci/liboauthcpp-master/include)
SET(LIBOAUTHCPP_SRC ${LIBOAUTHCPP_TOP_LEVEL}/eccoci/liboauthcpp-master/src)
SET(LIBOAUTHCPP_TEST ${LIBOAUTHCPP_TOP_LEVEL}/eccoci/liboauthcpp-master/tests)
SET(LIBOAUTHCPP_DEMO ${LIBOAUTHCPP_TOP_LEVEL}/eccoci/liboauthcpp-master/demo)

# CMake doesn't seem to allow adding include directories for specific
# projects...
INCLUDE_DIRECTORIES(${LIBOAUTHCPP_INCLUDE})

# Handle configuration options from parent projects
IF(LIBOAUTHCPP_ADDED_DEFINITIONS)
ADD_DEFINITIONS(${LIBOAUTHCPP_ADDED_DEFINITIONS})
ENDIF()

# The main library
SET(LIBOAUTHCPP_LIB_SOURCES
    ${LIBOAUTHCPP_SRC}/base64.cpp
    ${LIBOAUTHCPP_SRC}/HMAC_SHA1.cpp
    ${LIBOAUTHCPP_SRC}/liboauthcpp.cpp
    ${LIBOAUTHCPP_SRC}/SHA1.cpp
    ${LIBOAUTHCPP_SRC}/urlencode.cpp
    )
ADD_LIBRARY(oauthcpp STATIC ${LIBOAUTHCPP_LIB_SOURCES})
INSTALL(TARGETS oauthcpp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
    )
INSTALL(DIRECTORY
    ${LIBOAUTHCPP_INCLUDE}/ DESTINATION include
    )

# Allow disabling of tests
IF(NOT DEFINED LIBOAUTHCPP_BUILD_TESTS)
SET(LIBOAUTHCPP_BUILD_TESTS TRUE CACHE BOOL "Whether to build tests")
ELSE()
SET(LIBOAUTHCPP_BUILD_TESTS LIBOAUTHCPP_BUILD_TESTS CACHE BOOL "Whether to build tests")
ENDIF()

#############################   HTTPLIB cmakelist #############################

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::SSL>
    $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::Crypto>)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:CPPHTTPLIB_OPENSSL_SUPPORT>
    )


