
#ifndef CODEC_WIRETYPE_H
#define CODEC_WIRETYPE_H

/// referrence: https://protobuf.dev/programming-guides/encoding/

enum WireType {
  kVarInt = 0,
  kI64 = 1,
  kLen = 2,
  /* reserve */
  /* reserve */
  kI32 = 5,
};

#endif /* CODEC_WIRETYPE_H */
