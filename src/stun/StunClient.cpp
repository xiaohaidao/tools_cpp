
#include "stun/StunClient.h"

int StunClient::request(char *buff, size_t buff_size) {
  return StunCommon::request(buff, buff_size, 0x01);
}
