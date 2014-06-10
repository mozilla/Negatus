# Prerequisites - Android

You need to:

* have the Android NDK:
    * http://developer.android.com/tools/sdk/ndk/index.html
    * use the **r5c** version

* have the NSPR headers installed under /usr/include/nspr:
    * `apt-get install libnspr4-dev`, or
    * download from https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/

* export these vars:
    * `NDKPATH` = path to the NDK folder
    * `ADB_PATH` = path to the adb binary

* have a running B2G emulator/device with the ports 20700 and 20701 forwarded:
    * `adb forward tcp:20700 tcp:20700`
    * `adb forward tcp:20701 tcp:20701`

* install busybox on the device:
    * `./setup-tools.sh`

* have the toolchain:
    * `source bootstrap.sh`

# Building, running

Just `./run.sh`.

# Including this in B2G

Once you have the prerequisites and export the environment variables,
just clone this repo inside `$B2G_REPO/system/`. Build and flash B2G as usual.
You will be able to run the agent using `sutagent`. It will be in the `$PATH`.

You do need to set the `LD` path for this to work:

    export LD_LIBRARY_PATH=/vendor/lib:/system/lib:/system/b2g

# Start the agent on boot

Insert this after the exports in `$B2G_REPO/gonk-misc/b2g.sh`:

    export NSPR_LOG_MODULES="NegatusLOG:5, timestamp"
    sutagent &

# NSPR Logging
Before running the agent (not necessary if you use `run.sh`):

`export NSPR_LOG_MODULES="NegatusLOG:5, timestamp"`

Data will be logged to `$TESTROOT/Negatus.log`.

Prerequisites - Desktop Linux

The linux build requires the nspr libraries and headers. On Ubuntu, you can do

    sudo apt-get install g++ libnspr4-dev

Build with:

    make -f Makefile.linux

Invoke with:

    ./agent

Prerequisites - Mac OS X

You need to have the latest version of Xcode (available from the App Store) and the
command line developer tools packages installed. You will have to build your own
nspr Mac installation, available at
https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/.

The default nspr build installs nspr to /usr/local; the Mac Makefile assumes that you
have done that. To build Negatus, do:

    make -f Makefile.macosx

Invoke with:

    ./agent

Prerequisites - Windows

The Windows build requires Visual C++ 2010 or later to be installed, as well as [the MozillaBuild environment]. Install both, then launch a MozillaBuild shell by running start-shell-msvcXXXX.bat, where XXXX is the version you have installed (i.e. 2010).

You will first need to build a copy of NSPR. Download the latest NSPR source from [the Mozilla download server] (currently [NSPR 4.10.6]) and untar it,  or clone it from [the NSPR Mercurial repository] using hg. In the NSPR source directory execute:

    ./configure --enable-win32-target=WIN95 && make

Once NSPR is built, build Negatus with:

    make -f Makefile.win NSPR=/path/to/nspr_directory

where `/path/to/nspr_directory` is the directory you unpacked the NSPR source to in the previous step.

You can start Negatus by typing:

    PATH=$PATH:/path/to/nspr_directory/bin ./agent

You can alternately copy nspr4.dll and plc4.dll to the same directory as agent.exe to avoid setting PATH each time.

[the MozillaBuild environment]: http://ftp.mozilla.org/pub/mozilla.org/mozilla/libraries/win32/MozillaBuildSetup-Latest.exe
[the Mozilla download server]: http://ftp.mozilla.org/pub/mozilla.org/nspr/releases/
[NSPR 4.10.6]: http://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v4.10.6/src/nspr-4.10.6.tar.gz
[the NSPR Mercurial repository]: http://hg.mozilla.org/projects/nspr/
