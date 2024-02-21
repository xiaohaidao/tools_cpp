
#ifndef CODEC_WIRETYPE_H
#define CODEC_WIRETYPE_H

/// referrence: https://protobuf.dev/programming-guides/encoding/

namespace codec {

enum WireType {
  kVarInt = 0,
  kI64 = 1,
  kLen = 2,
  /* reserve */
  /* reserve */
  kI32 = 5,
};

} /* namespace codec */

#endif /* CODEC_WIRETYPE_H */
