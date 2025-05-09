@echo off
setlocal enabledelayedexpansion

:: Config
set SERVER=http://192.168.56.1:8000
set LIST_FILES=%SERVER%/list_files
set DOWNLOAD=%SERVER%/download
set DOWNLOAD_DIR=C:\Downloads

:: Step 1: Fetch file list (JSON array)
echo Fetching file list...
curl -s %LIST_FILES% > raw_file_list.txt

:: Step 2: Clean JSON -> one file path per line (strip ../client/)
echo Cleaning JSON file list...
del file_list.txt >nul 2>&1

for /f "delims=" %%A in (raw_file_list.txt) do (
    set line=%%A
    setlocal enabledelayedexpansion
    set line=!line:[=!
    set line=!line:]=!
    :: Loop over comma-separated items
    for %%B in (!line!) do (
        set item=%%~B
        set item=!item:,=!
        set item=!item:"=!
        set item=!item:../client/=!
        echo !item!>>file_list.txt
    )
    endlocal
)


:: Step 3: Create base download directory
if not exist "%DOWNLOAD_DIR%" mkdir "%DOWNLOAD_DIR%"

:: Step 4: Download each file
for /f "usebackq delims=" %%F in ("file_list.txt") do (
    set FILE=%%F
    set FILE=!FILE:/=\!

    echo.
    echo Downloading: !FILE!

    set LOCAL_PATH=%DOWNLOAD_DIR%\!FILE!
    for %%D in ("!LOCAL_PATH!") do set DIR_PATH=%%~dpD

    if not exist "!DIR_PATH!" mkdir "!DIR_PATH!"

    curl -s "%DOWNLOAD%/%%F" -o "!LOCAL_PATH!"
)

echo.

xcopy /Y /I "C:\Downloads\*" ".\"
