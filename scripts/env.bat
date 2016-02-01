@echo off
setlocal enabledelayedexpansion
set SCRIPT_DIR="%~dp0"
if not defined OSSIM_DEV_HOME (
   pushd %SCRIPT_DIR%..\..
   set OSSIM_DEV_HOME=!CD!
   popd
)
if not defined VISUAL_STUDIO_VERSION ( 
    set VISUAL_STUDIO_VERSION=14
)
if not defined OSSIM_INSTALL_PREFIX ( 
    set OSSIM_INSTALL_PREFIX=!OSSIM_DEV_HOME!install
)

call "C:\Program Files (x86)\Microsoft Visual Studio %VISUAL_STUDIO_VERSION%\VC\vcvarsall.bat" amd64
