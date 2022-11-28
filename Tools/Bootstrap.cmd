@echo off
setlocal ENABLEDELAYEDEXPANSION
echo ======================================
echo (1/6) - Pulling latest submodules...
echo ======================================
git submodule update --init --recursive

pushd %~dp0

mkdir "../build"
pushd "%~dp0/../build"

REM debug dependencies
echo ======================================
echo (2/6) - Installing debug dependencies...
echo ======================================
conan install .. -s build_type=Debug -o *:shared=False -if conan_debug -pr ../profile_debug
echo Done.

REM release dependencies
echo ======================================
echo (3/6) Installing release dependencies...
echo ======================================
conan install .. -s build_type=Release -o *:shared=False -if conan_release -pr ../profile_release
echo Done.

echo ======================================
echo (4/6) Generating RTTR build metadata
echo ======================================
set CMAKE_GENERATOR="Visual Studio 17 2022"
IF "%1"=="CI" (
    set CMAKE_GENERATOR="Ninja"
)
echo Using %CMAKE_GENERATOR%
cmake -B ../External/rttr/build -G %CMAKE_GENERATOR% ../External/rttr
echo Done.

echo ======================================
echo (5/6) Building DirectXTK
echo ======================================
call %~dp0/Build.cmd ../External/DirectXTK/DirectXTK_Desktop_2022.sln -p:Configuration=Debug -v:m
call %~dp0/Build.cmd ../External/DirectXTK/DirectXTK_Desktop_2022.sln -p:Configuration=Release -v:m


popd
popd
echo ======================================
echo (6/6) - Invoking Sharpmake.
echo ======================================
"%~dp0/GenerateSolution.cmd"
pause
