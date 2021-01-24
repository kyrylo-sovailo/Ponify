@REM The following file is part of Ironic Make project or was created with it's help
@REM To know more about the project visit github.com/Meta-chan/ironic_make

@echo off
setlocal
set DIRECTORY=%CD%

:start
	if "%1"=="" goto :main
	if "%1"=="vs" goto :compiler
	if "%1"=="gcc" goto :compiler
	if "%1"=="x86" goto :architecture
	if "%1"=="amd64" goto :architecture
	call :help
	exit /b 1

	:compiler
	if not "%COMPILER%"=="" (
		call :help
		exit /b 1
	)
	set COMPILER=%1
	shift
	goto :start

	:architecture
	if not "%ARCHITECTURE%"=="" (
		call :help
		exit /b 1
	)
	set ARCHITECTURE=%1
	shift
	goto :start
	
	:main
	for /f %%d in ('wmic logicaldisk get name ^| find ":"') do (
		call :try_disk %%d
		if not errorlevel 1 (
			echo Compilation success
			for /f "delims=" %%r in ('echo %cmdcmdline% ^| find "\compile.bat"') do pause
			exit /b 0
		)
	)
	echo Can not compile with specified compiler
	for /f "delims=" %%r in ('echo %cmdcmdline% ^| find "\compile.bat"') do pause
	exit /b 1

:try_disk
	if not "%COMPILER%"=="" (
		if not "%COMPILER%"=="vs" goto :skip_vs
	)
	call :try_vs_installation "%1\Program Files\"
	if not errorlevel 1 exit /b 0
	call :try_vs_installation "%1\Program Files (x86)\"
	if not errorlevel 1 exit /b 0
	:skip_vs
	
	if not "%COMPILER%"=="" (
		if not "%COMPILER%"=="gcc" goto :skip_gcc
	)
	call :try_gcc_installation "%1\"
	if not errorlevel 1 exit /b 0
	call :try_gcc_installation "%1\Program Files\"
	if not errorlevel 1 exit /b 0
	call :try_gcc_installation "%1\Program Files (x86)\"
	if not errorlevel 1 exit /b 0
	call :try_gcc_installation "%1\Program Files\CodeBlocks\"
	if not errorlevel 1 exit /b 0
	call :try_gcc_installation "%1\Program Files (x86)\CodeBlocks\"
	if not errorlevel 1 exit /b 0
	:skip_gcc
	
	exit /b 1
		
:try_vs_installation
	if not exist %1 exit /b 1
	if exist %1"Microsoft Visual Studio" (
		cd /d %1"Microsoft Visual Studio"
		for /f "delims=" %%v in ('dir /b /s /a:-d "vsdevcmd.bat" 2^>nul') do (
			cd /d "%DIRECTORY%"
			REM COMPILE
			if "%ARCHITECTURE%"=="" set CL_ARCHITECTURE=
			if not "%ARCHITECTURE%"=="" set CL_ARCHITECTURE=-arch=%ARCHITECTURE%
			 "%%v" %CL_ARCHITECTURE%
			cl ponify.c /link /OUT:ponify.exe
			if not errorlevel 1 (
				del ponify.obj
				exit /b 0
			)
			del ponify.obj
		)
	)
	exit /b 1

:try_gcc_installation
	if not exist %1 exit /b 1
	cd /d %1
	for /f "delims=" %%u in ('dir /b /a:d "*mingw*" 2^>nul') do (
		call :try_gcc_directory %%1\"%%u"
		if not errorlevel 1 exit /b 0
	)
	exit /b 1

:try_gcc_directory
	cd /d %1
	for /f "delims=" %%v in ('dir /b /s /a:d "bin" 2^>nul') do (
		call :try_gcc_path %%v
		if not errorlevel 1 exit /b 0
		exit /b 1
	)
	exit /b 1

:try_gcc_path
	cd /d "%DIRECTORY%"
	REM COMPILE
	set PATH=%*;%PATH%
	if "%ARCHITECTURE%"=="" set GCC_ARCHITECTURE=
	if "%ARCHITECTURE%"=="x86" set GCC_ARCHITECTURE=-m32
	if "%ARCHITECTURE%"=="amd64" set GCC_ARCHITECTURE=-m64
	gcc %GCC_ARCHITECTURE% ponify.c -o ponify.exe
	if not errorlevel 1 exit /b 0
	exit /b 1

:help
	echo This script will help you to compile your program on Windows
	echo Possible arguments are:
	echo	Nothing	- to compile with default compiler and architecture
	echo 	x86		- to compile with x86 32-bit architecture
	echo 	amd64	- to compile with x86 64-bit architecture
	echo 	gcc		- to compile with gcc/g++ compiler
	echo	vs		- to compile with Visual Studio compiler
	echo 	Everything else to ptint this help message
	echo Visit github.com/Meta-chan/ironic_make to know more about the project
	for /f "delims=" %%r in ('echo %cmdcmdline% ^| find "\compile.bat"') do pause
	exit /b 0