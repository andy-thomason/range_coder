

#include "range_encoder.hpp"
#include "range_decoder.hpp"

struct context {
  std::array<uint32_t, 256> starts;
  std::array<uint32_t, 256> sizes;
  static const uint32_t mask = 255;
  uint32_t total;
};

int main() {
  auto msg = "hellohello";
  uint8_t buf[256];
  context ctxt;
  uint8_t *end = range_encoder(ctxt, buf, msg, msg+10);
  printf("%d bytes\n", int(end - buf));

  char msg2[256];
  char *msg2end = range_decoder(ctxt, msg2, buf, end);
}

