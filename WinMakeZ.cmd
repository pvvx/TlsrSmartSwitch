@set TLPATH=D:\MCU\TelinkIoTStudio
@set PATH=%TLPATH%\bin;%TLPATH%\opt\tc32\bin;%TLPATH%\mingw\bin;%TLPATH%\opt\tc32\tc32-elf\bin;%PATH%
@set SWVER=_v0001
@del /Q .\bin\*.bin
@del /Q .\bin\*.zigbee
@del /Q .\build
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=MZSW01 POJECT_DEF="-DBOARD=BOARD_MZSW01_BL0942"
@if not exist "bin\MZSW01%SWVER%.bin" goto :error
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=MZSW02 POJECT_DEF="-DBOARD=BOARD_MZSW02_BL0937"
@if not exist "bin\MZSW02%SWVER%.bin" goto :error
cd .\zigpy_ota
call update.cmd %SWVER%
cd ..
@exit
:error
echo "Error!"

         