
#ifndef CODEC_VARINT_H
#define CODEC_VARINT_H

#include <stdint.h>
#include <string.h>

#include <algorithm>

struct VarInt {
  unsigned char buff[11];
  unsigned char size;

  VarInt() : size(0) {}

  VarInt(int8_t v) { size = encode(buff, zigzag(v), 0); }
  VarInt(int16_t v) { size = encode(buff, zigzag(v), 0); }
  VarInt(int32_t v) { size = encode(buff, zigzag(v), 0); }
  VarInt(int64_t v) { size = encode(buff, zigzag(v), 0); }
  // VarInt(uint8_t v) { size = encode(buff, v, 0); }
  // VarInt(uint16_t v) { size = encode(buff, v, 0); }
  // VarInt(uint32_t v) { size = encode(buff, v, 0); }
  // VarInt(uint64_t v) { size = encode(buff, v, 0); }
  // VarInt(bool v) { size = encode(buff, v, 0); } // it will match pointer
  template <typename T> VarInt(T v) { size = encode(buff, v, 0); }

  void byte_from(const unsigned char *d, size_t s = SIZE_MAX) {
    size = decode(d, 0, 0).second;
    size = std::min(std::min(sizeof(buff) - 1, (size_t)size), s);
    buff[size] = 0;
    memcpy(buff, d, size);
  }

  void byte_from(const char *d, size_t s = SIZE_MAX) {
    byte_from((const unsigned char *)d, s);
  }

  operator int8_t() { return unzigzag(decode(buff, 0, 0).first); }
  operator int16_t() { return unzigzag(decode(buff, 0, 0).first); }
  operator int32_t() { return unzigzag(decode(buff, 0, 0).first); }
  operator int64_t() { return unzigzag(decode(buff, 0, 0).first); }
  // operator uint8_t() { return decode(buff, 0, 0).first; }
  // operator uint16_t() { return decode(buff, 0, 0).first; }
  // operator uint32_t() { return decode(buff, 0, 0).first; }
  // operator uint64_t() { return decode(buff, 0, 0).first; }
  // operator bool() { return decode(buff, 0, 0).first; }
  template <typename T> operator T() { return decode(buff, 0, 0).first; }

  uint8_t encode(unsigned char *d, uint64_t v, uint8_t offset) {
    uint64_t offset_v = (v >> (offset++ * 7));
    d[0] = (offset_v & 0x7F);
    if (offset_v >> 7) {
      d[0] |= 0x80;
      return encode(d + 1, v, offset);
    }
    return offset;
  }

  std::pair<int64_t, uint8_t> decode(const unsigned char *d, int64_t v,
                                     uint8_t offset) {
    v += ((uint64_t)(d[0] & 0x7F) << (offset++ * 7));
    if (d[0] & 0x80) {
      return decode(d + 1, v, offset);
    }
    return {v, offset};
  }

  template <typename T> T unzigzag(T d) { return (d >> 1) ^ -(d & 1); }

  template <typename T> T zigzag(T d) {
    return ((typename std::make_unsigned<T>::type)(d) << 1) ^
           (d >> (sizeof(T) * 8 - 1));
  }
};

#endif /* CODEC_VARINT_H */
