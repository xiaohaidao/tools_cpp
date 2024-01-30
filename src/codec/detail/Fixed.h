
#ifndef CODEC_FIXED_H
#define CODEC_FIXED_H

#include "VarInt.h"

#ifdef _WIN32
inline uint16_t net_to_host(uint16_t v) { return nstoh(v); }
inline uint32_t net_to_host(uint32_t v) { return nltoh(v); }
inline uint64_t net_to_host(uint64_t v) { return nlltoh(v); }
inline uint16_t host_to_net(uint16_t v) { return htons(v); }
inline uint32_t host_to_net(uint32_t v) { return htonl(v); }
inline uint64_t host_to_net(uint64_t v) { return htonll(v); }
#else
inline uint16_t net_to_host(uint16_t v) { return be16toh(v); }
inline uint32_t net_to_host(uint32_t v) { return be32toh(v); }
inline uint64_t net_to_host(uint64_t v) { return be64toh(v); }
inline uint16_t host_to_net(uint16_t v) { return htobe16(v); }
inline uint32_t host_to_net(uint32_t v) { return htobe32(v); }
inline uint64_t host_to_net(uint64_t v) { return htobe64(v); }
#endif // _WIN32

struct Fix3264 {
  template <typename T> uint8_t encode(unsigned char *d, T v, uint8_t offset) {
    d[0] = ((v >> (offset++ * 8)) & 0xFF);
    if (offset < sizeof(T)) {
      return encode(d + 1, v, offset);
    }
    return offset;
  }

  template <typename T>
  std::pair<uint64_t, uint8_t> decode(const unsigned char *d, uint64_t v,
                                      uint8_t offset) {
    v += ((uint64_t)d[0] << (offset++ * 8));
    if (offset < sizeof(T)) {
      return decode<T>(d + 1, v, offset);
    }
    return {v, offset};
  }
};

struct I32 : public Fix3264 {
  unsigned char buff[4];
  unsigned char size;

  I32() : size(sizeof(buff)) {}

  I32(float v) {
    uint32_t *u = (uint32_t *)&v;
    // *u = host_to_net(*u);
    encode(buff, *u, 0);
    size = sizeof(buff);
  }

  operator float() {
    uint32_t u = decode<float>(buff, 0, 0).first;
    // u = net_to_host(u);
    float *v = (float *)&u;
    return *v;
  }

  void byte_from(const char *d) { memcpy(buff, d, sizeof(buff)); }
};

struct I64 : public Fix3264 {
  unsigned char buff[8];
  unsigned char size;

  I64() : size(sizeof(buff)) {}

  I64(double v) {
    uint64_t *u = (uint64_t *)&v;
    // *u = host_to_net(*u);
    encode(buff, *u, 0);
    size = sizeof(buff);
  }

  operator double() {
    uint64_t u = decode<uint64_t>(buff, 0, 0).first;
    // u = net_to_host(u);
    double *v = (double *)&u;
    return *v;
  }

  void byte_from(const char *d) { memcpy(buff, d, sizeof(buff)); }
};

#endif /* CODEC_FIXED_H */
