pushd %~dp0
%~dp0/../SharpMake/Sharpmake.application.exe /sources('../main.sharpmake.cs') %* 
popd
pause