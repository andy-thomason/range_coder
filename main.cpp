

#include <stdio.h>
#include <string.h>
#include <fstream>

#include "range_encoder.hpp"
#include "range_decoder.hpp"

struct context {
  char sig[8] = "rcoder";
  std::array<uint32_t, 256> starts;
  std::array<uint32_t, 256> sizes;
  static const uint32_t mask = 255;
  uint32_t total;
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

  auto f = std::ifstream(filename, std::ios::binary);
  f.seekg(0, std::ios::end);
  auto len = f.tellg();
  f.seekg(0, std::ios::beg);
  std::vector<uint8_t> file_in((size_t)len);
  f.read((char*)file_in.data(), len);
  f.close();

  std::vector<uint8_t> msg_out;
  context ctxt;

  if (decode) {
    auto p = file_in.data();
    memcpy(&ctxt, p, sizeof(ctxt));
    auto e = p + sizeof(ctxt) + ctxt.total;
    msg_out.resize(ctxt.total);
    auto end = range_decoder(ctxt, msg_out.begin(), msg_out.end(), file_in.begin(), file_in.end());

    std::string outname = filename;
    outname.append(".dec");
    auto of = std::ofstream(outname, std::ios::binary);

    of.write((char*)msg_out.data(), size_t(end - msg_out.begin()));
    of.close();
  } else {
    msg_out.resize(len * 2);
    auto end = range_encoder(ctxt, msg_out.begin(), msg_out.end(), file_in.begin(), file_in.end());
    printf("%d..%d bytes\n", int(len), int(end - msg_out.begin()));

    std::string outname = filename;
    outname.append(".rc");

    auto of = std::ofstream(outname, std::ios::binary);
    of.write((char*)&ctxt, sizeof(ctxt));
    of.write((char*)msg_out.data(), size_t(end - msg_out.begin()));
    of.close();
  }
}

