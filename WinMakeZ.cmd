@set TLPATH=D:\MCU\TelinkIoTStudio
@set PATH=%TLPATH%\bin;%TLPATH%\opt\tc32\bin;%TLPATH%\mingw\bin;%TLPATH%\opt\tc32\tc32-elf\bin;%PATH%
@set SWVER=_v0010
@del /Q .\bin\*.bin
@del /Q .\bin\*.zigbee
@del /Q .\build
set PROJECT_NAME=EM0SW1
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW1
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW1
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM0SW2
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW2
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW2
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM0SW1T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW1T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW1T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM0SW2T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW2T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW2T
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM0SW1TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW1TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW1TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM0SW2TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM1SW2TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
set PROJECT_NAME=EM8SW2TS
make -s -j clean
make -s -j VERSION_BIN=%SWVER% PROJECT_NAME=%PROJECT_NAME% POJECT_DEF="-DBOARD=BOARD_%PROJECT_NAME%"
@if not exist "bin\%PROJECT_NAME%%SWVER%.bin" goto :error
cd .\zigpy_ota
call update.cmd %SWVER%
cd ..
@exit
:error
echo "Error!"

         