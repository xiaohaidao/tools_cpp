
/**
 * @brief
 *
 * reference https://datatracker.ietf.org/doc/html/rfc3489
 * reference https://datatracker.ietf.org/doc/html/rfc5389
 * reference https://datatracker.ietf.org/doc/html/rfc8489
 */

#include "stun/Stun.h"

#include <string.h>

// #include "stun/StunDefinePri.h"
#include "sockets/SocketAddr.h"
#include "utils/random.h"

namespace {

void netToHost(MappedAddress &v) {
  v.family = sockets::netToHost(v.family);
  v.port = sockets::netToHost(v.port);
  v.ip_addr_in6[0] = sockets::netToHost(v.ip_addr_in6[0]);
  v.ip_addr_in6[1] = sockets::netToHost(v.ip_addr_in6[1]);
  v.ip_addr_in6[2] = sockets::netToHost(v.ip_addr_in6[2]);
  v.ip_addr_in6[3] = sockets::netToHost(v.ip_addr_in6[3]);
}

void netToHost(StunAttribute &v) {
  v.type = sockets::netToHost(v.type);
  v.length = sockets::netToHost(v.length);
}

void netToHost(StunHead &head) {
  head.message_type.short_type =
      sockets::netToHost(head.message_type.short_type);
  head.magic_cookie = sockets::netToHost(head.magic_cookie);
  head.message_length = sockets::netToHost(head.message_length);
  head.transaction_id[0] = sockets::netToHost(head.transaction_id[0]);
  head.transaction_id[1] = sockets::netToHost(head.transaction_id[1]);
  head.transaction_id[2] = sockets::netToHost(head.transaction_id[2]);
}

void hostToNet(MappedAddress &v) {
  v.family = sockets::hostToNet(v.family);
  v.port = sockets::hostToNet(v.port);
  v.ip_addr_in6[0] = sockets::hostToNet(v.ip_addr_in6[0]);
  v.ip_addr_in6[1] = sockets::hostToNet(v.ip_addr_in6[1]);
  v.ip_addr_in6[2] = sockets::hostToNet(v.ip_addr_in6[2]);
  v.ip_addr_in6[3] = sockets::hostToNet(v.ip_addr_in6[3]);
}

void hostToNet(StunAttribute &v) {
  v.type = sockets::hostToNet(v.type);
  v.length = sockets::hostToNet(v.length);
}

void hostToNet(StunHead &head) {
  head.message_type.short_type =
      sockets::hostToNet(head.message_type.short_type);
  head.magic_cookie = sockets::hostToNet(head.magic_cookie);
  head.message_length = sockets::hostToNet(head.message_length);
  head.transaction_id[0] = sockets::hostToNet(head.transaction_id[0]);
  head.transaction_id[1] = sockets::hostToNet(head.transaction_id[1]);
  head.transaction_id[2] = sockets::hostToNet(head.transaction_id[2]);
}

} // namespace

#ifdef __cplusplus
extern "C" {
#endif

int stun_read_head(const char *buff, size_t buff_size, StunHead *head) {
  if (buff_size < sizeof(StunHead)) {
    return -1;
  }

  memcpy(head, buff, sizeof(StunHead));
  netToHost(*head);

  if (head->message_type.zero1 != 0 || head->message_type.zero2 != 0) {

    return -1;
  }

  if (head->magic_cookie != MAGIC_COOKIE) {
    return -1;
  }

  // if (head.transaction_id[0] != transaction_id_[0] ||
  //     head.transaction_id[1] != transaction_id_[1] ||
  //     head.transaction_id[2] != transaction_id_[2]) {

  //   // return -1;
  // }

  if (head->message_length != (buff_size - sizeof(StunHead))) {
    return -1;
  }

  return sizeof(StunHead);
}

int stun_set_head(char *buff, size_t buff_size, const StunHead *head) {
  if (buff_size < sizeof(StunHead)) {
    return -1;
  }

  memcpy(buff, head, sizeof(StunHead));
  hostToNet(*reinterpret_cast<StunHead *>(buff));
  return sizeof(StunHead);
}

int stun_read_attribute(const char *buff, size_t buff_size,
                        StunAttribute *attr) {
  if (buff_size < sizeof(StunAttribute)) {
    return -1;
  }
  memcpy(attr, buff, sizeof(StunAttribute));
  netToHost(*attr);
  return sizeof(StunAttribute);
}

int stun_set_attribute(char *buff, size_t buff_size,
                       const StunAttribute *attr) {
  if (buff_size < sizeof(StunAttribute)) {
    return -1;
  }
  memcpy(buff, attr, sizeof(StunAttribute));
  hostToNet(*reinterpret_cast<StunAttribute *>(buff));
  return sizeof(StunAttribute);
}

int stun_read_error(const char *buff, size_t buff_size, ErrorCode *attr) {
  if (buff_size < sizeof(ErrorCode)) {
    return -1;
  }
  memcpy(attr, buff, sizeof(ErrorCode));
  return (int)sizeof(ErrorCode);
}

int stun_set_error(char *buff, size_t buff_size, const ErrorCode *attr,
                   const char *str) {

  size_t str_len = strlen(str);
  if (buff_size < sizeof(ErrorCode) + str_len) {
    return -1;
  }
  memcpy(buff, attr, sizeof(ErrorCode));
  memcpy(buff + sizeof(ErrorCode), str, str_len);
  return (int)(sizeof(ErrorCode) + strlen(str));
}

const char *stun_error_str(const ErrorCode &ec) {
  switch (ec.class_type * 100 + ec.number) {
  case 300:
    return "Try Alternate";
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 420:
    return "Unknown Attribute";
  case 438:
    return "Stale Nonce";
  case 500:
    return "Server Error";

  default:
    break;
  }
  return "unkown error code";
}

int stun_read_address(const char *buff, size_t buff_size, MappedAddress *attr) {
  int size = attr->family == 0x02 ? sizeof(MappedAddress)
                                  : sizeof(MappedAddress) - 3 * sizeof(int);
  if (buff_size < size) {
    return -1;
  }
  memcpy(attr, buff, size);
  netToHost(*attr);

  return size;
}

int stun_set_address(char *buff, size_t buff_size, const MappedAddress *attr) {
  int size = attr->family == 0x02 ? sizeof(MappedAddress)
                                  : sizeof(MappedAddress) - 3 * sizeof(int);
  if (buff_size < size) {
    return -1;
  }
  MappedAddress value = {};
  memcpy(&value, attr, size);
  netToHost(value);
  memcpy(buff, &value, size);
  return size;
}

int stun_read_xor_address(const char *buff, size_t buff_size,
                          MappedAddress *attr) {

  memcpy(attr, buff, sizeof(MappedAddress));
  netToHost(*attr);

  int family = attr->family;
  attr->port ^= (MAGIC_COOKIE >> 16);
  if (family == 0x02) {
    attr->ip_addr_in6[0] ^= MAGIC_COOKIE;
    attr->ip_addr_in6[1] ^= MAGIC_COOKIE;
    attr->ip_addr_in6[2] ^= MAGIC_COOKIE;
    attr->ip_addr_in6[3] ^= MAGIC_COOKIE;
  } else {
    attr->ip_addr_in ^= MAGIC_COOKIE;
  }
  hostToNet(*attr);
  return stun_read_address((const char *)attr, buff_size, attr);
}

int stun_set_xor_address(char *buff, size_t buff_size,
                         const MappedAddress *attr) {
  MappedAddress value = {};
  memcpy(&value, attr, sizeof(MappedAddress));
  value.port ^= (MAGIC_COOKIE >> 16);
  value.ip_addr_in6[0] ^= MAGIC_COOKIE;
  value.ip_addr_in6[1] ^= MAGIC_COOKIE;
  value.ip_addr_in6[2] ^= MAGIC_COOKIE;
  value.ip_addr_in6[3] ^= MAGIC_COOKIE;
  return stun_set_address(buff, buff_size, &value);
}

int stun_address_to_socket(const MappedAddress *attr, SocketAddr *s) {
  int family = attr->family;
  char ip_buff[128] = {};
  char port_buff[8] = {};
  if (family == 0x01) { // 0x01 ipv4, 0x02 ipv6
    snprintf(ip_buff, sizeof(ip_buff), "%d.%d.%d.%d",
             (uint8_t)(attr->ip_addr_in >> 24 & 0xff),
             (uint8_t)(attr->ip_addr_in >> 16 & 0xff),
             (uint8_t)(attr->ip_addr_in >> 8 & 0xff),
             (uint8_t)(attr->ip_addr_in >> 0 & 0xff));
    snprintf(port_buff, sizeof(port_buff), "%d", attr->port);
  } else if (family == 0x02) {
    snprintf(ip_buff, sizeof(ip_buff), "%x:%x:%x:%x:%x:%x:%x:%x",
             (uint16_t)(attr->ip_addr_in6[0] >> 16 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[0] >> 0 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[1] >> 16 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[1] >> 0 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[2] >> 16 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[2] >> 0 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[3] >> 16 & 0xffff),
             (uint16_t)(attr->ip_addr_in6[3] >> 0 & 0xffff));
    snprintf(port_buff, sizeof(port_buff), "%d", attr->port);
  }
  *s = SocketAddr(ip_buff, port_buff);

  return 0;
}

#ifdef __cplusplus
} // extern "C"
#endif
