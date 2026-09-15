@echo off
echo ========================================================
echo Generating Visual Studio 2022 Solution for UE 5.5...
echo ========================================================
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%~dp0AegisCombat.uproject" -game -engine -progress
pause
