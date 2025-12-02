@echo off
set COMPILER=g++
set SOURCE_FILE=sudoku_game.cpp
set OUTPUT_EXE=sudoku.exe
echo Compiling %SOURCE_FILE% (32-bit Win32 API)...
%COMPILER% %SOURCE_FILE% -o %OUTPUT_EXE% -m32 -Wl,-subsystem,windows -luser32 -lgdi32
IF %ERRORLEVEL% NEQ 0 (
echo.
echo [ERROR] Compilation failed.
echo Check the console output for details.
pause
) ELSE (
echo.
echo [SUCCESS] %OUTPUT_EXE% created.
)
