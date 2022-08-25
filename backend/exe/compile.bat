set currDir=%~dp0
set parentDir="%currDir%\.."

g++.exe -g %parentDir%\src\main.cpp %parentDir%\src\logDecoderClass.cpp %parentDir%\src\logParser.cpp %parentDir%\src\helperFunctions.cpp -o %currDir%\DLULogDecoder.exe
pause
