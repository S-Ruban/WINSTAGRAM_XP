@echo off
setlocal enabledelayedexpansion

:: Config
set SERVER=http://192.168.56.1:8000
set UPLOAD=%SERVER%/upload
set DELETE=%SERVER%/delete_all

echo ===========================================================
echo  Deleting existing files on the server: %DELETE%
echo ===========================================================
curl -s -X DELETE %DELETE%
echo.

echo ===========================================================
echo  Uploading all non-binary (text) files from current folder
echo ===========================================================

for %%F in (*.*) do (
    REM Check for binary characters
    findstr /R /C:"[^\x20-\x7E\t]" "%%F" >nul
    if errorlevel 0 (
        echo Uploading %%F ...
        curl -s -X POST -F "file=@%%F" %UPLOAD%
    ) else (
        echo Skipping binary file: %%F
    )
    echo:
)

echo.
