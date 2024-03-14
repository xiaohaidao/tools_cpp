
#ifndef CODEC_BUFF_H
#define CODEC_BUFF_H

namespace codec {

struct Buff {
  char *buff;
  unsigned int size;
  unsigned int offset;
};

struct CBuff {
  const char *buff;
  const unsigned int size;
  unsigned int offset;
};

} /* namespace codec */

#endif /* CODEC_BUFF_H */
