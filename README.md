# Prerequisites

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
