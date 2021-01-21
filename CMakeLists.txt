###################################
##  CMAKE FILE FOR PHONOMETRICA  ##
###################################

cmake_minimum_required(VERSION 3.17)

if(APPLE)
    project(Phonometrica)
else()
    project(phonometrica)
endif(APPLE)

set(CMAKE_CXX_STANDARD 17)


###########################
##  COMPILATION OPTIONS  ##
###########################

# Embed scripts as strings or read the from files
set(EMBED_SCRIPTS ON)
# Logging (for debugging)
set(ENABLE_LOGGING ON)
# Compile the whole application or the scripting engine only
set(WITH_GUI ON)
# Run unit tests for core types
set(BUILD_UNIT_TEST OFF)

if (WIN32)
    set(BOOST_DIR "C:/Devel/Lib/boost_1_72_0")
    set(LIBSNDFILE_DIR "C:/Devel/Lib/libsndfile")
endif()

if(WIN32)
    set(BUILD_X64 off)
    set(LIB_DIR "C:/Lib")
    include_directories(${LIB_DIR}/include)

    set(wxWidgets_FIND_STYLE "unix")
    set(wxWidgets_ROOT_DIR C:/Lib/wxWidgets)
    set(wxWidgets_LIB_DIR C:/Lib/wxWidgets/lib/vc_lib)
    set(wxWidgets_CONFIGURATION mswu)
    set(wxWidgets_INCLUDE_DIRS C:/Lib/wx/include C:/Lib/wx/lib/vc_lib/mswu)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    #include_directories(C:/Lib/wxWidgets/include C:/Lib/wxWidgets/lib/vc_lib/mswu)

    if (BUILD_X64)
        link_directories(${LIB_DIR}/x64)
    else()
        link_directories(${LIB_DIR}/x86)
    endif(BUILD_X64)
endif(WIN32)


###########################
##  VERSION INFORMATION  ##
###########################

# Major release: incremented when major changes are introduced
add_definitions(-DPHON_VERSION_MAJOR=0)
# Minor release: incremented when new features and/or bug fixes are introduced
add_definitions(-DPHON_VERSION_MINOR=8)
# Revision release: incremented when bugs are fixed are small features are added
add_definitions(-DPHON_VERSION_MICRO=0)
# Build: this is only used internally for development (set to 0 on public release)
add_definitions(-DPHON_VERSION_NANO=3)

# Release date
add_definitions(-DPHON_RELEASE_DATE_YEAR=2021)
add_definitions(-DPHON_RELEASE_DATE_MONTH=1)
add_definitions(-DPHON_RELEASE_DATE_DAY=21)


########################
##  OPERATING SYSTEM  ##
########################

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

if (ENABLE_LOGGING)
    add_definitions(-DPHON_ENABLE_LOGGING=1)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_definitions(-DPHON_WITH_WX=1)

if(WIN32)
    # Must be defined before including cmath with MSVC to get M_PI
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
        phon/runtime/*.cpp
        phon/runtime/compiler/*.cpp
        phon/utils/*.cpp
        phon/third_party/utf8proc/utf8proc.c
)

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

if (EMBED_SCRIPTS)
    add_definitions(-DPHON_EMBED_SCRIPTS=1)
endif()

# SPTK
#include_directories(phon/third_party/sptk/include/)

if (WITH_GUI)
    add_definitions(-DPHON_GUI=1)

    if(WIN32)
        set(EXTRA_COMPONENTS scintilla)
    else()
        set(EXTRA_COMPONENTS "")
    endif(WIN32)

    # XML is needed by richtext
    find_package(wxWidgets COMPONENTS aui html adv stc richtext xml ${EXTRA_COMPONENTS} core base REQUIRED)
    include("${wxWidgets_USE_FILE}")

    # needed by wxCheckListBox
    if(WIN32)
        add_definitions(-DwxUSE_OWNER_DRAWN=1)
    endif(WIN32)


    if(CMAKE_COMPILER_IS_GNUCXX)
        message("Compiling with GNU C++")
        set(CMAKE_CXX_FLAGS "-Wall") # -Wextra")
    endif()

    file(GLOB APP_FILES
            phon/application/*.cpp
            phon/gui/*.cpp
            phon/gui/views/*.cpp
            phon/gui/pref/*.cpp
            phon/third_party/zip/zip.cpp
    )

else (WITH_GUI)
    find_package(wxWidgets COMPONENTS core base REQUIRED)
    include("${wxWidgets_USE_FILE}")
    file(GLOB APP_FILES )
endif(WITH_GUI)

add_executable(${PROJECT_NAME} phonometrica.cpp ${APP_FILES})
target_link_libraries(${PROJECT_NAME} phon-runtime pcre2-8 ${wxWidgets_LIBRARIES})

if(BUILD_UNIT_TEST)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    file(GLOB TEST_FILES ./unit_test/*.cpp)
    add_executable(test_phon ${TEST_FILES})
    target_link_libraries(test_phon phon-runtime pcre2-8 ${wxWidgets_LIBRARIES})
endif(BUILD_UNIT_TEST)
