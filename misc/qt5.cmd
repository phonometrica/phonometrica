REM Setup build environment for Qt with Visual Studio on Windows
REM See: https://doc.qt.io/qt-5/windows-building.html

REM Set up Microsoft Visual Studio 2019 for Windows 64 bit
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
SET _ROOT=C:\Qt\5.13.0
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
REM Uncomment the below line when using a git checkout of the source repository
REM SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET _ROOT=


REM Builing Qt:
REM configure -release -nomake examples -nomake tests -skip qtwebengine -skip qtdeclarative -skip qtconnectivity -skip qtscript -opensource
REM jom
