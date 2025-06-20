# sys-nbframework-src

Framework to build mobile, desktop and web apps in C and their backends.

# Origin

Created by [Marcos Ortega](https://mortegam.com/), this project aims to create an unified and improved version of [lib-auframework-src](https://github.com/marcosjom/lib-auframework-src), [lib-auframework-media-src](https://github.com/marcosjom/lib-auframework-media-src) and [lib-auframework-app-src](https://github.com/marcosjom/lib-auframework-app-src) for coding and building libraries, command line tools, and visual apps for multiple platforms.

Projects already built using this includes:

- A [web server](https://github.com/marcosjom/sys-tocatl-webserver-src) and client that you can run on Linux, Windows, MacOS or integrate into other projects.
- A [real-time surveillance camera system](https://github.com/marcosjom/sys-thinstream-src) for streams storage and/or distribution.
- [Mobile and desktop apps](https://mortegam.com/) and their backends (sharing the same code).
- [Command line tools](https://github.com/marcosjom/sys-devtools-src) for development.
- [Visual tools](https://github.com/marcosjom/sys-noteu-src) for development.
- A Windows-driver and MacOS kernel-extension for virtual inputs (microphone) and outputs (speaker).
- Others...

# Features*

- Operating system abstractions, use: `NBSocket`, `NBFile`, `NBIO`, `NBIOPollster`, `NBFont`, `NBSsl`, `NBThread`, `NBMemory`, `NBLog`, `NBString`, `NBLocale`, `NBThreadMutex`, `NBThreadCond`, ...
  - and the library will use the operating-system specifics at compilation time; desktop, mobile, driver/kernel or web.

- Thread-safe by default

- Support for event-driven-io (Input/Output: sockets, files, storage). 

- Debug runtime verification:
  - In debug mode, the framework will `assert` risks for dangerous situations like: `freeing-unknown-pointer`, `double-free`, `mutual-thread-locks`, others...
    - these dangerous code-logic can randomly block or crash your program at runtime.
    - these assertions will force you to refactor your code.

- Files and protocols support: `NBSintaxParser.h` (C99 code), `NBJsonParser`, `NBPlistParser`, `NBXmlParser`, `NBSmtpHeader`, `NBSmtpBody`, `NBNtln`, `NBOAuthClient`, `NBMsExchangeClt`, `NBHttpBuilder`, `NBHttpMessage`, `NBHttp2Parser`, `NBRtsp`, `NBRtcParser`, `NBRtcpParser`, `NBUtlParser`, `NBMp4` (file container), `NBAvc` (H.264 units), `NBPng`, `NBJpeg`, `NBTextMetricsBuilder`, `NBFontGlyphs`, ...
  - implemented from scratch, as state-machines capable to parse/write even one byte at the time (for quick parsing errors detection).

(*) some features are are work in progress.

# How to compile

The following steps will create static libraries. In your project you should link to the `static-libray` and add a reference to its `include` folder.

## Windows

Open `projects/visual-studio/nbframework.sln` and compile the desired target.

## MacOS and iOS

Open `projects/xcode/nbframework.xcworkspace` and compile the desired target.

## Android

In a terminal:

```
cd sys-nbframework-src
make NB_CFG_HOST=Android
```

Check each project's `Makefile` and `MakefileProject.mk` files, and the [MakefileFuncs.mk](https://github.com/marcosjom/sys-nbframework-src/blob/main/MakefileFuncs.mk) to understand the `make` process, including the accepted flags and targets. 

## Linux and Others

In a terminal:

```
cd sys-nbframework-src
make
```

Check each project's `Makefile` and `MakefileProject.mk` files, and the [MakefileFuncs.mk](https://github.com/marcosjom/sys-nbframework-src/blob/main/MakefileFuncs.mk) to understand the `make` process, including the accepted flags and targets.

# MakefileFuncs.mk

This is a `make` file containing functions that allows you to describe a workspace similarly to XCode, Visual-Stuido and other IDEs.

Its purpose is to describe your project in an structure and be able to set individual compiler-flags to each code-group. The structure is:

- `Workspace`
  - `Project`
    - `Target` (exe, lib, so, dlyb)
      - `Code-group`

The `workspace` is the folder you are explicitly calling `make` from. You can import your current project and dependencies from other folders.

Once all the `Projects`, `Targets` and `Code-groups` were imported, the current call to make will build all (or an specific) target; compiling each file with their respective flags, merging outputs and linking libraries.

Basically is a command-line version of how you manage and build your projects, inspired in how you organize your work in XCode.

The `compiler` and `tool-chain` is selected automatically or explicitly by the `NB_CFG_HOST=` param. As example `NB_CFG_HOST=Android` will compile using `Android NDK` tool-chain.

# Contact

Visit [mortegam.com](https://mortegam.com/) for more information and visual examples of projects built with this libray.

May you be surrounded by passionate and curious people. :-)
