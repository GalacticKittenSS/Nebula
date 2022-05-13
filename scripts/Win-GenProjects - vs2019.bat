@echo off
pushd %~dp0\..\
call premake\premake5.exe vs2019
IF %ERRORLEVEL% NEQ 0 (
  PAUSE
)
popd