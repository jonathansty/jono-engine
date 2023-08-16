@echo off


setlocal ENABLEDELAYEDEXPANSION
echo ======================================
echo (1/6) - Pulling latest submodules...
echo ======================================
git submodule update --init --recursive

pushd %~dp0
pushd ..

REM debug dependencies
REM echo ======================================
REM echo (2/6) - Installing debug dependencies...
REM echo ======================================
REM conan install .. -s build_type=Debug -o *:shared=False -if conan_debug -pr ../profile_debug
REM echo Done.
REM 
REM REM release dependencies
REM echo ======================================
REM echo (3/6) Installing release dependencies...
REM echo ======================================
REM conan install .. -s build_type=Release -o *:shared=False -if conan_release -pr ../profile_release
REM echo Done.


set CMAKE_GENERATOR="Visual Studio 17 2022"
IF "%1"=="CI" (
    set CMAKE_GENERATOR="Ninja"
)

echo Building FMT
cmake "build/fmt" -G "Visual Studio 17 2022" -A x64 -S "External/fmt" -B "build/fmt/x64" 
cmake --build "build/fmt/x64" --config debug
cmake --build "build/fmt/x64" --config release

echo Building Assimp
SET BINARIES_DIR="build/assimp/x64"
cmake "build/assimp" -G "Visual Studio 17 2022" -A x64 -S "External/assimp" -B "%BINARIES_DIR%" -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON -DASSIMP_BUILD_ZLIB=ON
cmake --build %BINARIES_DIR% --config debug
cmake --build %BINARIES_DIR% --config release

mkdir "../build"
pushd "%~dp0/../build"

echo ======================================
echo Generating RTTR build metadata
echo ======================================

echo Using %CMAKE_GENERATOR%
cmake -B ../External/rttr/build -G %CMAKE_GENERATOR% ../External/rttr
echo Done.

echo ======================================
echo (5/6) Building DirectXTK
echo ======================================
REM call %~dp0/Build.cmd ../External/DirectXTK/DirectXTK_Desktop_2022.sln -p:Configuration=Debug -v:m
REM call %~dp0/Build.cmd ../External/DirectXTK/DirectXTK_Desktop_2022.sln -p:Configuration=Release -v:m


popd
popd
echo ======================================
echo (6/6) - Invoking Sharpmake.
echo ======================================
"%~dp0/GenerateSolution.cmd"
pause
