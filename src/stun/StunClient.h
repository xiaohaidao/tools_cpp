
#ifndef STUN_STUNCLIENT_H
#define STUN_STUNCLIENT_H

#include "stun/StunCommon.h"

class StunClient : public StunCommon {
public:
  StunClient() = default;

  // return -1 when error
  int request(char *buff, size_t buff_size);

};

#endif // STUN_STUNCLIENT_H
