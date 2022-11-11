@echo off
setlocal EnableDelayedExpansion 

set VERBOSITY=q
set CLP_PARAMS=Summary,ShowCommandLine,PerformanceSummary

set CONFIGS=Debug Release
set LINKAGE=Lib Dll
set SLN_PATH=%~dp0/../Build/jono-engine_vs2022.sln

(for %%c in (%CONFIGS%) do (
(for %%l in (%LINKAGE%) do (
	set CONFIG="%%c %%l"
	set BUILD_PARAMS= /t:rebuild  -v:%VERBOSITY% -clp:%CLP_PARAMS%
	echo ====================================================
	echo Building !CONFIG!
	echo ====================================================
	cmd /c "Build.cmd %SLN_PATH% !BUILD_PARAMS! -p:Configuration=!CONFIG!
))
))