pushd %~dp0

mkdir "../build"
pushd "../build"

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
%~dp0/GenerateSolution.cmd
