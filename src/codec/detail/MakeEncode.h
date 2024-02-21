
#ifndef CODEC_MAKEENCODE_H
#define CODEC_MAKEENCODE_H

#include <string>

#include "Buff.h"
#include "Fixed.h"
#include "Len.h"
#include "Slice.h"
#include "TagType.h"
#include "VarInt.h"

namespace codec {

struct MakeEncode {
  int operator()(const char *v, uint32_t n, uint32_t index, Buff &buff) {
    TagType tag(kLen, index);
    Len value(v, n);
    if (buff.size <
        buff.offset + value.size + value.size_buff.size + tag.size) {
      return -1;
    }
    // write type
    memcpy(buff.buff + buff.offset, tag.buff, tag.size);
    buff.offset += tag.size;
    // write size
    memcpy(buff.buff + buff.offset, value.size_buff.buff, value.size_buff.size);
    buff.offset += value.size_buff.size;
    // write byte
    if (value.buff) {
      memcpy(buff.buff + buff.offset, value.buff, value.size);
    }
    buff.offset += (uint32_t)value.size;

    return 0;
  }

  int operator()(const std::string &v, uint32_t index, Buff &buff) {
    return operator()(v.c_str(), (uint32_t)v.size(), index, buff);
  }

  int operator()(const Slice &v, uint32_t index, Buff &buff) {
    return operator()(v.buff, v.size, index, buff);
  }

  template <typename T, typename W = VarInt, WireType wire_type = kVarInt>
  int operator()(T v, uint32_t index, Buff &buff) {
    TagType tag(wire_type, index);
    W value(v);
    if (buff.size < buff.offset + value.size + tag.size) {
      return -1;
    }
    // write type
    memcpy(buff.buff + buff.offset, tag.buff, tag.size);
    buff.offset += tag.size;

    // write byte
    memcpy(buff.buff + buff.offset, value.buff, value.size);
    buff.offset += value.size;

    return 0;
  }

  int operator()(float v, uint32_t index, Buff &buff) {
    return operator()<float, I32, kI32>(v, index, buff);
  }

  int operator()(double v, uint32_t index, Buff &buff) {
    return operator()<double, I64, kI64>(v, index, buff);
  }
};

template <size_t index, typename T, typename M = MakeEncode>
int make_encode(Buff &buff, T const &v) {
  return M{}(v, index, buff) * index;
}

} /* namespace codec */

#endif /* CODEC_MAKEENCODE_H */
