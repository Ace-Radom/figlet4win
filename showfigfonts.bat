@rem showfigfonts for Windows by Sichen Lyu, 17 Jan 2024
@rem Based on the original showfigfonts script by Glenn Chappell <ggc@uiuc.edu>, 25 Aug 1994
@rem You can check the original script at: https://github.com/cmatsuoka/figlet/blob/master/showfigfonts
@rem
@rem Prints a list of available figlet fonts, along with a sample of each
@rem font.  If directory is given, lists fonts in that directory; otherwise
@rem uses the default font directory.  If word is given, prints that word
@rem in each font; otherwise prints the font name.
@rem
@rem Usage: showfigfonts [ -d directory ] [ word ]

@echo off
setlocal enabledelayedexpansion

set figlet_path="%~dp0figlet.exe"

set usage_msg=Usage: %~n0 [ -d directory ] [ word ]

set argc=0
for %%i in (%*) do set /a argc+=1
@rem count args

if "%1"=="-d" (
    if %argc% gtr 3 (
        echo %usage_msg%
        exit 1
    )
    if %argc% lss 2 (
        echo %usage_msg%
        exit 1
    )
    set fontdir="%2"
    if %argc% lss 3 (
        goto:start_print
    )
    set word="%3"
) else (
    for /f "delims=" %%i in ('%figlet_path% -I2') do set fontdir="%%i"
    if not "!fontdir:~2,1!"==":" (
        if not "!fontdir:~3,1!"=="\" (
            set fontdir="%~dp0!fontdir!"
        )
    )
    if %argc% lss 1 (
        goto:start_print
    )
    set word="%1"
    if %argc% gtr 1 (
        echo %usage_msg%
        exit 1
    )
)

:start_print

for /r %fontdir% %%i in (*.flf) do (
    echo %%~ni :
    if not defined word (
        call %figlet_path% -d %fontdir% -f "%%~ni" "%%~ni"
    ) else (
        call %figlet_path% -d %fontdir% -f "%%~ni" "%word%"
    )
    echo.
)

endlocal
