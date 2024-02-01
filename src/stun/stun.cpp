
/**
 * @brief
 *
 * reference
 * - https://datatracker.ietf.org/doc/html/rfc3489
 * - https://datatracker.ietf.org/doc/html/rfc5389
 * - https://datatracker.ietf.org/doc/html/rfc8489
 *
 */

#include "stun/stun.h"

#include <cstring>

// #include "stun/StunDefinePri.h"
#include "sockets/SocketAddr.h"
#include "utils/random.h"

namespace {

void net_to_host(MappedAddress &v) {
  v.family = sockets::net_to_host(v.family);
  v.port = sockets::net_to_host(v.port);
  v.ip_addr_in6[0] = sockets::net_to_host(v.ip_addr_in6[0]);
  v.ip_addr_in6[1] = sockets::net_to_host(v.ip_addr_in6[1]);
  v.ip_addr_in6[2] = sockets::net_to_host(v.ip_addr_in6[2]);
  v.ip_addr_in6[3] = sockets::net_to_host(v.ip_addr_in6[3]);
}

void net_to_host(StunAttribute &v) {
  v.type = sockets::net_to_host(v.type);
  v.length = sockets::net_to_host(v.length);
}

void net_to_host(StunHead &head) {
  head.message_type.short_type =
      sockets::net_to_host(head.message_type.short_type);
  head.magic_cookie = sockets::net_to_host(head.magic_cookie);
  head.message_length = sockets::net_to_host(head.message_length);
  head.transaction_id[0] = sockets::net_to_host(head.transaction_id[0]);
  head.transaction_id[1] = sockets::net_to_host(head.transaction_id[1]);
  head.transaction_id[2] = sockets::net_to_host(head.transaction_id[2]);
}

void host_to_net(MappedAddress &v) {
  v.family = sockets::host_to_net(v.family);
  v.port = sockets::host_to_net(v.port);
  v.ip_addr_in6[0] = sockets::host_to_net(v.ip_addr_in6[0]);
  v.ip_addr_in6[1] = sockets::host_to_net(v.ip_addr_in6[1]);
  v.ip_addr_in6[2] = sockets::host_to_net(v.ip_addr_in6[2]);
  v.ip_addr_in6[3] = sockets::host_to_net(v.ip_addr_in6[3]);
}

void host_to_net(StunAttribute &v) {
  v.type = sockets::host_to_net(v.type);
  v.length = sockets::host_to_net(v.length);
}

void host_to_net(StunHead &head) {
  head.message_type.short_type =
      sockets::host_to_net(head.message_type.short_type);
  head.magic_cookie = sockets::host_to_net(head.magic_cookie);
  head.message_length = sockets::host_to_net(head.message_length);
  head.transaction_id[0] = sockets::host_to_net(head.transaction_id[0]);
  head.transaction_id[1] = sockets::host_to_net(head.transaction_id[1]);
  head.transaction_id[2] = sockets::host_to_net(head.transaction_id[2]);
}

} // namespace

#ifdef __cplusplus
extern "C" {
#endif

static void local_set_head(StunStruct *data, const StunHead *head) {
  StunHead net = *head;
  net_to_host(net);
  *(StunHead *)(data->head) = net;
}

static void local_set_attribute(char *buff, StunAttribute attribute,
                                const void *atrr) {

  int attr_len = attribute.length;
  host_to_net(attribute);
  memcpy(buff, &attribute, sizeof(StunAttribute));
  memcpy(buff + sizeof(StunAttribute), atrr, attr_len);
}

static StunHead local_get_head(const StunStruct *data) {
  StunHead head = {};
  head = *(StunHead *)(data->head);
  net_to_host(head);
  return head;
}

static StunAttribute local_get_attribute_by_type(AttributeType type,
                                                 int is_ipv4) {
  StunAttribute attr = {};
  attr.type = type;
  switch (type) {
  case kChangedAddress:
  case kOtherAddress:
  case kXorMappedAddress:
  case kMappedAddress:
    attr.length = sizeof(MappedAddress) - !!is_ipv4 * 12;
    break;
  case kErrorCode:
    attr.length = sizeof(ErrorCode);
    break;
  case kChangeRequest:
    attr.length = sizeof(ChangeRequest);
    break;
  case kUsername:
  case kMessageIntegrity:
  case kUnkownAttributes:
  case kRealm:
  case kNonce:
  case kSoftware:
  case kAlternateServer:
  case kFingerprint:
  default:
    break;
  }
  return attr;
}

static void *local_get_attribute_address(const void *iter) {
  return (char *)iter + sizeof(StunAttribute);
}

static StunAttribute local_get_attribute_iter(const void *iter) {
  StunAttribute attr = *(StunAttribute *)iter;
  net_to_host(attr);
  return attr;
}

static void *local_get_next_attribute_iter(const void *iter) {
  StunAttribute attr = *(StunAttribute *)iter;
  net_to_host(attr);
  return (char *)iter + attr.length + sizeof(StunAttribute);
}

static int local_get_attribute_value(const void *iter, void *value, int size) {
  StunAttribute a = local_get_attribute_iter(iter);
  // if (a.length > size) {
  //   return -1;
  // }
  switch (a.type) {
  case kChangedAddress:
  case kOtherAddress:
  case kXorMappedAddress:
  case kMappedAddress: {
    if (size != sizeof(MappedAddress) && size != sizeof(MappedAddress) - 12) {
      return -1;
    }
    MappedAddress attr = *(MappedAddress *)local_get_attribute_address(iter);
    net_to_host(attr);
    *(MappedAddress *)value = attr;
    return 0;
  }
  case kErrorCode: {
    if (size != sizeof(ErrorCode)) {
      return -1;
    }
    ErrorCode attr = *(ErrorCode *)local_get_attribute_address(iter);
    // net_to_host(attr);
    *(ErrorCode *)value = attr;
    return 0;
  }
  case kChangeRequest: {
    if (size != sizeof(ChangeRequest)) {
      return -1;
    }
    ChangeRequest attr = *(ChangeRequest *)local_get_attribute_address(iter);
    // net_to_host(attr);
    *(ChangeRequest *)value = attr;
    return 0;
  }
  case kUsername:
  case kMessageIntegrity:
  case kUnkownAttributes:
  case kRealm:
  case kNonce:
  case kSoftware:
  case kAlternateServer:
  case kFingerprint:
  default:
    break;
  }
  return -1;
}

static void *iter_begin(const StunStruct *data) {
  if (data->head == nullptr) {
    return nullptr;
  }
  return (char *)data->head + sizeof(StunHead);
}

static void *iter_end(const StunStruct *data) {
  if (data->head == nullptr) {
    return nullptr;
  }
  StunHead head = local_get_head(data);
  if (!head.message_length) {
    return iter_begin(data);
  }
  return (char *)data->head + head.message_length + sizeof(StunHead);
}

static void *iter_current(const StunStruct *data) {
  if (!data->iter) {
    return iter_begin(data);
  }
  return data->iter;
}

static void *iter_next(const StunStruct *data) {
  void *current = iter_current(data);
  void *end = iter_end(data);
  if (current == end) {
    return nullptr;
  }
  current = local_get_next_attribute_iter(current);
  if ((char *)current - (char *)data->head > data->buff_size) {
    return nullptr;
  }
  return current == end ? nullptr : current;
}

static void iter_set_current(StunStruct *data, void *current) {
  data->iter = current;
}

static int local_check_head(const StunStruct *data) {
  if (data == nullptr || data->head == nullptr) {
    return -1;
  }

  StunHead head = local_get_head(data);
  if (head.message_type.zero1 != 0 || head.message_type.zero2 != 0) {
    return -1;
  }
  if (head.magic_cookie != MAGIC_COOKIE) {
    return -1;
  }
  // if (head.transaction_id[0] != transaction_id_[0] ||
  //     head.transaction_id[1] != transaction_id_[1] ||
  //     head.transaction_id[2] != transaction_id_[2]) {

  //   // return -1;
  // }

  return 0;
}

int stun_init_struct(const char *buff, int size, StunStruct *data, int method) {
  if (size < sizeof(StunHead)) {
    return -1;
  }
  if (data == nullptr) {
    return -1;
  }

  StunStruct d = {};
  d.iter = nullptr;
  d.head = (void *)buff;
  d.buff_size = size;

  StunHead head = {};
  // head.message_type.c0 = 0;
  // head.message_type.c1 = 0;
  // head.message_type.zero1 = 0;
  // head.message_type.zero2 = 0;
  // head.message_type.short_type = 0x01; // Binding Request
  head.message_type.short_type = method;
  head.magic_cookie = MAGIC_COOKIE;
  head.transaction_id[0] = (unsigned int)rand_num();
  head.transaction_id[1] = (unsigned int)rand_num();
  head.transaction_id[2] = (unsigned int)rand_num();

  host_to_net(head);
  *(StunHead *)d.head = head;

  *data = d;
  return 0;
}

int stun_get_struct(const char *buff, int size, StunStruct *data) {
  if (size < sizeof(StunHead)) {
    return -1;
  }
  if (data == nullptr || buff == nullptr) {
    return -1;
  }
  StunStruct d = {};
  d.iter = nullptr;
  d.head = (void *)buff;
  d.buff_size = size;
  if (local_check_head(&d)) {
    return -1;
  }
  StunHead head = local_get_head(&d);
  if (size < head.message_length + sizeof(StunHead)) {
    return -1;
  }

  *data = d;
  return 0;
}

int stun_get_struct_len(const StunStruct *data) {
  if (data == nullptr || data->head == nullptr) {
    return -1;
  }

  StunHead head = local_get_head(data);
  return head.message_length + sizeof(StunHead);
}

int stun_compare_id(const StunStruct *data, const StunStruct *other) {
  if (data == nullptr || data->head == nullptr) {
    return -1;
  }
  if (other == nullptr || other->head == nullptr) {
    return -1;
  }
  StunHead head = local_get_head(data);
  StunHead other_head = local_get_head(other);
  for (size_t i = 0; i < 3; ++i) {
    if (head.transaction_id[i] != other_head.transaction_id[i]) {
      return -1;
    }
  }
  return 0;
}

int stun_append_attribute(StunStruct *data, AttributeType type,
                          const void *attribute, int size) {

  if (data == nullptr || data->head == nullptr || attribute == nullptr) {
    return -1;
  }
  StunAttribute attr = local_get_attribute_by_type(type, 1);
  StunHead head = local_get_head(data);
  if (attr.length == 0 || attr.length != size) {
    return -1;
  }
  if (attr.length + head.message_length + sizeof(StunHead) +
          sizeof(StunAttribute) >
      data->buff_size) {

    return -1;
  }

  local_set_attribute((char *)iter_end(data), attr, attribute);

  head.message_length += (attr.length + sizeof(StunAttribute));
  local_set_head(data, &head);
  return 0;
}

int stun_get_current_type(const StunStruct *data, AttributeType *type) {
  if (data == nullptr || data->head == nullptr || type == nullptr) {
    return -1;
  }
  void *current = iter_current(data);
  void *end = iter_end(data);
  if (current == end) {
    return -1;
  }
  StunAttribute attr = local_get_attribute_iter(current);
  *type = (AttributeType)attr.type;
  return 0;
}

int stun_get_current_attribute(const StunStruct *data, void *attribute,
                               int size) {

  if (data == nullptr || data->head == nullptr || attribute == nullptr) {
    return -1;
  }
  AttributeType type = {};
  if (stun_get_current_type(data, &type)) {
    return -1;
  }
  return local_get_attribute_value(iter_current(data), attribute, size);
}

int stun_has_next(const StunStruct *data) {
  if (data == nullptr || data->head == nullptr) {
    return 0;
  }

  return iter_next(data) == nullptr ? 0 : 1;
}

int stun_next(StunStruct *data) {
  if (!stun_has_next(data)) {
    return -1;
  }
  iter_set_current(data, iter_next(data));
  return 0;
}

int stun_set_begin(StunStruct *data) {
  if (data == nullptr || data->head == nullptr) {
    return -1;
  }
  iter_set_current(data, iter_begin(data));
  return 0;
}

MappedAddress stun_get_xor_address(const MappedAddress *addr) {
  MappedAddress attr = *addr;
  // int family = attr.family;
  attr.port ^= (MAGIC_COOKIE >> 16);
  // if (family == 0x02) {
  attr.ip_addr_in6[0] ^= MAGIC_COOKIE;
  attr.ip_addr_in6[1] ^= MAGIC_COOKIE;
  attr.ip_addr_in6[2] ^= MAGIC_COOKIE;
  attr.ip_addr_in6[3] ^= MAGIC_COOKIE;
  // } else {
  //   attr.ip_addr_in ^= MAGIC_COOKIE;
  // }
  return attr;
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

#ifdef __cplusplus
} // extern "C"
#endif
