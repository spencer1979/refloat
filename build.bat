@echo off
setlocal

set BASH="C:\Program Files\Git\bin\bash.exe"
set REPO=/c/Users/Chen/Desktop/git_repo/refloat

if not exist "C:\tc\bin\arm-none-eabi-gcc.exe" (
    echo [ERROR] Toolchain not found at C:\tc
    echo        Please run install.sh first:
    echo        %BASH% -c "%REPO%/install.sh"
    exit /b 1
)

%BASH% -c "cd %REPO% && make OLDVT=1 %*"
