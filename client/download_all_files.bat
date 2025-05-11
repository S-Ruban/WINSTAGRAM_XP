@echo off
setlocal ENABLEEXTENSIONS

:: Step 1: Download raw JSON list
curl -s http://192.168.56.1:8000/list_files -o file_list_raw.json

:: Step 2: Convert JSON to list manually
>file_list.txt (
    for /f "tokens=*" %%a in (file_list_raw.json) do (
        set line=%%a
        call :process_line
    )
)

:: Step 3
for /f "tokens=*" %%f in (file_list.txt) do (
    echo Downloading %%f...
    curl -s http://192.168.56.1:8000/download/%%f -o %%f
)




:process_line
:: Remove [ ] , " and ../client/ or client/
set "line=%line:[=%"
set "line=%line:]=%"
set "line=%line:,=%"
set "line=%line:\"=%"
set "line=%line:../client/=%"
set "line=%line:client/=%"
echo %line%
goto :eof
