@echo off
echo Pulling latest submodules...
git submodule update --init --recursive

pushd %~dp0

mkdir "../build"
pushd "%~dp0/../build"

REM debug dependencies
echo Installing debug dependencies...
conan install .. -s build_type=Debug -o *:shared=False -if conan_debug -pr ../profile_debug
echo Done.

REM release dependencies
echo Installing release dependencies...
conan install .. -s build_type=Release -o *:shared=False -if conan_release -pr ../profile_release
echo Done.

echo Generating RTTR build metadata
cmake -B ../External/rttr/build -G Ninja ../External/rttr
echo Done.

popd
popd
echo Invoking Sharpmake.
"%~dp0/GenerateSolution.cmd"
pause
