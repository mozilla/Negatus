/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>

#include "misc.h"
#include "CommandEventHandler.h"


// we can use the code in this function to write some tests
int main(int argc, char **argv) {

  CommandEventHandler cmd;

  std::cout << getCmdOutput("uname -s -m -r") << std::endl;

  cmd.cd("/data/local");
  std::cout << cmd.cwd() << std::endl;

  std::cout << cmd.clok() << std::endl;
  std::cout << time(NULL) << std::endl;

  std::cout << cmd.dirw("/") << std::endl;
  std::cout << cmd.dirw("/data/local") << std::endl;

  std::cout << cmd.hash("/init.rc") << std::endl;
  std::cout << cmd.hash("/weird/path") << std::endl;

  std::cout << cmd.id() << std::endl;

  std::cout << cmd.uptime() << std::endl;

  std::cout << cmd.systime() << std::endl;

  std::cout << cmd.screen() << std::endl;

  std::cout << cmd.power() << std::endl;

  std::cout << cmd.memory() << std::endl;

  std::cout << cmd.ps() << std::endl;

  std::cout << cmd.isDir("/data/local") << std::endl;
  std::cout << cmd.isDir("/init.rc") << std::endl;
  std::cout << cmd.isDir("/weird/path") << std::endl;

  std::cout << cmd.ls("/") << std::endl;

  std::cout << cmd.mkdir("/data/local/testdir") << std::endl;
  std::cout << cmd.mkdir("/data/local") << std::endl;

  std::cout << cmd.rm("/data/something") << std::endl;

  return 0;
}
