
#ifndef CODEC_SLICE_H
#define CODEC_SLICE_H

namespace codec {

struct Slice {
  const char *buff;
  unsigned int size;

  constexpr bool operator==(const Slice &r) noexcept {
    if (size != r.size) {
      return false;
    }
    return memcmp(buff, r.buff, size) == 0;
  }
};

} /* namespace codec */

#endif /* CODEC_SLICE_H */
