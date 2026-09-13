@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\cmake.exe" -S %~dp0 -B %~dp0build -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\cmake.exe" --build %~dp0build
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\ctest.exe" --test-dir %~dp0build --output-on-failure
