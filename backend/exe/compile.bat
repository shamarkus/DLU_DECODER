set currDir=%~dp0
set parentDir="%currDir%\.."

g++.exe %parentDir%\src\main.cpp %parentDir%\src\logDecoderClass.cpp %parentDir%\src\helperFunctions.cpp -o %currDir%\DLULogDecoder.exe
pause
