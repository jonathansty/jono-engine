@echo off
echo Pulling latest submodules...
git submodule update --init --recursive



mkdir build
pushd build

REM debug dependencies
echo Installing debug dependencies...
conan install .. -s build_type=Debug -o *:shared=False -if conan_debug 
echo Done.

REM release dependencies
echo Installing release dependencies...
conan install .. -s build_type=Release -o *:shared=False -if conan_release 
echo Done.

echo Generating RTTR build metadata
cmake -B ../External/rttr/build -G "Visual Studio 16 2019" ../External/rttr
echo Done.

popd
echo Invoking Sharpmake.
call "sharpmake.cmd"
