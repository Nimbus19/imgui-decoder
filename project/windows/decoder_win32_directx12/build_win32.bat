@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Debug
@set OUT_EXE=decoder_win32_directx12
@set INCLUDES=/I..\..\..\third_party\imgui /I..\..\..\third_party\imgui\backends /I "%WindowsSdkDir%Include\um" /I "%WindowsSdkDir%Include\shared"
@set SOURCES=main.cpp ..\..\..\third_party\imgui\backends\imgui_impl_dx12.cpp ..\..\..\third_party\imgui\backends\imgui_impl_win32.cpp ..\..\..\third_party\imgui\imgui*.cpp
@set LIBS=d3d12.lib d3dcompiler.lib dxgi.lib
mkdir Debug
cl /nologo /Zi /MD /utf-8 %INCLUDES% /D UNICODE /D _UNICODE %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%
