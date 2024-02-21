
#ifndef CODEC_MAKEDECODE_H
#define CODEC_MAKEDECODE_H

#include <string>

#include "Buff.h"
#include "Fixed.h"
#include "Len.h"
#include "Slice.h"
#include "TagType.h"
#include "VarInt.h"

namespace codec {

struct MakeDecode {
  int operator()(uint32_t index, CBuff &buff) {
    TagType tag = TagType::byte_from(buff.buff + buff.offset);
    if (buff.size < buff.offset + tag.size) {
      return -1;
    }
    if ((uint32_t)tag != index) {
      return -1;
    }
    return 0;
  }

  int operator()(const char *&v, uint32_t &n, uint32_t index, CBuff &buff) {
    TagType tag = TagType::byte_from(buff.buff + buff.offset);
    if (buff.size < buff.offset + tag.size) {
      return -1;
    }
    if ((WireType)tag != kLen) {
      return -1;
    }
    if ((uint32_t)tag != index) {
      return -1;
    }
    auto offset = buff.offset + tag.size;
    Len value;
    value.byte_from(buff.buff + offset);
    if (buff.size < offset + value.size + value.size_buff.size) {
      return -1;
    }
    v = value.buff;
    n = (uint32_t)value.size;
    offset += value.size_buff.size;
    offset += (uint32_t)value.size;
    buff.offset = offset;
    return 0;
  }

  int operator()(Slice &v, uint32_t index, CBuff &buff) {
    return operator()(v.buff, v.size, index, buff);
  }

  int operator()(std::string &v, uint32_t index, CBuff &buff) {
    Slice slice = {};
    int ret = operator()(slice, index, buff);
    if (ret < 0) {
      return ret;
    }
    v = std::string(slice.buff, slice.size);
    return 0;
  }

  template <typename T, typename W = VarInt, WireType wire_type = kVarInt>
  int operator()(T &v, uint32_t index, CBuff &buff) {
    TagType tag = TagType::byte_from(buff.buff + buff.offset);
    if (buff.size < buff.offset + tag.size) {
      return -1;
    }
    if ((WireType)tag != wire_type) {
      return -1;
    }
    if ((uint32_t)tag != index) {
      return -1;
    }
    auto offset = buff.offset + tag.size;
    W value;
    value.byte_from(buff.buff + offset);
    if (buff.size < offset + value.size) {
      return -1;
    }
    v = value;
    offset += value.size;
    buff.offset = offset;
    return 0;
  }

  int operator()(float &v, uint32_t index, CBuff &buff) {
    return operator()<float, I32, kI32>(v, index, buff);
  }

  int operator()(double &v, uint32_t index, CBuff &buff) {
    return operator()<double, I64, kI64>(v, index, buff);
  }
};

template <size_t index, typename M = MakeDecode>
int make_decode_exist(CBuff &buff) {
  return M{}(index, buff) * index;
}

template <size_t index, typename T, typename M = MakeDecode>
int make_decode(CBuff &buff, T &v) {
  return M{}(v, index, buff) * index;
}

} /* namespace codec */

#endif /* CODEC_MAKEDECODE_H */
