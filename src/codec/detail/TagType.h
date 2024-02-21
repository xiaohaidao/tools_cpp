
#ifndef CODEC_TAGTYPE_H
#define CODEC_TAGTYPE_H

#include "VarInt.h"
#include "WireType.h"

namespace codec {

struct TagType {
  unsigned char buff[11];
  unsigned char size;

  TagType(WireType type, uint32_t index) {
    buff[0] = type | ((index & 0x0F) << 3);
    size = 1;
    if (index & 0xF0) {
      buff[0] |= 0x80;
      VarInt i = (index >> 4);
      memcpy(buff + 1, i.buff, i.size);
      size += i.size;
    }
  }

  operator WireType() { return (WireType)(buff[0] & 0x07); }
  operator uint32_t() {
    uint32_t re = buff[0] >> 3;
    if (!(re & 0xF0)) {
      return re;
    }
    VarInt v;
    v.byte_from(buff + 1);
    uint32_t high = v;
    return re + (high << 4);
  }

  static TagType byte_from(const char *b) {
    TagType tag{kVarInt, 0};
    tag.byte(b);
    return tag;
  }

private:
  void byte(const char *b) {
    if (!(b[0] & 0x80)) {
      buff[0] = b[0];
      size = 1;
      return;
    }
    VarInt v;
    v.byte_from(b + 1);
    size = v.size + 1;
    size = size >= sizeof(buff) ? sizeof(buff) - 1 : size;
    buff[sizeof(buff) - 1] = 0;
    memcpy(buff, b, size);
  }
};

} /* namespace codec */

#endif /* CODEC_TAGTYPE_H */
