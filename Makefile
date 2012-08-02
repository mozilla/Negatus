AGPP := \
  $(NDKPATH)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/darwin-x86/bin/arm-linux-androideabi-g++
CFLAGS := \
  --sysroot $(NDKPATH)/platforms/android-9/arch-arm -fPIC -mandroid -DANDROID \
  -DOS_ANDROID -fno-short-enums -fno-exceptions -lstdc++
FILES := SUTAgent.cpp
LDIRS := -L$(NDKPATH)/platforms/android-9/arch-arm/usr/lib

agent:
	$(AGPP) -v $(LDIRS) $(CFLAGS) $(FILES) -o agent
