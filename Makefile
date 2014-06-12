# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

OBJ := .o

include Makefile.common

$(AGENT): $(OBJS)
	$(LD) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

