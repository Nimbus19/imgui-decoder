@echo off
chcp 65001
setlocal enabledelayedexpansion
@REM ==============================================================================
@REM Copy and rename imgui example
@REM
@REM Author : Wythe
@REM ==============================================================================

@REM ------------------------------------------------------------------------------
@REM Set variables
set "source_folder=..\third_party\imgui\examples"
set "destination_folder=."
set "replace_str=decoder"
@REM ------------------------------------------------------------------------------

:main
	@REM Copy Android example
	call :copy_and_rename_android_example "example_android_opengl3" "%replace_str%_android_opengl3"

	@REM Copy iOS example
	call :copy_and_rename_apple_example "example_apple_metal" "%replace_str%_apple_metal"

	@REM Copy Windows example
	call :copy_and_rename_windows_example "example_win32_directx11" "%replace_str%_win32_directx11"
	call :copy_and_rename_windows_example "example_win32_directx12" "%replace_str%_win32_directx12"
	call :copy_file "imgui_examples.sln" "windows\imgui_%replace_str%.sln"
	call :copy_file "README.txt" "windows\"
	call :replace_file "windows\imgui_%replace_str%.sln"

	exit /b 0

:copy_and_rename_android_example
	set "example_name=%~1"
	set "new_name=%~2"
	call :copy_folder "%example_name%\" "android\%new_name%\" 
	call :replace_path "android\%new_name%\CMakeLists.txt" 
	echo rootProject.name='imgui_%replace_str%'>>"%destination_folder%\android\%new_name%\android\settings.gradle"
	call :replace_file "android\%new_name%\main.cpp"
	call :replace_file "android\%new_name%\CMakeLists.txt"
	call :replace_file "android\%new_name%\android\app\build.gradle"
	call :replace_file "android\%new_name%\android\app\src\main\AndroidManifest.xml"
	call :replace_file "android\%new_name%\android\app\src\main\java\MainActivity.kt"
	exit /b 0


:copy_and_rename_apple_example
	set "example_name=%~1"
	set "new_name=%~2"
	call :copy_folder "%example_name%\" "apple\%new_name%\" 
	call :rename_file "apple\%new_name%\%example_name%.xcodeproj" "%new_name%.xcodeproj"
	call :replace_path "apple\%new_name%\%new_name%.xcodeproj\project.pbxproj"
	call :replace_file "apple\%new_name%\Makefile"
	call :replace_file "apple\%new_name%\%new_name%.xcodeproj\project.pbxproj"
	exit /b 0

:copy_and_rename_windows_example
	set "example_name=%~1"
	set "new_name=%~2"
	call :copy_folder "%example_name%\" "windows\%new_name%\"
	call :rename_file "windows\%new_name%\%example_name%.vcxproj" "%new_name%.vcxproj"
	call :rename_file "windows\%new_name%\%example_name%.vcxproj.filters" "%new_name%.vcxproj.filters"
	call :replace_path "windows\%new_name%\build_win32.bat" 
	call :replace_path "windows\%new_name%\%new_name%.vcxproj" 
	call :replace_path "windows\%new_name%\%new_name%.vcxproj.filters"
	call :replace_file "windows\%new_name%\build_win32.bat"
	call :replace_file "windows\%new_name%\%new_name%.vcxproj"
	exit /b 0

:copy_folder
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	xcopy /s /e /y /i /q /r /h "%src%" "%dst%"
	exit /b 0

:copy_file
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	echo F | xcopy /y /q /r /f "%src%" "%dst%"
	exit /b 0

:rename_file
	set "src=%destination_folder%\%~1"
	set "dst=%~2"
	ren "%src%" "%dst%"
	exit /b 0

:replace_path
	set "target=%destination_folder%\%~1"	
	@REM repalce "../.." with "../../../third_party/imgui" 
	powershell -Command "(Get-Content '%target%') -replace '\.\./\.\.', '../../../third_party/imgui' | Set-Content '%target%'"
	@REM repalce "..\.." with "..\..\..\third_party\imgui" 
	powershell -Command "(Get-Content '%target%') -replace '\.\.\\\.\.', '..\..\..\third_party\imgui' | Set-Content '%target%'"
	exit /b 0

:replace_file
	set "target=%destination_folder%\%~1"
	@REM replace "example" with %replace_str%
	powershell -Command "(Get-Content '%target%') -replace 'example', '%replace_str%' | Set-Content '%target%'"
	exit /b 0
