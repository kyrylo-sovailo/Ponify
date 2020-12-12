@echo off
if "%1"=="" (
	call :help
	exit /b
)
if not "%2"=="" (
	call :help
	exit /b
)
if "%1"=="x86" (
	set BUILD_ARCHITECTURE=x86
	call :compile
	exit /b
)
if "%1"=="amd64" (
	set BUILD_ARCHITECTURE=amd64
	call :compile
	exit /b
)
call :help
exit /b

:compile
	if not defined BUILD_VSDEVCMD_PATH (
		if exist vsdevcmd (
			set /p BUILD_VSDEVCMD_PATH=<vsdevcmd
			set BUILD_VSDEVCMD_ARCHITECTURE=BUILD_ARCHITECTURE
			call "%%BUILD_VSDEVCMD_PATH%%" -arch=%BUILD_ARCHITECTURE%
		) else (
			call :enter
		)
	)
		
	if defined BUILD_VSDEVCMD_PATH (
		if not "%BUILD_VSDEVCMD_ARCHITECTURE%"=="%BUILD_ARCHITECTURE%" (
			echo Please restart command prompt to compile the program with different architecture
		) else (
			cl ponify.c
		)
	)
	exit /b

:enter
	SET BUILD_REPOSITORY_DIRECTORY=%CD%
	for /f %%d in ('wmic logicaldisk get name ^| find ":"') do (
		if exist "%%d\Program Files (x86)\Microsoft Visual Studio" (
			cd /d "%%d\Program Files (x86)\Microsoft Visual Studio"
			for /f "delims=" %%v in ('dir /b /s /a:-d "vsdevcmd.bat"') do (
				cd /d "%BUILD_REPOSITORY_DIRECTORY%"
				echo %%v> vsdevcmd
				set BUILD_VSDEVCMD_PATH=%%v
				set BUILD_VSDEVCMD_ARCHITECTURE=%BUILD_ARCHITECTURE%
				call "%%BUILD_VSDEVCMD_PATH%%" -arch=%%BUILD_ARCHITECTURE%%
				echo on
				exit /b
			)
		)
	)
	echo "Visual Studio compiler not found. If you are using non-standard location, create file named 'vsdevcmd' with full path to vsdevcmd.bat"
	exit /b

:help
	echo This script will help you to enter Visual Studio comamnd prompt and compile the program under Windows
	echo Possible arguments are:
	echo 	x86 - to compile with x86 32-bit architecture
	echo 	amd64 - to compile with x86 64-bit architecture
	echo 	Everything else to ptint this help message
	exit /b