

#include <stdio.h>
#include <string.h>

#include "range_encoder.hpp"
#include "range_decoder.hpp"

#include "map.hpp"

struct context {
  char sig[8] = "rcoder";
  std::array<uint32_t, 256> starts;
  std::array<uint32_t, 256> sizes;
  static const uint32_t mask = 255;
  uint32_t total;
  size_t size;
};

int usage() {
  printf("usage: rcoder [-d] filename\n");
  return 1;
}

int main(int argc, char **argv) {
  bool decode = false;
  char *filename = nullptr;

  for (int i = 1; i < argc; ++i) {
    char *arg = argv[i];
    if (arg[0] == '-') {
      if (!strcmp(arg+1, "d")) {
        decode = true;
      } else {
        return usage();
      }
    } else {
      if (filename != nullptr) {
        return usage();
      }
      filename = arg;
    }
  }

  map in_file(filename, "r");

  context ctxt;
  if (decode) {
    std::string outname = filename;
    outname.append(".dec");

    auto p = in_file.begin();
    memcpy(&ctxt, p, sizeof(ctxt));
    p += sizeof(ctxt);
    auto e = p + ctxt.total;

    map out_file(outname, "w", ctxt.total);
    auto end = range_decoder(ctxt, out_file.begin(), out_file.end(), p, e);

    printf("%d..%d bytes\n", int(in_file.size()), int(out_file.size()));
  } else {
    std::string outname = filename;
    outname.append(".rc");

    map out_file(outname, "w", in_file.size() * 2);
    auto end = range_encoder(ctxt, out_file.begin(), out_file.end(), in_file.begin(), in_file.end());
    out_file.truncate(end - out_file.begin());
    printf("%d..%d bytes\n", int(in_file.size()), int(out_file.size()));
  }
}

