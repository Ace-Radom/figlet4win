@rem figlist for Windows by Sichen Lyu, 17 Jan 2024
@rem Based on the original figlist script by Glenn Chappell <gcc@uiuc.edu>, 25 Aug 1994
@rem You can check the original script at: https://github.com/cmatsuoka/figlet/blob/master/figlist
@rem
@rem Lists all fonts and control files in figlet's default font directory.
@rem Replaces "figlet -F", which was removed from figlet version 2.1.
@rem
@rem Usage: figlist [ -d directory ]

@echo off
setlocal enabledelayedexpansion

set figlet_path="%~dp0figlet.exe"

set usage_msg=Usage: %~n0 [ -d directory ]

set argc=0
for %%i in (%*) do set /a argc+=1
@rem count args

if "%1"=="-d" (
    if %argc% neq 2 (
        echo %usage_msg%
        exit 1
    )
    set fontdir=%2
) else (
    if %argc% neq 0 (
        echo %usage_msg%
        exit 1
    )
    set fontdir=
)

if "%fontdir%"=="" (
    for /f "delims=" %%i in ('%figlet_path% -I2') do set fontdir="%%i"
)
for /f "delims=" %%i in ('%figlet_path% -I3') do set font=%%i
echo Default font: %font%
echo Font directory: %fontdir%

if not "!fontdir:~2,1!"==":" (
    if not "!fontdir:~3,1!"=="\" (
        set fontdir="%~dp0!fontdir!"
    )
)

if exist %fontdir% (
    set flf_count=0
    for /r %fontdir% %%i in (*.flf) do (
        if !flf_count! equ 0 (
            echo Figlet fonts in this directory:
        )
        set /a flf_count+=1
        echo %%~ni
    )
    if !flf_count! equ 0 (
        echo No figlet fonts in this directory
    )
    set flc_count=0
    for /r %fontdir% %%i in (*.flc) do (
        if !flc_count! equ 0 (
            echo Figlet control files in this directory:
        )
        set /a flc_count+=1
        echo %%~ni
    )
    if !flc_count! equ 0 (
        echo No figlet control files in this directory
    )
) else (
    echo Unable to open directory
)

endlocal
