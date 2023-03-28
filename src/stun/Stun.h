
#ifndef STUN_STUN_H
#define STUN_STUN_H

#include <stddef.h>

#include "StunType.h"

struct StunHead;
class SocketAddr;

#ifdef __cplusplus
extern "C" {
#endif

int stun_read_head(const char *buff, size_t buff_size, StunHead *head);
int stun_set_head(char *buff, size_t buff_size, const StunHead *head);

int stun_read_attribute(const char *buff, size_t buff_size,
                        StunAttribute *attr);
int stun_set_attribute(char *buff, size_t buff_size, const StunAttribute *attr);

const char *stun_error_str(const ErrorCode &ec);
int stun_read_error(const char *buff, size_t buff_size, ErrorCode *attr);
int stun_set_error(char *buff, size_t buff_size, const ErrorCode *attr,
                   const char *str);

int stun_read_address(const char *buff, size_t buff_size, MappedAddress *attr);
int stun_set_address(char *buff, size_t buff_size, const MappedAddress *attr);

int stun_read_xor_address(const char *buff, size_t buff_size,
                          MappedAddress *attr);
int stun_set_xor_address(char *buff, size_t buff_size,
                         const MappedAddress *attr);

int stun_address_to_socket(const MappedAddress *attr, SocketAddr *s);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STUN_STUN_H
