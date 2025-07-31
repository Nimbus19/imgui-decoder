@echo off
chcp 65001
setlocal enabledelayedexpansion
@REM ==============================================================================
@REM Copy and rename imgui examples
@REM
@REM Author : Wythe
@REM ==============================================================================

@REM ------------------------------------------------------------------------------
@REM Set variables
set "source_folder=..\..\third_party\imgui\examples"
set "destination_folder=..\platforms"
set "project_str=decoder"
@REM ------------------------------------------------------------------------------

:main
	@REM Copy Android example
	call :copy_and_rename_android_example "example_android_opengl3" "%project_str%_android_opengl3"

	@REM Copy iOS example
	call :copy_and_rename_apple_example "example_apple_metal" "%project_str%_apple_metal"

	@REM Copy Windows example
	call :copy_and_rename_windows_example "example_win32_directx11" "%project_str%_win32_directx11"
	call :copy_and_rename_windows_example "example_win32_directx12" "%project_str%_win32_directx12"
	call :copy_and_rename_visual_studio_solution

	Pause
	exit /b 0

:copy_and_rename_android_example
	set "example_name=%~1"
	set "new_name=%~2"
	echo Copying: %example_name%
	call :copy_folder "%example_name%\" "%new_name%\" 
	call :replace_path "%new_name%\CMakeLists.txt" 
	echo rootProject.name='imgui_%project_str%'>>"%destination_folder%\%new_name%\android\settings.gradle"
	call :replace_proj_name "%new_name%\main.cpp"
	call :replace_proj_name "%new_name%\CMakeLists.txt"
	call :replace_proj_name "%new_name%\android\app\build.gradle"
	call :replace_proj_name "%new_name%\android\app\src\main\AndroidManifest.xml"
	call :replace_proj_name "%new_name%\android\app\src\main\java\MainActivity.kt"
	call :fix_android_studio_project "%new_name%"
	exit /b 0


:copy_and_rename_apple_example
	set "example_name=%~1"
	set "new_name=%~2"
	echo Copying: %example_name%
	call :copy_folder "%example_name%\" "%new_name%\" 
	call :rename_file "%new_name%\%example_name%.xcodeproj" "%new_name%.xcodeproj"
	call :replace_path "%new_name%\%new_name%.xcodeproj\project.pbxproj"
	call :replace_proj_name "%new_name%\Makefile"
	call :replace_proj_name "%new_name%\%new_name%.xcodeproj\project.pbxproj"
	exit /b 0

:copy_and_rename_windows_example
	set "example_name=%~1"
	set "new_name=%~2"
	echo Copying: %example_name%
	call :copy_folder "%example_name%\" "%new_name%\"
	call :rename_file "%new_name%\%example_name%.vcxproj" "%new_name%.vcxproj"
	call :rename_file "%new_name%\%example_name%.vcxproj.filters" "%new_name%.vcxproj.filters"
	call :replace_path "%new_name%\build_win32.bat" 
	call :replace_path "%new_name%\%new_name%.vcxproj" 
	call :replace_path "%new_name%\%new_name%.vcxproj.filters"
	call :replace_proj_name "%new_name%\build_win32.bat"
	call :replace_proj_name "%new_name%\%new_name%.vcxproj"
	exit /b 0

:copy_and_rename_visual_studio_solution
	call :copy_file "imgui_examples.sln" "imgui_%project_str%.sln"
	call :copy_file "README.txt" "README.txt"
	call :replace_proj_name "imgui_%project_str%.sln"
	exit /b 0

:copy_folder
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	xcopy /s /e /y /i /q /r /h "%src%" "%dst%"
	exit /b 0

:copy_file
	set "src=%source_folder%\%~1"
	set "dst=%destination_folder%\%~2"
	copy /y "%src%" "%dst%" >nul 2>&1
	echo 1 File(s) copied
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

:replace_proj_name
	set "target=%destination_folder%\%~1"
	for /f %%i in ('powershell -Command "('decoder').Substring(0,1).ToUpper() + ('decoder').Substring(1)"') do set "project_str_upper=%%i"
	@REM replace "example" with %project_str%
	powershell -Command "(Get-Content '%target%') -creplace 'example', '%project_str%' | Set-Content '%target%'"
	@REM replace "Example" with %project_str_upper%
	powershell -Command "(Get-Content '%target%') -creplace 'Example', '%project_str_upper%' | Set-Content '%target%'"
	exit /b 0

:fix_android_studio_project
	@REM Fix failed to build issue in Android Studio
	set "target=%destination_folder%\%~1"
	@REM Create gradle-wrapper.properties
	echo Creating: gradle-wrapper.properties
	mkdir "%target%\android\gradle\wrapper" 2>nul
	echo distributionBase=GRADLE_USER_HOME> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo distributionPath=wrapper/dists>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo distributionUrl=https\://services.gradle.org/distributions/gradle-8.10-bin.zip>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo networkTimeout=10000>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo validateDistributionUrl=true>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo zipStoreBase=GRADLE_USER_HOME>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	echo zipStorePath=wrapper/dists>> "%target%\android\gradle\wrapper\gradle-wrapper.properties"
	@REM Create gradle.properties
	echo Creating: gradle.properties
	echo org.gradle.jvmargs=-Xmx1536m> "%target%\android\gradle.properties"
	echo android.useAndroidX=true>> "%target%\android\gradle.properties"
	echo android.enableJetifier=true>> "%target%\android\gradle.properties"
	@REM repalce "/.." with "/../../../../../.." in CmakeLists.txt
	powershell -Command "(Get-Content '%target%\CMakeLists.txt') -replace '}/\.\.', '}/../../../../../..' | Set-Content '%target%\CMakeLists.txt'"
	@REM repalce "../../CMakeLists.txt" with "src/main/cpp/CMakeLists.txt" in gradle.build
	powershell -Command "(Get-Content '%target%\android\app\build.gradle') -replace '\.\./\.\./CMakeLists', 'src/main/cpp/CMakeLists' | Set-Content '%target%\android\app\build.gradle'"
	@REM Move CmakeLists.txt to "android/app/src/main/cpp" folder
	mkdir "%target%\android\app\src\main\cpp" 2>nul
	move "%target%\CMakeLists.txt" "%target%\android\app\src\main\cpp\" >nul 2>&1
	move "%target%\main.cpp" "%target%\android\app\src\main\cpp\" >nul 2>&1

	echo Notice: you need to manually change gradle JDK version to 11 in Android Studio settings.
	exit /b 0