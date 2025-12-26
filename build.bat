@echo off
REM Build script for Windows

echo Building test program...

REM Try different compilers in order of preference
where gcc >nul 2>&1
if %ERRORLEVEL% == 0 (
    gcc test.c -o test.exe -std=c99 -Wall
    if %ERRORLEVEL% == 0 (
        echo Build successful! Run with: test.exe
        exit /b 0
    )
)

where clang >nul 2>&1
if %ERRORLEVEL% == 0 (
    clang test.c -o test.exe -std=c99 -Wall
    if %ERRORLEVEL% == 0 (
        echo Build successful! Run with: test.exe
        exit /b 0
    )
)

where cl >nul 2>&1
if %ERRORLEVEL% == 0 (
    cl test.c /Fe:test.exe /W3
    if %ERRORLEVEL% == 0 (
        echo Build successful! Run with: test.exe
        exit /b 0
    )
)

echo Error: No C compiler found. Please install:
echo   - MinGW-w64 (gcc)
echo   - Clang
echo   - Microsoft Visual Studio (cl)
exit /b 1

