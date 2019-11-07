cmake_minimum_required(VERSION 3.12)

if(APPLE)
    project(Phonometrica)
else()
    project(phonometrica)
endif(APPLE)

set(CMAKE_CXX_STANDARD 17)


###########################
##  COMPILATION OPTIONS  ##
###########################

set(BUILD_GUI ON)
set(EMBED_SCRIPTS ON)
set(BUILD_UNIT_TEST OFF)
set(DEBUG_CONSOLE OFF)
set(ENABLE_LOGGING OFF)
set(USE_FFTW OFF)

if (WIN32)
    set(QT_DIR "C:/Qt5/5.12.5/mingw73_64/lib/cmake/Qt5")
    set(BOOST_DIR "C:/Devel/Lib/boost_1_66_0")
    set(LIBSNDFILE_DIR "C:/Devel/Lib/libsndfile")
endif()

###########################
##  VERSION INFORMATION  ##
###########################

# Major release: incremented when major changes are introduced
add_definitions(-DPHON_VERSION_MAJOR=0)
# Minor release: incremented when new features and/or bug fixes are introduced
add_definitions(-DPHON_VERSION_MINOR=7)
# Revision release: incremented when bugs are fixed are small features are added
add_definitions(-DPHON_VERSION_MICRO=2)
# Build: this is only used internally for development (set to 0 on public release)
add_definitions(-DPHON_VERSION_NANO=1)

# Release date
add_definitions(-DPHON_RELEASE_DATE_YEAR=2019)
add_definitions(-DPHON_RELEASE_DATE_MONTH=11)
add_definitions(-DPHON_RELEASE_DATE_DAY=7)

# Operating system
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-DPHON_WINDOWS=1 -DPHON_MACOS=0 -DPHON_LINUX=0 -DPHON_UNIX=0)
    if (MINGW)
        add_definitions(-DPHON_POSIX=1 -DPHON_MINGW=1)
    else()
        add_definitions(-DPHON_POSIX=0 -DPHON_MINGW=0)
    endif()
else()
    add_definitions(-DPHON_POSIX=1 -DPHON_UNIX=1 -DPHON_WINDOWS=0)

    if(APPLE)
        add_definitions(-DPHON_MACOS=1 -DPHON_LINUX=0)
    else()
        # Assume linux
        add_definitions(-DPHON_MACOS=0 -DPHON_LINUX=1)
        add_definitions(-DUNIX) # for sendpraat
    endif()
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


#####################
##  BUILD RUNTIME  ##
#####################

if (EMBED_SCRIPTS)
    add_definitions(-DPHON_EMBED_SCRIPTS=1)
endif()

if (NOT BUILD_UNIT_TEST)
    add_definitions(-DPHON_WITH_QT)
endif()

if (DEBUG_CONSOLE)
    add_definitions(-DPHON_DEBUG_CONSOLE=1)
endif()

if (ENABLE_LOGGING)
    add_definitions(-DPHON_ENABLE_LOGGING=1)
endif()

if (BUILD_GUI)
    add_definitions(-DPHON_GUI=1)
    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    # Instruct CMake to run moc automatically when needed
    set(CMAKE_AUTOMOC ON)
    # Handle resources files
    set(CMAKE_AUTORCC ON)

    # Find the QtWidgets library
    if (WIN32)
        set(Qt5_DIR ${QT_DIR})
        find_package(Qt5 REQUIRED PATHS ${QT_DIR} COMPONENTS Widgets Core Gui)
    elseif (APPLE)
        find_package(Qt5 REQUIRED PATHS $ENV{HOME}/Qt/5.11.2/clang_64/lib/cmake/Qt5/ COMPONENTS Widgets Core Gui)
        include_directories(/opt/local/include)
        link_directories(/opt/local/lib)
    endif(WIN32)
    find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

    if (LINUX)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

        include_directories(${GTK2_INCLUDE_DIRS})
        link_directories(${GTK2_LIBRARY_DIRS})
        add_definitions(${GTK2_CFLAGS_OTHER})
    endif (LINUX)

endif(BUILD_GUI)

include_directories(${CMAKE_SOURCE_DIR})

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
	add_definitions(-DUTF8PROC_STATIC)
	add_definitions(-DENABLE_SNDFILE_WINDOWS_PROTOTYPES=1 -DENABLE_SNDFILE_WINDOWS_PROTOTYPES=1)
    include_directories(${BOOST_DIR})

    if(MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
    endif()

elseif(APPLE)
    include_directories(/usr/local/include)
endif()

add_definitions(-DEIGEN_MPL2_ONLY)

file(GLOB RUNTIME_FILES
        phon/*.cpp phon/*.hpp
        phon/runtime/*.cpp phon/runtime/*.hpp phon/runtime/*.h
        phon/utils/*.cpp phon/utils/*.hpp
        phon/third_party/utf8proc/utf8proc.c
        phon/third_party/utf8proc/utf8proc.h
)


#if(CMAKE_COMPILER_IS_GNUCXX)
#    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#endif()


add_library(phon-runtime STATIC ${RUNTIME_FILES})
set_property(TARGET phon-runtime PROPERTY POSITION_INDEPENDENT_CODE ON)

# Use UTF-8 in PCRE2.
add_definitions(-DPCRE2_CODE_UNIT_WIDTH=8 -DPCRE2_STATIC=1)
add_subdirectory(phon/third_party/pcre2)
include_directories(${PROJECT_BINARY_DIR}/phon/third_party/pcre2)
link_directories(${PROJECT_BINARY_DIR}/phon/third_party/pcre2)

if (WIN32)
    target_link_libraries(phon-runtime shlwapi pcre2-8)
else()
    target_link_libraries(phon-runtime m pcre2-8)
endif(WIN32)


#########################
##  BUILD APPLICATION  ##
#########################

add_definitions(-DDEFAULT_WINDOW_DURATION=10.0)


if (BUILD_GUI)
    include_directories(${Qt5Widgets_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/phon)

    # SPTK
    include_directories(phon/third_party/sptk/include/)

    file(GLOB_RECURSE APP_FILES
            phon/application/*.cpp phon/application/*.hpp
            phon/analysis/*.cpp phon/analysis/*.hpp
            phon/gui/*.cpp phon/gui/*.hpp
            phon/third_party/pugixml/pugixml.cpp
            phon/third_party/rtaudio/RtAudio.cpp
            phon/third_party/sqlite/sqlite3.c
            phon/third_party/sendpraat.c
            phon/third_party/r8brain/*.cpp
            phon/third_party/zip/zip.cpp
            phon/third_party/qxt/qxtcheckcombobox.cpp
            phon/third_party/sptk/*.c
            phon/third_party/sptk/*.cpp
            #phon/third_party/snack/*.c
            phon/third_party/swipe/*.c phon/third_party/swipe/*.cpp phon/third_party/swipe/*.h
    )

    if(WIN32)
        set(GUI_TYPE WIN32)
		set(RC_FILE "phonometrica.rc")
    elseif(APPLE)
        set(GUI_TYPE MACOSX_BUNDLE)
		set(RC_FILE "")
    else()
        set(GUI_TYPE "")
		set(RC_FILE "")
    endif (WIN32)

    if (USE_FFTW)
        add_definitions(-DPHON_USE_FFTW=1)
        add_subdirectory(phon/third_party/fftw)
        include_directories(phon/third_party/fftw/api)
        link_directories(${PROJECT_BINARY_DIR}/phon/third_party/fftw)
        set(FFT_LIB fftw3)
    else()
        add_definitions(-DPHON_USE_FFTS=1)
        add_subdirectory(phon/third_party/ffts)
        include_directories(phon/third_party/ffts/include)
        link_directories(${PROJECT_BINARY_DIR}/phon/third_party/ffts)
        set(FFT_LIB ffts)
    endif (USE_FFTW)

    add_executable(${PROJECT_NAME} ${GUI_TYPE} phonometrica.cpp ${RC_FILE} icons.qrc ${APP_FILES})

    if (WIN32)
        add_definitions(-D__WINDOWS_DS__=1)
        add_definitions(-D_USE_MATH_DEFINES) # for M_PI
        include_directories(${LIBSNDFILE_DIR})
        link_directories(${LIBSNDFILE_DIR})
        #add_subdirectory("${PROJECT_SOURCE_DIR}/libsndfile")
        target_link_libraries(
                ${PROJECT_NAME} phon-runtime pcre2-8 ${FFT_LIB} ole32 winmm dsound
				${LIBSNDFILE_DIR}/libsndfile-1.lib
                Qt5::Core Qt5::Gui Qt5::Widgets
        )
    elseif (APPLE)
        add_definitions(-D__MACOSX_CORE__=1)
        target_link_libraries(
                ${PROJECT_NAME} phon-runtime pcre2-8 ${FFT_LIB} pthread dl sndfile
                Qt5::Core Qt5::Gui Qt5::Widgets
                "-framework Foundation" "-framework Cocoa" "-framework CoreAudio" objc
        )
    else()
        add_definitions(-D__LINUX_ALSA__=1)
        target_link_libraries(
                ${PROJECT_NAME} phon-runtime pcre2-8 ${FFT_LIB} pthread dl asound sndfile
                Qt5::Core Qt5::Gui Qt5::Widgets
                ${GTK2_LIBRARIES}
        )

    endif (WIN32)

else(BUILD_GUI)

    add_executable(${PROJECT_NAME} phonometrica.cpp)
    target_link_libraries(${PROJECT_NAME} phon-runtime pcre2-8)

endif(BUILD_GUI)


if(BUILD_UNIT_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    file(GLOB TEST_FILES ./unit_test/*.cpp)
    add_executable(test_phon ${TEST_FILES})
    target_link_libraries(test_phon phon-runtime pcre2-8 Qt5::Core)
endif(BUILD_UNIT_TEST)