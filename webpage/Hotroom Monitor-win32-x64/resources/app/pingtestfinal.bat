rem Ping Test Final 
set /a x=0 
:start
echo Start > pingloss.txt
echo Start > badips.txt
echo New Test %x% >> pinglossall.txt
echo Failed IP----Date---------Time >> pinglossall.txt
FOR /F %%i IN (C:\Users\Ejnar-2\Documents\Arduino\HotroomMonitoring\webpage\iplist.txt) DO CALL :firstpass %%i

FOR /F "tokens=1 skip=1" %%h IN (C:\Users\Ejnar-2\Documents\Arduino\HotroomMonitoring\webpage\pingloss.txt) DO CALL :secondpass %%h  
set /a x+=1
timeout 900
goto start

:firstpass
FOR /f "tokens=3 delims=," %%B IN ('ping -n 1 %1 ^|find "Lost = 1"') DO echo %1 %date% %time% >>pingloss.txt
goto :eof
:secondpass
FOR /f "tokens=3 delims=," %%B IN ('ping -n 1 %1 ^|find "Lost = 1"') DO echo %1 %date% %time% FAIL >> pinglossall.txt 
FOR /f "tokens=3 delims=," %%B IN ('ping -n 1 %1 ^|find "Lost = 1"') DO echo %1>> badips.txt