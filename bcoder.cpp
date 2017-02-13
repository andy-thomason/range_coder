


#include <stdio.h>
#include <string.h>

#include "block_sorting_encoder.hpp"

#include "map.hpp"

struct context {
  char sig[8] = "rcoder";
  size_t size;
  std::array<uint32_t, 256+1> starts;
  static const uint32_t mask = 255;

  void error(size_t offset, const char *msg) {
    printf("%s @ %lx", msg, long(offset));
  }
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
/*    std::string outname = filename;
    size_t f = outname.rfind(".rc");
    if (false && f == outname.size() - 3) {
      outname.resize(outname.size() - 3);
    } else {
      outname.append(".dec");
    }

    auto p = in_file.begin();
    memcpy(&ctxt, p, sizeof(ctxt));
    p += sizeof(ctxt);
    auto e = p + ctxt.size;

    map out_file(outname, "w", ctxt.size);
    auto end = range_decoder(ctxt, out_file.begin(), out_file.end(), p, e);

    printf("%ld..%ld bytes\n", long(in_file.size()), long(out_file.size()));*/
  } else {
    std::string outname = filename;
    outname.append(".rc");

    map out_file(outname, "w", in_file.size() + in_file.size()/0x10000);
    auto end = block_sorting_encoder(ctxt, out_file.begin(), out_file.end(), in_file.begin(), in_file.end());
    if (end == out_file.end()) {
      printf("error: compressed file too long\n");
      out_file.truncate(0);
      return 1;
    }
    out_file.truncate(end - out_file.begin());
    printf("%ld..%ld bytes\n", long(in_file.size()), long(out_file.size()));
  }
}

