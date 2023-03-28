
#ifndef STUN_STUNCOMMON_H
#define STUN_STUNCOMMON_H

#include "Stun.h"

class SocketAddr;

class StunCommon {
public:
  StunCommon();

  // step 1 and end
  int request(char *buff, size_t buff_size, int method);
  // step other
  int request(char *buff, size_t buff_size, const ErrorCode &ec);
  int request(char *buff, size_t buff_size, const MappedAddress &ec);
  int request_xor(char *buff, size_t buff_size, const MappedAddress &ec);

  int response(const char *buff, size_t size, int &error,
               SocketAddr &public_addr);

private:
  unsigned int transaction_id_[3];
};

#endif // STUN_STUNCOMMON_H
