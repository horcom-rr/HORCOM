@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\cmake.exe" -S %~dp0 -B %~dp0build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\cmake.exe" --build %~dp0build
if errorlevel 1 exit /b 1
"C:\Program Files\CMake\bin\ctest.exe" --test-dir %~dp0build --output-on-failure
