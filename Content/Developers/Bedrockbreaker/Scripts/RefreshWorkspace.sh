#!/bin/bash

echo "Unimplemented!"
exit 1

PROJECT_NAME="Ganymede"
UE_PATH="/this/needs/testing"
DEV_PATH="$(pwd)/Content/Developers/Bedrockbreaker"

echo "Regenerating UE5 workspace files..."
"$UE_PATH/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe" -projectfiles -project="$(PWD)/$PROJECT_NAME.uproject" -game -rocket -progress -engine -VSCode
#"$UE_PATH/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" -project="$(PWD)/$PROJECT_NAME.uproject"

echo "Restoring settings..."
cp -f "$DEV_PATH/Config/c_cpp_properties.json" ".vscode/c_cpp_properties.json"

echo "Done!"