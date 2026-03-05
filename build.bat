@echo off
setlocal

set BASH="C:\Program Files\Git\bin\bash.exe"
set REPO=/c/Users/Chen/Desktop/git_repo/refloat

if not exist "%~dp0toolchain\bin\arm-none-eabi-gcc.exe" (
    echo [ERROR] Toolchain not found at %~dp0toolchain
    echo        Please run install.sh first:
    echo        %BASH% install.sh
    exit /b 1
)

%BASH% -c "cd %REPO% && make OLDVT=1 %*"
