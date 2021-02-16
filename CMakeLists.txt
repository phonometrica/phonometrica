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

    message(${CMAKE_BUILD_TYPE})
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(SUFFIX "d")
        set(OUTPUT_DIR "Debug")
        message("Debug build")
    else()
        set(SUFFIX "")
        set(OUTPUT_DIR "Release")
        message("Release build")
    endif(CMAKE_BUILD_TYPE MATCHES "Debug")

    set(BOOST_DIR "C:/Devel/Lib/boost_1_72_0")
    set(LIBSNDFILE_DIR "C:/Devel/Lib/libsndfile")

    include_directories(C:/Devel/Lib/fftw-3.3.9/build/${OUTPUT_DIR}
            C:/Devel/Lib/fftw-3.3.9/build
            C:/Devel/Lib/fftw-3.3.9/api
            )
    link_directories(C:/Devel/Lib/fftw-3.3.9/build/${OUTPUT_DIR}
            )

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MD${SUFFIX}")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj /MD${SUFFIX}")
    SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /bigobj /MD${SUFFIX}")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj /MD${SUFFIX}")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj /MD${SUFFIX}")
endif()

if(WIN32)
    # FIXME: find_package() doesn't work well with wxWidgets on Windows. On this platform, we hard-code include
    #  and link paths. This is less portable, but it works reliably.
    set(WX_ROOT C:/Devel/Lib/wxWidgets-3.1.4)
    include_directories(${WX_ROOT}/include ${WX_ROOT}/release/lib/vc_x64_lib/mswu${SUFFIX})
    link_directories(${WX_ROOT}/release/lib/vc_x64_lib)
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
add_definitions(-DPHON_VERSION_NANO=12)

# Release date
add_definitions(-DPHON_RELEASE_DATE_YEAR=2021)
add_definitions(-DPHON_RELEASE_DATE_MONTH=2)
add_definitions(-DPHON_RELEASE_DATE_DAY=16)


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
    add_definitions(-DWIN32_LEAN_AND_MEAN=1)
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


if (WITH_GUI)
    add_definitions(-DPHON_GUI=1)

    if (WIN32)
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
    endif(WIN32)

    # SPTK
    include_directories(phon/third_party/sptk/include/)

    # So that LBFGS can find Eigen
    include_directories(${CMAKE_SOURCE_DIR}/phon/third_party)

    if(WIN32)
        set(EXTRA_COMPONENTS scintilla)
    else()
        set(EXTRA_COMPONENTS "")
    endif(WIN32)


    if(WIN32)
        # FIXME: wxWindows is supposed to enable DPI awareness with this macro but this doesn't seem to work on my
        #  machine. Instead, we call SetProcessDPIAware() in WinMain() before creating the application. This seems
        #  to work, although Microsoft recommends using a manifest file instead. See:
        #  https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setprocessdpiaware
        #add_definitions(-DwxUSE_DPI_AWARE_MANIFEST=2)

        # needed by wxCheckListBox
        add_definitions(-DwxUSE_OWNER_DRAWN=1)

        set(WX_LIBS
                #wxmsw31u${SUFFIX}_propgrid.lib
                #wxmsw31u${SUFFIX}_ribbon.lib
                wxmsw31u${SUFFIX}_stc.lib
                #wxmsw31u${SUFFIX}_webview.lib
                #wxmsw31u${SUFFIX}_gl.lib
                wxmsw31u${SUFFIX}_richtext.lib
                #wxmsw31u${SUFFIX}_xrc.lib
                wxmsw31u${SUFFIX}_aui.lib
                wxmsw31u${SUFFIX}_html.lib
                wxmsw31u${SUFFIX}_adv.lib
                wxmsw31u${SUFFIX}_core.lib
                wxbase31u${SUFFIX}_xml.lib
                #wxbase31u${SUFFIX}_net.lib
                wxbase31u${SUFFIX}.lib
                wxscintilla${SUFFIX}.lib
                wxtiff${SUFFIX}.lib
                wxjpeg${SUFFIX}.lib
                wxpng${SUFFIX}.lib
                wxzlib${SUFFIX}.lib
                wxregexu${SUFFIX}.lib
                wxexpat${SUFFIX}.lib
#                wxmsw31u${SUFFIX}_media.lib
        )

    else(WIN32)
        # XML is needed by richtext
        find_package(wxWidgets COMPONENTS aui html adv stc richtext xml ${EXTRA_COMPONENTS} core base REQUIRED)
        #find_package(wxWidgets 3.1 REQUIRED)
        include("${wxWidgets_USE_FILE}")
    endif(WIN32)

    add_definitions(-DPHON_USE_FFTW=1)


    if(CMAKE_COMPILER_IS_GNUCXX)
        message("Compiling with GNU C++")
        set(CMAKE_CXX_FLAGS "-Wall") # -Wextra")
    endif()

    file(GLOB APP_FILES
            phon/application/*.cpp
            phon/application/conc/*.cpp
            phon/application/cmd/*.cpp
            phon/analysis/*.cpp
            phon/gui/*.cpp
            phon/gui/views/*.cpp
            phon/gui/pref/*.cpp
            phon/gui/conc/*.cpp
            phon/gui/ctrl/*.cpp
            phon/third_party/pugixml/pugixml.cpp
            phon/third_party/rtaudio/RtAudio.cpp
            phon/third_party/sqlite/sqlite3.c
            phon/third_party/sendpraat.c
            phon/third_party/r8brain/*.cpp
            phon/third_party/zip/zip.cpp
            phon/third_party/sptk/*.c
            phon/third_party/sptk/*.cpp
            #phon/third_party/snack/*.c
            phon/third_party/swipe/*.c phon/third_party/swipe/*.cpp phon/third_party/swipe/*.h
    )

else (WITH_GUI)
    find_package(wxWidgets COMPONENTS core base REQUIRED)
    include("${wxWidgets_USE_FILE}")
    file(GLOB APP_FILES )
endif(WITH_GUI)


if (WIN32)
    add_definitions(-D__WINDOWS_DS__=1)
    include_directories(${LIBSNDFILE_DIR})
    link_directories(${LIBSNDFILE_DIR})

    add_executable(${PROJECT_NAME} WIN32 phonometrica.cpp windows.rc ${APP_FILES})
    target_link_libraries(
            ${PROJECT_NAME} phon-runtime pcre2-8 fftw3 ole32 comctl32 winmm uuid Rpcrt4 User32 dsound
            ${LIBSNDFILE_DIR}/libsndfile-1.lib
            ${WX_LIBS}
    )
elseif (APPLE)
    add_definitions(-D__MACOSX_CORE__=1)
    include_directories(/opt/local/include)
    link_directories(/opt/local/lib)

    # Since we have a version of libiconv in /opt/local/lib, the linker will look for that version first and will
    # fail. As a workaround, we pass the full path to the correct version we want to link against.
    set(LIB_ICONV /usr/lib/libiconv.dylib)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE phonometrica.cpp ${APP_FILES})
    target_link_libraries(
            ${PROJECT_NAME} phon-runtime pcre2-8 fftw3 pthread dl sndfile ${LIB_ICONV}
            ${wxWidgets_LIBRARIES}
            "-framework Foundation" "-framework Cocoa" "-framework CoreAudio" objc
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Phonometrica")
else()
    add_definitions(-D__LINUX_ALSA__=1)

    set(FFTW3 /usr/lib/x86_64-linux-gnu/libfftw3.a)
    add_executable(${PROJECT_NAME} phonometrica.cpp ${APP_FILES})
    target_link_libraries(
            ${PROJECT_NAME} phon-runtime pcre2-8 ${FFTW3} pthread dl asound sndfile ${wxWidgets_LIBRARIES}
    )

endif (WIN32)


if(BUILD_UNIT_TEST)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    file(GLOB TEST_FILES ./unit_test/*.cpp)
    add_executable(test_phon ${TEST_FILES})
    target_link_libraries(test_phon phon-runtime pcre2-8 ${wxWidgets_LIBRARIES})
endif(BUILD_UNIT_TEST)
