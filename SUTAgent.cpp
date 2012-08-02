/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this file,
 You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>
#include <vector>

//using namespace std;

int main(int argc, char **argv) {

  std::cout << "Hello, world!" << std::endl;

  std::vector<int> ints;
  int i;
  for (i = 0; i < 25; ++i) {
    ints.push_back(i);
  }

  while (!ints.empty()) {
    std::cout << ints.back() << " ";
    ints.pop_back();
  }

  std::cout << std::endl;

  return 0;
}
