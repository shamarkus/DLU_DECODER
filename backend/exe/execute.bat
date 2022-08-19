set currDir=%~dp0
set parentDir="%currDir%\..\\"

call "%~1"

%currDir%\DLULogDecoder.exe %parentDir% %files% %logType% %times% %params% %outputName%

pause
