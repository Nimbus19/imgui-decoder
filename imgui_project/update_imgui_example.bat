@echo off
chcp 65001
setlocal enabledelayedexpansion
@REM ==============================================================================
@REM Copy imgui example
@REM
@REM Author : Wythe
@REM ==============================================================================

@REM ------------------------------------------------------------------------------
@REM Set path
set "source_folder=..\third_party\imgui\examples"
set "destination_folder=."
@REM ------------------------------------------------------------------------------

:main
	@REM Copy Android example
	call :copy_and_rename_android_example "example_android_opengl3"

	@REM Copy iOS example
	call :copy_and_rename_apple_example "example_apple_metal"

	@REM Copy Windows example
	call :copy_and_rename_windows_example "example_win32_directx11"
	call :copy_and_rename_windows_example "example_win32_directx12"
	call :copy_file "imgui_examples.sln" "windows\imgui_examples.sln"
	call :copy_file "README.txt" "windows\"

	exit /b 0

:copy_and_rename_android_example
	set "example_name=%~1"
	call :copy_folder "%example_name%\" "android\%example_name%\" 
	call :replace_path "\android\%example_name%\CMakeLists.txt" 
	echo rootProject.name='imgui_examples'>>"%destination_folder%\android\%example_name%\android\settings.gradle"
	exit /b 0

:copy_and_rename_apple_example
	set "example_name=%~1"
	call :copy_folder "%example_name%\" "apple\%example_name%\" 
	call :replace_path "\apple\%example_name%\%example_name%.xcodeproj\project.pbxproj" 
	exit /b 0

:copy_and_rename_windows_example
	set "example_name=%~1"
	call :copy_folder "%example_name%\" "windows\%example_name%\"
	call :replace_path "\windows\%example_name%\build_win32.bat" 
	call :replace_path "\windows\%example_name%\%example_name%.vcxproj" 
	call :replace_path "\windows\%example_name%\%example_name%.vcxproj.filters" 
	exit /b 0

:copy_folder
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	xcopy /s /e /y /i /q /r /h "%src%" "%dst%"
	exit /b 0


:copy_file
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	xcopy /y /q /r "%src%" "%dst%"
	exit /b 0


:replace_path
	set "target=%destination_folder%\%~1"
	
	@REM repalce "../.." with "../../../third_party/imgui" 
	powershell -Command "(Get-Content '%target%') -replace '\.\./\.\.', '../../../third_party/imgui' | Set-Content '%target%'"
	
	@REM repalce "..\.." with "..\..\..\third_party\imgui" 
	powershell -Command "(Get-Content '%target%') -replace '\.\.\\\.\.', '..\..\..\third_party\imgui' | Set-Content '%target%'"

	exit /b 0
