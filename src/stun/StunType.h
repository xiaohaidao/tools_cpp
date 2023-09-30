
#ifndef STUN_STUNDEFINEPRI_H
#define STUN_STUNDEFINEPRI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 *
 * @brief
 *
 * reference:
 * - https://datatracker.ietf.org/doc/html/rfc3489
 * - https://datatracker.ietf.org/doc/html/rfc5389
 * - https://datatracker.ietf.org/doc/html/rfc8489
 *
 */

/**
~~~
big-endian
                       0                 1
                       2  3  4 5 6 7 8 9 0 1 2 3 4 5
                      +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
                      |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
                      |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
                      +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
                Figure 3: Format of STUN Message Type Field

   The Message Types can take on the following values:
      0x0001  :  Binding Request
      0x0101  :  Binding Response
      0x0111  :  Binding Error Response
      0x0002  :  Shared Secret Request
      0x0102  :  Shared Secret Response
      0x0112  :  Shared Secret Error Response
~~~
size 2byte
~~~
C1C0, 0b00 request 0b01 indication 0b10 success response 0b11 error response
   #define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
   #define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
   #define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
   #define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)
~~~
*/
union MessageType {
  struct {
    unsigned short m0 : 1;
    unsigned short m1 : 1;
    unsigned short m2 : 1;
    unsigned short m3 : 1;
    unsigned short c0 : 1;
    unsigned short m4 : 1;
    unsigned short m5 : 1;
    unsigned short m6 : 1;
    unsigned short c1 : 1;
    unsigned short m7 : 1;
    unsigned short m8 : 1;
    unsigned short m9 : 1;
    unsigned short m10 : 1;
    unsigned short m11 : 1;
    unsigned short zero1 : 1;
    unsigned short zero2 : 1;
  };
  unsigned short short_type;
};

static_assert(sizeof(MessageType) == 2, "MessageType type error!");

/**
~~~
big-endian
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0 0|     STUN Message Type     |         Message Length        |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                         Magic Cookie                          |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                                                               |
     |                     Transaction ID (96 bits)                  |
     |                                                               |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                  Figure 2: Format of STUN Message Header
~~~
size 20 byte
*/
struct StunHead {
  MessageType message_type;
  unsigned short message_length;
  unsigned int magic_cookie;      //< Fixed value 0x2112A442
  unsigned int transaction_id[3]; //< RFC3489 is 128bit hasn't magic cookie
};

static_assert(sizeof(struct StunHead) == 20, "StunHead type error!");

static constexpr unsigned int MAGIC_COOKIE = 0x2112A442;

/**
~~~
                                         Binding  Shared  Shared  Shared
                       Binding  Binding  Error    Secret  Secret  Secret
   Att.                Req.     Resp.    Resp.    Req.    Resp.   Error
                                                                  Resp.
   _____________________________________________________________________
   MAPPED-ADDRESS      N/A      M        N/A      N/A     N/A     N/A
   RESPONSE-ADDRESS    O        N/A      N/A      N/A     N/A     N/A
   CHANGE-REQUEST      O        N/A      N/A      N/A     N/A     N/A
   SOURCE-ADDRESS      N/A      M        N/A      N/A     N/A     N/A
   CHANGED-ADDRESS     N/A      M        N/A      N/A     N/A     N/A
   USERNAME            O        N/A      N/A      N/A     M       N/A
   PASSWORD            N/A      N/A      N/A      N/A     M       N/A
   MESSAGE-INTEGRITY   O        O        N/A      N/A     N/A     N/A
   ERROR-CODE          N/A      N/A      M        N/A     N/A     M
   UNKNOWN-ATTRIBUTES  N/A      N/A      C        N/A     N/A     C
   REFLECTED-FROM      N/A      C        N/A      N/A     N/A     N/A

   Table 2: Summary of Attributes
   M: mandatory
   O: optional
   C: conditional
   N/A: not applicable
~~~
~~~
   Comprehension-required range (0x0000-0x7FFF):
   0x0000: Reserved
   0x0001: MAPPED-ADDRESS
   0x0002: Reserved; was RESPONSE-ADDRESS prior to [RFC5389]
   0x0003: Reserved; was CHANGE-REQUEST prior to [RFC5389]
   0x0004: Reserved; was SOURCE-ADDRESS prior to [RFC5389]
   0x0005: Reserved; was CHANGED-ADDRESS prior to [RFC5389]
   0x0006: USERNAME
   0x0007: Reserved; was PASSWORD prior to [RFC5389]
   0x0008: MESSAGE-INTEGRITY
   0x0009: ERROR-CODE
   0x000A: UNKNOWN-ATTRIBUTES
   0x000B: Reserved; was REFLECTED-FROM prior to [RFC5389]
   0x0014: REALM
   0x0015: NONCE
   0x0020: XOR-MAPPED-ADDRESS

   Comprehension-optional range (0x8000-0xFFFF)
   0x8022: SOFTWARE
   0x8023: ALTERNATE-SERVER
   0x8028: FINGERPRINT

   // New Attributes
   Comprehension-required range (0x0000-0x7FFF):
   0x001C: MESSAGE-INTEGRITY-SHA256
   0x001D: PASSWORD-ALGORITHM
   0x001E: USERHASH

   Comprehension-optional range (0x8000-0xFFFF)
   0x8002: PASSWORD-ALGORITHMS
   0x8003: ALTERNATE-DOMAIN
~~~
*/
enum AttributeType {
  kMappedAddress = 0x0001,
  kChangeRequest = 0x0003,
  kSourceAddress = 0x0004,
  kChangedAddress = 0x0005,
  kUsername = 0x0006,
  kMessageIntegrity = 0x0008,
  kErrorCode = 0x0009,
  kUnkownAttributes = 0x000A,
  kRealm = 0x0014,
  kNonce = 0x0015,
  kXorMappedAddress = 0x0020,

  kSoftware = 0x8022,
  kAlternateServer = 0x8023,
  kFingerprint = 0x8028,
  kOtherAddress = 0x802C,
};

/**
~~~
big-endian
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |         Type                  |            Length             |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                         Value (variable)                ....
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    Figure 4: Format of STUN Attributes
~~~
*/
struct StunAttribute {
  unsigned short type;
  unsigned short length;
  // value;
};

static_assert(sizeof(StunAttribute) == 4, "StunAttribute type error!");

/**
~~~
big-endian
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0 0 0 0 0 0 0 0|    Family     |           Port                |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                                                               |
     |                 Address (32 bits or 128 bits)                 |
     |                                                               |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
               Figure 5: Format of MAPPED-ADDRESS Attribute
~~~
*/
struct MappedAddress {
  unsigned short family;
  unsigned short port;
  union {
    unsigned int ip_addr_in;
    unsigned int ip_addr_in6[4];
  };
};

static_assert(sizeof(MappedAddress) == 20, "MappedAddress type error!");

/**
~~~
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |           Reserved, should be 0         |Class|     Number    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |      Reason Phrase (variable)                                ..
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                      Figure 7: ERROR-CODE Attribute
~~~
*/
struct ErrorCode {
  int /*zeros*/ : 16;
  int class_type : 8;
  int number : 8;
};

static_assert(sizeof(ErrorCode) == 4, "ErrorCode type error!");

/**
~~~
big-endian
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 A B 0|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~~~
*/
struct ChangeRequest {
  int zeros : 24;
  int /*zero*/ : 1;
  int b_change_port : 1;
  int a_change_ip : 1;
  int /*reserver*/ : 5;
};

static_assert(sizeof(ChangeRequest) == 4, "ChangeRequest type error!");

#ifdef __cplusplus
} // extern "C"
#endif

#endif // STUN_STUNDEFINEPRI_H