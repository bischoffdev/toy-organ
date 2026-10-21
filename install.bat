@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Laxersaz Organ - Windows Installer
echo ===================================================
echo.

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

:: Define target directories
set "VST3_DIR=%COMMONPROGRAMFILES%\VST3"
set "LV2_DIR=%COMMONPROGRAMFILES%\LV2"
set "STANDALONE_DIR=%LOCALAPPDATA%\Programs\LaxersazOrgan"
set "SOUNDSETS_DIR=%USERPROFILE%\Music\LaxersazOrgan\Soundsets"

:: Check uninstall flag
if "%~1"=="--uninstall" (
    echo Uninstalling Laxersaz Organ...
    if exist "%VST3_DIR%\Laxersaz Organ.vst3" rmdir /s /q "%VST3_DIR%\Laxersaz Organ.vst3"
    if exist "%LV2_DIR%\Laxersaz Organ.lv2" rmdir /s /q "%LV2_DIR%\Laxersaz Organ.lv2"
    if exist "%STANDALONE_DIR%" rmdir /s /q "%STANDALONE_DIR%"
    echo [OK] Removed VST3, LV2, and Standalone.
    echo [INFO] Custom soundsets in '%SOUNDSETS_DIR%' were left intact.
    echo Uninstallation complete!
    goto :end
)

:: Locate artifacts (release layout or CMake build layout)
set "VST3_SRC="
if exist "%SCRIPT_DIR%\Laxersaz Organ.vst3" set "VST3_SRC=%SCRIPT_DIR%\Laxersaz Organ.vst3"
if not defined VST3_SRC if exist "%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\VST3\Laxersaz Organ.vst3" set "VST3_SRC=%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\VST3\Laxersaz Organ.vst3"

set "LV2_SRC="
if exist "%SCRIPT_DIR%\Laxersaz Organ.lv2" set "LV2_SRC=%SCRIPT_DIR%\Laxersaz Organ.lv2"
if not defined LV2_SRC if exist "%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\LV2\Laxersaz Organ.lv2" set "LV2_SRC=%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\LV2\Laxersaz Organ.lv2"

set "EXE_SRC="
if exist "%SCRIPT_DIR%\Laxersaz Organ.exe" set "EXE_SRC=%SCRIPT_DIR%\Laxersaz Organ.exe"
if not defined EXE_SRC if exist "%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\Standalone\Laxersaz Organ.exe" set "EXE_SRC=%SCRIPT_DIR%\build\ToyOrgan_artefacts\Release\Standalone\Laxersaz Organ.exe"

set "SOUNDSETS_SRC="
if exist "%SCRIPT_DIR%\Soundsets" set "SOUNDSETS_SRC=%SCRIPT_DIR%\Soundsets"
if not defined SOUNDSETS_SRC if exist "%SCRIPT_DIR%\..\Soundsets" set "SOUNDSETS_SRC=%SCRIPT_DIR%\..\Soundsets"

:: Install / Update VST3
if defined VST3_SRC (
    if not exist "%VST3_DIR%" mkdir "%VST3_DIR%" 2>nul
    if exist "%VST3_DIR%\Laxersaz Organ.vst3" rmdir /s /q "%VST3_DIR%\Laxersaz Organ.vst3"
    xcopy /e /i /y /q "%VST3_SRC%" "%VST3_DIR%\Laxersaz Organ.vst3" >nul
    echo [OK] Installed/Updated VST3 -^> %VST3_DIR%\Laxersaz Organ.vst3
) else (
    echo [SKIP] VST3 not found.
)

:: Install / Update LV2
if defined LV2_SRC (
    if not exist "%LV2_DIR%" mkdir "%LV2_DIR%" 2>nul
    if exist "%LV2_DIR%\Laxersaz Organ.lv2" rmdir /s /q "%LV2_DIR%\Laxersaz Organ.lv2"
    xcopy /e /i /y /q "%LV2_SRC%" "%LV2_DIR%\Laxersaz Organ.lv2" >nul
    echo [OK] Installed/Updated LV2  -^> %LV2_DIR%\Laxersaz Organ.lv2
) else (
    echo [SKIP] LV2 not found.
)

:: Install / Update Standalone
if defined EXE_SRC (
    if not exist "%STANDALONE_DIR%" mkdir "%STANDALONE_DIR%" 2>nul
    copy /y "%EXE_SRC%" "%STANDALONE_DIR%\Laxersaz Organ.exe" >nul
    echo [OK] Installed/Updated App  -^> %STANDALONE_DIR%\Laxersaz Organ.exe
) else (
    echo [SKIP] Standalone executable not found.
)

:: Install / Update Factory Soundsets (preserves custom user folders)
if defined SOUNDSETS_SRC (
    if not exist "%SOUNDSETS_DIR%" mkdir "%SOUNDSETS_DIR%" 2>nul
    for /d %%D in ("%SOUNDSETS_SRC%\*") do (
        set "SET_NAME=%%~nxD"
        if not exist "%SOUNDSETS_DIR%\!SET_NAME!" mkdir "%SOUNDSETS_DIR%\!SET_NAME!" 2>nul
        xcopy /e /i /y /q "%%D" "%SOUNDSETS_DIR%\!SET_NAME!" >nul
    )
    echo [OK] Updated factory soundsets in -^> %SOUNDSETS_DIR%
    echo      (Any custom soundsets you added remain untouched)
) else (
    echo [SKIP] Soundsets folder not found.
)

echo.
echo Installation successful!
echo Restart your DAW to load the updated Laxersaz Organ plugin.
echo.

:end
pause
