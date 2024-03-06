# FIGlet4Win

```
 _____ ___ ____ _      _   _  ___        ___
|  ___|_ _/ ___| | ___| |_| || \ \      / (_)_ __
| |_   | | |  _| |/ _ \ __| || |\ \ /\ / /| | '_ \
|  _|  | | |_| | |  __/ |_|__   _\ V  V / | | | | |
|_|   |___\____|_|\___|\__|  |_|  \_/\_/  |_|_| |_|
```

FIGlet is a program that creates large characters out of ordinary screen characters. It can create characters in many different styles and can kern and "smush" these characters together in various ways.

FIGlet output is generally reminiscent of the sort of "signatures" many people like to put at the end of e-mail and UseNet messages.

And FIGlet4Win is a project based on the original FIGlet, which is designed for DOS and Unix-like systems, to let it operate on modern Windows just like it used to be.

Since the official FIGlet on Windows (FIGWin) is a re-implementation of FIGlet with a GUI and it is not maintained after Windows 2000, and ~~we actually don't need to change a lot of things to make FIGlet run well on Windows 11~~, FIGlet4Win is totally based on the original FIGlet code and GUI is not considered.

The official FIGlet web page: http://www.figlet.org/

## Build

FIGlet4Win should (and only can) be built with MinGW. MSVC and Clang is not supported up till now.

Don't like the original FIGlet, FIGlet4Win uses CMake for construction.

1. Clone this repo

    ```bat
    git clone https://github.com/Ace-Radom/figlet4win.git
    cd figlet4win
    ```

2. Make a build directory

    ```bat
    mkdir build
    cd build
    ```

3. Do CMake configure

    ```bat
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE
    ```

    If you want to remove TLF2 font file support, add `-DFIGlet_Enable_TLF_FONTS=OFF` to config command.

    If you want to change FIGlet default font (`standard`), add `-DDEFAULT_FONT_FILE={font_file}` to config command.

    If you want to change FIGlet default font directory (`fonts`), add `-DDEFAULT_FONT_DIR={font_directory}` to config command.

4. Make

    ```bat
    mingw32-make
    ```

5. Test the build if you want to (Python3 is required)

    ```bat
    ctest -C RELEASE
    ```
## Installed files

| File | Description |
| ---- | ----------- |
| `figlet.exe` | The FIGlet program. |
| `figinstall.exe` | A program that installs font files to FIGlet default font directory. |
| `chkfont.exe` | A program that checks FIGlet fonts for formatting errors. You can ignore this file unless you intend to design or edit fonts. |
| `figlist.bat` | Script that lists available fonts and control files. |
| `showfigfonts.bat` | Script that gives a sample of each available font. |
| `figman.bat` | Script that opens FIGlet4Win manual page |
| `doc/` | Directory containing FIGlet4Win chm manual page |
| `fonts/` | Directory containing fonts and control files. |
| `*.flf` | All files with `.flf` ext are FIGlet font files. |
| `*.flc` | All files with `.flc` ext are FIGlet control files. |

## How to use

Type `figlet` at the shell prompt, then type `Hello, World!` and press return. `Hello, World!` in nice, big, designer characters should appear on your screen. If you chose `standard.flf` to be the default font, you should see:

```
 _   _      _ _         __        __         _     _ _ 
| | | | ___| | | ___    \ \      / /__  _ __| | __| | |
| |_| |/ _ \ | |/ _ \    \ \ /\ / / _ \| '__| |/ _` | |
|  _  |  __/ | | (_) |    \ V  V / (_) | |  | | (_| |_|
|_| |_|\___|_|_|\___( )    \_/\_/ \___/|_|  |_|\__,_(_)
                    |/
```

Then type something else, or type an EOF (typically Ctrl-D) to quit FIGlet.

You can just copy the output of FIGlet, or redirect the output to a file (e.g., `figlet Hello, World! > helloworld.txt`) and use them in your own way.

To use other fonts, use the `-f` command line option. For example, if you had typed `figlet -f smslant` above, you would have seen:

```
   __ __    ____         _      __         __   ____
  / // /__ / / /__      | | /| / /__  ____/ /__/ / /
 / _  / -_) / / _ \_    | |/ |/ / _ \/ __/ / _  /_/
/_//_/\__/_/_/\___( )   |__/|__/\___/_/ /_/\_,_(_)
                  |/
```

Here are some other useful command line options:

| Option | Description |
| ------ | ----------- |
| `-c` | centers the output of FIGlet. |
| `-k` | tells FIGlet to kern characters without smushing them together. |
| `-t` | FIGlet asks your terminal how wide it is, and uses this to determine when to break lines. Normally, FIGlet assumes 80 columns so that people with wide terminals won't annoy the people they e-mail FIGlet output to. |
| `-p` | enable paragraph mode, eliminates some spurious line breaks when piping a multi-line file through FIGlet. |
| `-v` | prints information about your copy of FIGlet. |

Other usage are discribed in the chm manual file. Type `figman` in console to open it.

## Authors

FIGlet was written mostly by Glenn Chappell <c486scm@semovm.semo.edu>, Ian Chai <ianchai@usa.net> and has since moved on to another FIGlet enthusiast, Christiaan Keet <info@figlet.org>. The latest maintenance is conducted by Claudio Matsuoka <cmatsuoka@gmail.com>.

FIGlet4Win is a fork of [Claudio's FIGlet tree](https://github.com/cmatsuoka/figlet) and maintained by Sichen Lyu <sichenradomlyu@gmail.com>. Its goal is to get FIGlet running on modern Windows just like it used to be.

Sincere thanks to the former developers of FIGlet project.
