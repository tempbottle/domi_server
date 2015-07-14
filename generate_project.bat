
@echo off

cd ./
if not exist "build" (md "build")
cd build
if exist "CMakeCache.txt" (del "CMakeCache.txt")
if exist "CMakeFiles" rd /S /Q "CMakeFiles"
cmake -G "Visual Studio 12 2013" --build ..

pause