@echo off
setlocal ENABLEEXTENSIONS

curl -s http://192.168.56.1:8000/list_files -o file_list.txt

for /f "tokens=*" %%f in (file_list.txt) do (

    echo Downloading %%f...

    for %%d in ("%%f") do (
        if not "%%~dpd"=="" (
            if not exist "%%~dpd" mkdir "%%~dpd"
        )
    )

    curl -s http://192.168.56.1:8000/download/%%f -o "%%f"
)