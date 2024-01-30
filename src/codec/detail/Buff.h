
#ifndef CODEC_BUFF_H
#define CODEC_BUFF_H

struct Buff {
  char *buff;
  unsigned int size;
  unsigned int offset;
};

struct CBuff {
  const char *buff;
  const unsigned int size;
  unsigned int offset;
  CBuff const *next;
};

#endif /* CODEC_BUFF_H */
