/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>
#include <sstream>
#include <string>

#include "misc.h"
#include "commands.h"


int main(int argc, char **argv) {

  std::cout << getCmdOutput("uname -s -m -r") << std::endl;

  cd("/data/local");
  std::cout << cwd() << std::endl;

  std::cout << clok() << std::endl;
  std::cout << time(NULL) << std::endl;

  std::cout << dirw("/") << std::endl;
  std::cout << dirw("/data/local") << std::endl;

  std::cout << hash("/init.rc") << std::endl;
  std::cout << hash("/weird/path") << std::endl;

  std::cout << id() << std::endl;

  std::cout << uptime() << std::endl;

  std::cout << systime() << std::endl;

  std::cout << screen() << std::endl;

  std::cout << power() << std::endl;

  std::cout << memory() << std::endl;

  std::cout << ps() << std::endl;

  return 0;
}
