@echo off
setlocal

set PROJECT_NAME=Memento
set UE_PATH=C:\UE5_5
set DEV_PATH=%CD%\Content\Developers\Bedrockbreaker

echo Regenerating UE5 workspace files...
::%UE_PATH%\Engine\Build\BatchFiles\GenerateProjectFiles.bat -project="%CD%\%PROJECT_NAME%.uproject"
%UE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe -projectfiles -project="%CD%\%PROJECT_NAME%.uproject" -game -rocket -progress -engine -VSCode

echo Restoring settings...
copy /Y "%DEV_PATH%\Config\c_cpp_properties.json" ".vscode\c_cpp_properties.json"

echo Done!
endlocal
::https://gist.github.com/boocs/f63a4878156295b6e854cac68672f305