# ESAP

ESAP stands for the Extremely Potent and Sophisticated (Trojan)

How it works:

1. Rubber ducky USB is plugged into the user's computer.
2. The rubber ducky runs a powershell script which downloads ESAP.exe into %TEMP% as esp.exe and runs it.
3. esp.exe (ESAP.exe) downloads GoogleUpdateBroker.exe from the server into %APPDATA%/GoogIe
4. GoogleUpdateBroker.exe is added to startup for persistence and run.

## Features: FILESYSTEM

- [x] Change directory
- [x] List directory
- [x] Download file from user's computer
- [x] Upload file to user's computer

## Features: TROLL

- [x] Block screen
- [x] Block input
- [x] Playsound
- [x] Kill internet
- [x] Kill PC

## Features: SPY

- [x] Grab public IP
- [x] Record audio
- [x] Get screenshot
- [x] Get webcam image
- [x] Get clipboard
- [x] Get history
- [ ] Get cookies

## Features: OTHER

- [x] Reverse shell
- [x] Self destruct (uninstall from user)
- [ ] Disable taskmgr
- [ ] Disable apps

## Disclaimer
This repository is for educational purposes only, and is used as a proof-of-concept.

I have no intention to use this to cause harm or damage to others.
