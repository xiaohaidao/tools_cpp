
#include "stun/StunCommon.h"

#include <stdio.h>
#include <string.h>

#include "sockets/SocketAddr.h"
#include "utils/random.h"

namespace {

#define CHECK_OFFSET_RET(buff, remain_size, offset)                            \
  do {                                                                         \
    if (offset < 0) {                                                          \
      return offset;                                                           \
    }                                                                          \
    if (remain_size < offset) {                                                \
      return -2;                                                               \
    }                                                                          \
    buff += offset;                                                            \
    remain_size -= offset;                                                     \
  } while (false)

int parse_attribute(const char *buff, size_t buff_size, SocketAddr &addr,
                    int &err_nm) {
  int offset = 0;
  const char *b = buff;
  int remain_size = buff_size;

  err_nm = 0;

  while (remain_size >= sizeof(StunAttribute)) {
    StunAttribute attr = {};
    offset = stun_read_attribute(b, remain_size, &attr);
    CHECK_OFFSET_RET(b, remain_size, offset);

    int ret = 0;
    MappedAddress address = {};
    ErrorCode ec = {};
    switch ((AttributeType)attr.type) {
    case kMappedAddress:
      ret = stun_read_address(b, remain_size, &address);
      if (ret != attr.length) {
        // printf("read address length error %d\n", ret);
      }
      stun_address_to_socket(&address, &addr);
      break;
    case kXorMappedAddress:
      ret = stun_read_xor_address(b, remain_size, &address);
      if (ret != attr.length) {
        // printf("read xor address length error %d\n", ret);
      }
      stun_address_to_socket(&address, &addr);
      break;
    case kErrorCode:
      ret = stun_read_error(b, remain_size, &ec);
      if (ret != attr.length) {
        // printf("read error length error %d\n", ret);
      }
      err_nm = ec.class_type * 100 + ec.number;
      break;

    default:
      break;
    }
    CHECK_OFFSET_RET(b, remain_size, attr.length);
  }
  return buff_size - remain_size;
}

int get_error_size(const ErrorCode &ec) { return strlen(stun_error_str(ec)); }

} // namespace

StunCommon::StunCommon()
    : transaction_id_{(unsigned int)randNum(), (unsigned int)randNum(),
                      (unsigned int)randNum()} {}

int StunCommon::request(char *buff, size_t buff_size, const ErrorCode &ec) {
  int offset = 0;
  char *b = buff;
  int remain_size = buff_size;

  int str_size = get_error_size(ec);
  StunAttribute attribute = {};
  attribute.type = kErrorCode;
  attribute.length = sizeof(ErrorCode) + str_size;

  offset = stun_set_attribute(b, remain_size, &attribute);
  CHECK_OFFSET_RET(b, remain_size, offset);

  offset = stun_set_error(b, remain_size, &ec, stun_error_str(ec));
  CHECK_OFFSET_RET(b, remain_size, offset);

  /*
  {
    {kMappedAddress, MappedAddress}
    {kErrorCode, ErrorCode}
    {kXorMappedAddress, MappedAddress}

  }
  */
  return buff_size - remain_size;
}
int StunCommon::request(char *buff, size_t buff_size, int method) {
  int offset = 0;
  char *b = buff;
  int remain_size = buff_size;
  // step 1
  StunHead head = {};
  // head.message_type.c0 = 0;
  // head.message_type.c1 = 0;
  // head.message_type.zero1 = 0;
  // head.message_type.zero2 = 0;
  // head.message_type.short_type = 0x01; // Binding Request
  head.message_type.short_type = method;
  head.magic_cookie = MAGIC_COOKIE;
  head.transaction_id[0] = transaction_id_[0];
  head.transaction_id[1] = transaction_id_[1];
  head.transaction_id[2] = transaction_id_[2];

  offset = stun_set_head(b, remain_size, &head);
  CHECK_OFFSET_RET(b, remain_size, offset);

  head.message_length = buff_size - remain_size - sizeof(head);
  stun_set_head(buff, buff_size, &head);
  return buff_size - remain_size;
}

int StunCommon::response(const char *buff, size_t buff_size, int &error,
                         SocketAddr &addr) {
  int offset = 0;
  const char *b = buff;
  int remain_size = buff_size;

  // head
  StunHead head = {};
  offset = stun_read_head(b, remain_size, &head);
  CHECK_OFFSET_RET(b, remain_size, offset);
  if (head.transaction_id[0] != transaction_id_[0] ||
      head.transaction_id[1] != transaction_id_[1] ||
      head.transaction_id[2] != transaction_id_[2]) {

    // printf("head id error %u %u %u : %u %u %u\n", head.transaction_id[0],
    //        head.transaction_id[1], head.transaction_id[2],
    //        transaction_id_[0], transaction_id_[1], transaction_id_[2]

    // );
  }

  // attribute
  offset = parse_attribute(b, remain_size, addr, error);
  CHECK_OFFSET_RET(b, remain_size, offset);

  if (remain_size > 0) {
    // printf("parse size has remain %d\n", remain_size);
  }

  return buff_size - remain_size;
}
