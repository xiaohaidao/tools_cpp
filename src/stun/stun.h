
#ifndef STUN_STUN_H
#define STUN_STUN_H

/**
 * @file Stun.h
 * @brief ceuite
 *
 */
#include <stddef.h>

#include "StunType.h"

struct StunHead;
class SocketAddr;

#ifdef __cplusplus
extern "C" {
#endif

struct StunStruct {
  void *head;
  void *iter;
  int buff_size;
};

//  The Message Types can take on the following values:
//     0x0001  :  Binding Request
//     0x0101  :  Binding Response
//     0x0111  :  Binding Error Response
//     0x0002  :  Shared Secret Request
//     0x0102  :  Shared Secret Response
//     0x0112  :  Shared Secret Error Response
int stun_init_struct(const char *buff, int size, StunStruct *data, int method);
int stun_get_struct(const char *buff, int size, StunStruct *data);
int stun_get_struct_len(const StunStruct *data);
int stun_compare_id(const StunStruct *data, const StunStruct *other);

int stun_append_attribute(StunStruct *data, AttributeType type,
                          const void *attribute, int size);

int stun_get_current_type(const StunStruct *data, AttributeType *type);
int stun_get_current_attribute(const StunStruct *data, void *attribute,
                               int size);
int stun_has_next(const StunStruct *data);
int stun_next(StunStruct *data);
int stun_set_begin(StunStruct *data);

MappedAddress stun_get_xor_address(const MappedAddress *addr);

const char *stun_error_str(const ErrorCode &ec);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STUN_STUN_H
