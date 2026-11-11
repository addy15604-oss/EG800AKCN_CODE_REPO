@echo off

setlocal EnableDelayedExpansion
set CURDIR=%~dp0
set target=%1

set /P APP_MODE=<..\..\ql-config\config\APP_MODE.ini
if "%target%" == "clean" (
	gnumake -f Makefile clean TOP_DIR=" %CURDIR%\"
) else (
	echo  %APP_MODE%|findstr "RAM" >nul
	if !errorlevel! neq 1 (
		set APP_INPSRAM=ENABLE  
		
	) else (
	set APP_INPSRAM=DISABLE
	
	)
	gnumake -f Makefile TOP_DIR=" %CURDIR%\"
)
exit /b 0
