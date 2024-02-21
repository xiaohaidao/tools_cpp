
#ifndef CODEC_LEN_H
#define CODEC_LEN_H

#include "VarInt.h"

namespace codec {

struct Len {
  const char *buff;
  size_t size;
  VarInt size_buff;

  Len() : buff(nullptr), size(0) {}

  Len(const char *d, size_t n) {
    buff = d;
    size = n;
    size_buff = size;
  }

  Len(const char *d) {
    buff = d;
    size = strlen(d);
    size_buff = size;
  }

  void byte_from(const char *d) {
    size_buff.byte_from(d);
    size = size_buff;
    buff = d + size_buff.size;
  }
};

} /* namespace codec */

#endif /* CODEC_LEN_H */
