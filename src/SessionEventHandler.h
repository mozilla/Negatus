/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __SESSION_EVENT_HANDLER__
#define __SESSION_EVENT_HANDLER__

#include <string>

class SessionEventHandler {
public:
    std::string cwd;

    SessionEventHandler();
};

#endif
