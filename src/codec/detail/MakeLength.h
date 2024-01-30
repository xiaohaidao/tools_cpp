
#ifndef CODEC_MAKELENGTH_H
#define CODEC_MAKELENGTH_H

#include <string>

#include "Buff.h"
#include "Fixed.h"
#include "Len.h"
#include "Slice.h"
#include "TagType.h"
#include "VarInt.h"

struct MakeLength {
  uint32_t operator()(const char *v, size_t n, uint32_t index) {
    TagType tag(kLen, index);
    Len value(v, n);
    return value.size + value.size_buff.size + tag.size;
  }

  uint32_t operator()(const std::string &v, uint32_t index) {
    return operator()(v.c_str(), v.size(), index);
  }

  uint32_t operator()(const Slice &v, uint32_t index) {
    return operator()(v.buff, v.size, index);
  }

  template <typename T, typename W = VarInt, WireType wire_type = kVarInt>
  uint32_t operator()(T v, uint32_t index) {
    TagType tag(wire_type, index);
    W value(v);
    return value.size + tag.size;
  }

  uint32_t operator()(float v, uint32_t index) {
    return operator()<float, I32, kI32>(v, index);
  }

  uint32_t operator()(double v, uint32_t index) {
    return operator()<double, I64, kI64>(v, index);
  }
};

template <size_t index, typename T, typename M = MakeLength>
uint32_t make_length(T const &v) {
  return M{}(v, index);
}

#endif /* CODEC_MAKELENGTH_H */
