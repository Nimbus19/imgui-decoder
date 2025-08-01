@echo off
@REM ==============================================================================
@REM Build Project via CMake's Visual Studio Generator
@REM
@REM Author : Wythe
@REM ==============================================================================
chcp 65001
setlocal enabledelayedexpansion

@REM CMake Settings
@REM Valid ABI : Win32, x64, ARM, ARM64
set "CM_ABI=x64"
@REM Valid Config : Debug, Release, RelWithDebInfo, MinSizeRel
set "CM_CONFIG=Debug"
@REM Valid Generator : Visual Studio 16 2019, Visual Studio 17 2022
set "CM_GENERATOR=Visual Studio 16 2019"

:Main
	set "CM_WORK_DIR=..\build\%CM_ABI%\"
	set "CM_OUT_DIR=..\..\lib\%CM_ABI%\"

	call :CheckRebuild
	call :GenerateProject
	call :CompileProject
	Pause	
	endlocal
    exit /b 0
	
:CheckRebuild
	echo. | set /p answer=Rebuild (y/n)?:
	if "%answer%"=="" (
		set /p answer=
	)
	echo %answer%
	if /i "%answer%"=="y" (
		rmdir /s /q %CM_WORK_DIR% >nul 2>nul
	)
	exit /b

:GenerateProject
	mkdir %CM_WORK_DIR% >nul 2>nul
	cmake ^
		-S .. ^
		-B %CM_WORK_DIR% ^
		-G "%CM_GENERATOR%" ^
		-A %CM_ABI% ^
		-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%CM_OUT_DIR%
    exit /b

:CompileProject
	cmake --build %CM_WORK_DIR% --config %CM_CONFIG%
    exit /b