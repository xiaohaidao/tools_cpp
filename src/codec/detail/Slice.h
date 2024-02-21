
#ifndef CODEC_SLICE_H
#define CODEC_SLICE_H

namespace codec {

struct Slice {
  const char *buff;
  unsigned int size;
};

} /* namespace codec */

#endif /* CODEC_SLICE_H */
