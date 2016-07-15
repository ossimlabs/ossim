@echo off

::*************************************************************************************
::
::  Windows Batch file for initializing the CMAKE build system for building OSSIM 
::  with MicroSoft compiler.
::
::  This script must be copied into a dedicated build directory.
::  The script can then be invoked from that directory. DO NOT run this script out of
::  the ossim_package_support/cmake/build_scripts/windows.
::  
::  Optional arguments (in any order):
::
::    win32 | win64 | x64 (defaults to x64)
::    vs2005 | vs2010 | vs2015 (defaults vs2015)
::    ide | nmake (defaults to ide)
::
::  No environment variables need be defined prior to running this script. However,
::  OSSIM_DEV_HOME, OSSIM_DEPENDENCIES and OSSIM_BUILD_DIR need to be edited below.
::
::  Author: Oscar Kramer, GEOEYE (Aug, 2011)
::
::*************************************************************************************

setlocal enabledelayedexpansion
set SCRIPT_DIR="%~dp0"

:: Edit these to your local system:
if NOT DEFINED OSSIM_DEPENDENCIES (
  set OSSIM_DEPENDENCIES=C:\GoAgent\pipelines\ossim-windows-temp\ossim-deps-1.0.0
)

:: Default settings:
set PLATFORM=x64
set DEVENV=vs2015
set GEN_TYPE=NMAKE
set CMAKE_BUILD_TYPE=Release
:while_valid_arg
  if "%1"=="win32" set PLATFORM=Win32
  if "%1"=="win64" set PLATFORM=x64
  if "%1"=="vs2005" set DEVENV=vs2005
  if "%1"=="vs2008" set DEVENV=vs2008
  if "%1"=="vs2015" set DEVENV=vs2015
  if "%1"=="nmake" set GEN_TYPE=NMAKE
  shift
if not "%1"=="" goto while_valid_arg

if not defined OSSIM_DEV_HOME (
   pushd %SCRIPT_DIR%..\..
   set OSSIM_DEV_HOME=!CD!
   popd
)


IF NOT DEFINED OSSIM_DEPENDENCIES (
  set OSSIM_DEPENDENCIES=%OSSIM_DEV_HOME\ossim-deps-1.0.0
)
IF NOT EXIST %OSSIM_DEPENDENCIES% (
  echo ERROR: OSSIM_DEPENDENCIES is not set to a valid path. Aborting
  GOTO ABORTED_END
)

IF NOT DEFINED OSSIM_BUILD_DIR (
  set OSSIM_BUILD_DIR=%OSSIM_DEV_HOME%\build
)
IF NOT EXIST %OSSIM_BUILD_DIR% MKDIR %OSSIM_BUILD_DIR%
pushd %OSSIM_BUILD_DIR%

:: Change all backslashes to forward slashes in path env vars:
set OSSIM_DEV_HOME=%OSSIM_DEV_HOME:\=/%
set OSSIM_DEPENDENCIES=%OSSIM_DEPENDENCIES:\=/%
set CMAKE_DIR=%OSSIM_DEV_HOME%\ossim\cmake

echo.
echo Environment settings:
echo OSSIM_DEV_HOME = %OSSIM_DEV_HOME%
echo OSSIM_DEPENDENCIES = %OSSIM_DEPENDENCIES%
echo PLATFORM = %PLATFORM%
echo DEVENV = %DEVENV%
echo OSSIM_BUILD_DIR = %OSSIM_BUILD_DIR%
echo.

:: Create system specific subdirectory if needed:
if not exist %DEVENV% mkdir %DEVENV%
pushd %DEVENV%

:: Create platform and target specific subdirectory if needed:
if not exist %PLATFORM% mkdir %PLATFORM%
pushd %PLATFORM%
if not exist %GEN_TYPE% mkdir %GEN_TYPE%
pushd %GEN_TYPE%

:: Interpret the target system and set up environment:
IF %DEVENV%==vs2005 (
  IF %PLATFORM%==Win32 (
    set TARGET_SYSTEM="Visual Studio 8 2005"
    call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall" x86
  ) ELSE (
    set TARGET_SYSTEM="Visual Studio 8 2005 Win64"
    call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall" x64
  )
) 
IF %DEVENV%==vs2008 (
  IF %PLATFORM%==Win32 (
    set TARGET_SYSTEM="Visual Studio 9 Win32"
    call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall" x86
  ) ELSE (
    set TARGET_SYSTEM="Visual Studio 9 Win64"
    call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall" x64
  )
) 
IF %DEVENV%==vs2010 (
  IF %PLATFORM%==Win32 (
    set TARGET_SYSTEM="Visual Studio 10 Win32"
    call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall" x86
  ) ELSE (
    set TARGET_SYSTEM="Visual Studio 10 Win64"
    call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall" x64
  )
)

IF %DEVENV%==vs2015 (
  IF %PLATFORM%==Win32 (
    set TARGET_SYSTEM="Visual Studio 14 Win32"
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" x86
  ) ELSE (
    set TARGET_SYSTEM="Visual Studio 14 Win64"
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" x64
  )
)

set DEPLIBDIR=%OSSIM_DEPENDENCIES%/lib
set DEPINCDIR=%OSSIM_DEPENDENCIES%/include

:: Here are the cmake options. Everything is about setting variables except the 
:: the last line (the business end). Make sure that source path points to the
:: ossim_package_support/cmake directory containing the top-level CMakeLists.txt
rem set CMAKE_PARAMETERS=^
rem -D%PLATFORM%_USE_MP=ON ^
rem -DBUILD_LIBRARY_DIR=lib ^
rem -DBUILD_OSSIM=ON ^
rem -DBUILD_OSSIM_MPI_SUPPORT=0 ^
rem -DBUILD_RUNTIME_DIR=bin ^
rem -DBUILD_SHARED_LIBS=ON ^
rem -DOPENTHREADS_LIBRARY=%DEPLIBDIR%/OpenThreadsx64.lib ^
rem -DTIFF_LIBRARY=%DEPLIBDIR%/libtiff_i.lib ^
rem -DTIFF_INCLUDE_DIR=%DEPINCDIR%/tiff ^
rem -DPROJ4_LIBRARY=%DEPLIBDIR%/proj.lib ^
rem -DPODOFO_INCLUDE_DIR=%DEPINCDIR% ^
rem -DPODOFO_LIBRARY=%DEPLIBDIR%/podofo.lib ^
rem -DEXPAT_INCLUDE_DIR=%DEPINCDIR%/expat ^
rem -DEXPAT_LIBRARY=%DEPLIBDIR%/podofo.lib ^
rem -DGEOTIFF_LIBRARY=%DEPLIBDIR%/geotiff_i.lib ^
rem -DGEOTIFF_INCLUDE_DIR=%DEPINCDIR%/geotiff ^
rem -DFFTW3_LIBRARY=%DEPLIBDIR%/libfftw3-3.lib ^
rem -DJPEG_LIBRARY=%DEPLIBDIR%/libjpeg.lib ^
rem -DJPEG_INCLUDE_DIR=%DEPINCDIR%/jpeg8a ^
rem -DFREETYPE_LIBRARY=%DEPLIBDIR%/freetype237.lib ^
rem -DFREETYPE_INCLUDE_DIRS=%DEPINCDIR%/freetype ^
rem -DZLIB_LIBRARY=%DEPLIBDIR%/zlib.lib ^
rem -DZLIB_INCLUDE_DIR=%DEPINCDIR%/zlib ^
rem -DOSSIM_COMPILE_WITH_FULL_WARNING=ON ^
rem -DOSSIM_DEPENDENCIES=%OSSIM_DEPENDENCIES%^
rem -DBUILD_SHARED_LIBS=ON ^
rem %OSSIM_BUILD_DIR%

set CMAKE_PARAMETERS=^
-DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
-DOSSIM_DEV_HOME=%OSSIM_DEV_HOME% ^
-D%PLATFORM%_USE_MP=ON ^
-DBUILD_LIBRARY_DIR=lib ^
-DBUILD_OMS=%BUILD_OMS% ^
-DBUILD_CNES_PLUGIN=%BUILD_CNES_PLUGIN% ^
-DBUILD_GEOPDF_PLUGIN=%BUILD_GEOPDF_PLUGIN% ^
-DBUILD_GDAL_PLUGIN=%BUILD_GDAL_PLUGIN% ^
-DBUILD_HDF5_PLUGIN=%BUILD_HDF5_PLUGIN% ^
-DBUILD_KAKADU_PLUGIN=%BUILD_KAKADU_PLUGIN% ^
-DKAKADU_ROOT_SRC=%KAKADU_ROOT_SRC% ^
-DKAKADU_AUX_LIBRARY=%KAKADU_AUX_LIBRARY% ^
-DKAKADU_LIBRARY=%KAKADU_LIBRARY% ^
-DBUILD_KML_PLUGIN=%BUILD_KML_PLUGIN% ^
-DBUILD_MRSID_PLUGIN=%BUILD_MRSID_PLUGIN% ^
-DMRSID_DIR=%MRSID_DIR% ^
-DOSSIM_PLUGIN_LINK_TYPE=SHARED ^
-DBUILD_OPENCV_PLUGIN=%BUILD_OPENCV_PLUGIN% ^
-DBUILD_OPENJPEG_PLUGIN=%BUILD_OPENJPEG_PLUGIN% ^
-DBUILD_PDAL_PLUGIN=5BUILD_PDAL_PLUGIN% ^
-DBUILD_PNG_PLUGIN=%BUILD_PNG_PLUGIN% ^
-DBUILD_SQLITE_PLUGIN=%BUILD_SQLITE_PLUGIN% ^
-DBUILD_OSSIM_VIDEO=%BUILD_OSSIM_VIDEO% ^
-DBUILD_OSSIM_GUI=%BUILD_OSSIM_GUI% ^
-DBUILD_OSSIM_WMS=%BUILD_OSSIM_WMS% ^
-DBUILD_OSSIM_PLANET=%BUILD_OSSIM_PLANET% ^
%CMAKE_DIR%

:: Finally the cmake commands themselves:
echo TARGET_SYSTEM = %TARGET_SYSTEM%
IF %GEN_TYPE%==IDE (
  echo Generating IDE solution and project files...
  cmake -G %TARGET_SYSTEM% %CMAKE_PARAMETERS%
) ELSE (
  echo Generating nmake makefiles...
  cmake -G "NMake Makefiles" %CMAKE_PARAMETERS%
)

:: Restore environment
popd
popd
popd
popd
set OSSIM_DEV_HOME=%OSSIM_DEV_HOME:/=\%
set OSSIM_DEPENDENCIES=%OSSIM_DEPENDENCIES:/=\%
echo Successfully configured build environment.
goto FINISHED

:ABORTED_END
echo Script terminated prematurely...

:FINISHED
pause
