

#include "range_coder.hpp"


int main() {
  auto msg = "hellohello";
  uint8_t buf[256];
  uint8_t *end = range_coder(buf, msg, msg+10);
  printf("%d bytes\n", int(end - buf));
}

