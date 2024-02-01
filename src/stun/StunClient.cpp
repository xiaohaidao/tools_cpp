
#include "stun/StunClient.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>

#include "sockets/SocketAddr.h"
#include "sockets/TcpListener.h"
#include "sockets/UdpSocket.h"
#include "utils/random.h"

using namespace std::placeholders;

namespace {

template <typename T> struct C {
  C(T &t) : s(t) {}
  ~C() {
    std::error_code ec;
    s.close(ec);
  }
  T &s;
};

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

int tcp_call(TcpStream &tcp, const char *buff, size_t len, char *recv_buff,
             int &recv_size) {
  std::error_code ec;
  if (tcp.write(buff, (size_t)len, ec) <= 0) {
    return -1;
  }
  int ret = tcp.read(recv_buff, recv_size, ec);
  if (ret < 0) {
    return -1;
  }
  recv_size = ret;
  return 0;
}

int udp_call(UdpSocket &udp, const SocketAddr &to, const char *buff, size_t len,
             char *recv_buff, int &recv_size) {

  std::error_code ec;
  if (udp.send_to(buff, (size_t)len, to, ec) <= 0) {
    return -1;
  }
  auto pair = udp.recv_from(recv_buff, recv_size, ec);
  int ret = pair.first;
  if (ret < 0 || ec) {
    return -1;
  }
  recv_size = ret;
  return 0;
}

/**
~~~
                        +--------+
                        |  Test  |
                        |   I    |
                        +--------+
                             |
                             |
                             V
                            /\              /\
                         N /  \ Y          /  \ Y             +--------+
          UDP     <-------/Resp\--------->/ IP \------------->|  Test  |
          Blocked         \ ?  /          \Same/              |   II   |
                           \  /            \? /               +--------+
                            \/              \/                    |
                                             | N                  |
                                             |                    V
                                             V                    /\
                                         +--------+  Sym.      N /  \
                                         |  Test  |  UDP    <---/Resp\
                                         |   II   |  Firewall   \ ?  /
                                         +--------+              \  /
                                             |                    \/
                                             V                     |Y
                  /\                         /\                    |
   Symmetric  N  /  \       +--------+   N  /  \                   V
      NAT  <--- / IP \<-----|  Test  |<--- /Resp\               Open
                \Same/      |   I    |     \ ?  /               Internet
                 \? /       +--------+      \  /
                  \/                         \/
                  |                           |Y
                  |                           |
                  |                           V
                  |                           Full
                  |                           Cone
                  V              /\
              +--------+        /  \ Y
              |  Test  |------>/Resp\---->Restricted
              |   III  |       \ ?  /
              +--------+        \  /
                                 \/
                                  |N
                                  |       Port
                                  +------>Restricted

                 Figure 2: Flow for type discovery process
~~~
*/
template <typename T>
int test1(T &call, StunStruct &stun, const char *buff, int buff_size,
          SocketAddr &addr, SocketAddr &change_addr, bool initial = true) {

  // auto call = std::bind(tcp_call, tcp, _1, _2, _3, _4);

  if (initial) {
    if (stun_init_struct(buff, buff_size, &stun, 0x01) < 0) {
      return -1;
    }
  }
  int len = 0;
  if ((len = stun_get_struct_len(&stun)) < 0) {
    return -1;
  }

  char recv_buff[512];
  int recv_size = sizeof(recv_buff);
  if (call((char *)stun.head, len, recv_buff, recv_size) < 0) {
    return -1;
  }

  StunClient recv(recv_buff, (size_t)recv_size);
  if (recv.get_socket_address(addr) < 0) {
    return -1;
  }
  if (recv.get_other_address(change_addr) < 0) {
  }
  return 0;
}

template <typename T> int test2(T &call, StunStruct &stun, SocketAddr &addr) {
  // auto call = std::bind(tcp_call, tcp, _1, _2, _3, _4);

  ChangeRequest change = {};
  change.a_change_ip = 1;
  change.b_change_port = 1;
  if (stun_append_attribute(&stun, kChangeRequest, &change, sizeof(change)) <
      0) {
    return -1;
  }
  int len = 0;
  if ((len = stun_get_struct_len(&stun)) < 0) {
    return -1;
  }

  char recv_buff[512];
  int recv_size = sizeof(recv_buff);
  if (call((char *)stun.head, len, recv_buff, recv_size) < 0) {
    return -1;
  }
  StunClient recv(recv_buff, (size_t)recv_size);
  if (recv.get_socket_address(addr) < 0) {
    return -1;
  }
  return 0;
}

template <typename T> int test3(T &call, StunStruct &stun, SocketAddr &addr) {
  // auto call = std::bind(tcp_call, tcp, _1, _2, _3, _4);

  ChangeRequest change = {};
  change.b_change_port = 1;
  if (stun_append_attribute(&stun, kChangeRequest, &change, sizeof(change)) <
      0) {
    return -1;
  }
  int len = 0;
  if ((len = stun_get_struct_len(&stun)) < 0) {
    return -1;
  }

  char recv_buff[512];
  int recv_size = sizeof(recv_buff);
  if (call((char *)stun.head, len, recv_buff, recv_size) < 0) {
    return -1;
  }
  StunClient recv(recv_buff, (size_t)recv_size);
  if (recv.get_socket_address(addr) < 0) {
    return -1;
  }
  return 0;
}

enum NatType {
  kNone,
  kOpenInternet,
  kFirewall,
  kFullCone,
  kRestrictedCone,
  kPortRestrictedCone,
  kSymmetricNAT
};

const char *get_nat_type(NatType type) {
  switch (type) {
  case kNone:
    return "None";
  case kOpenInternet:
    return "OpenInternet";
  case kFirewall:
    return "Firewall";
  case kFullCone:
    return "FullCone";
  case kRestrictedCone:
    return "RestrictedCone";
  case kPortRestrictedCone:
    return "PortRestrictedCone";
  case kSymmetricNAT:
    return "SymmetricNAT";
  default:
    return "Error NatType";
  }
}

NatType rfc3478(const char *server, unsigned short port,
                unsigned short bind_port) {
  char p[16] = {};
  snprintf(p, sizeof(p), "%u", port);
  SocketAddr to(server, p);
  snprintf(p, sizeof(p), "%u", bind_port);

  /*
  std::error_code ec;
  // TcpStream tcp = tcp.connect(to, ec);
  TcpStream tcp = TcpListener::bind_port(p, ec);
  tcp.connected(to, ec);
  if (ec) {
    return kNone;
  }
  C<TcpStream> c(tcp);
  auto call = std::bind(tcp_call, tcp, _1, _2, _3, _4);
  auto &socket = tcp;
  */
  // udp
  std::error_code ec;
  UdpSocket udp = udp.bind(p, ec);
  if (ec) {
    return kNone;
  }
  C<UdpSocket> c(udp);
  std::function<int(const char *, size_t, char *, int &)> call =
      std::bind(udp_call, udp, to, _1, _2, _3, _4);
  auto &socket = udp;
  // udp end

  char buff[512] = {};
  StunStruct stun;

  socket.set_write_timeout(5000, ec);
  socket.set_read_timeout(5000, ec);

  /* test 1 */
  SocketAddr addr1 = {};
  SocketAddr change_addr = {};
  if (test1(call, stun, buff, sizeof(buff), addr1, change_addr)) {
    return kNone;
  }
  SocketAddr local_addr =
      SocketAddr::get_local_socket(socket.native_handle(), ec);
  bool is_open = false;
  if (strcmp(local_addr.get_ip(), addr1.get_ip()) == 0 &&
      local_addr.get_port() == addr1.get_port()) {
    is_open = true;
  }

  /* test 2 */
  SocketAddr addr2 = {};
  if (test2(call, stun, addr2)) {
    if (is_open) {
      return kFirewall;
    }
    return kSymmetricNAT;
  } else {
    return is_open ? kOpenInternet : kFullCone;
  }

  /* test 1 and 3 */
  SocketAddr addr21 = {};
  if (test1(call, stun, buff, sizeof(buff), addr21, change_addr)) {
    return kNone;
  }
  if (strcmp(addr1.get_ip(), addr21.get_ip()) != 0) {
    return kSymmetricNAT;
  }
  SocketAddr addr22 = {};
  if (test3(call, stun, addr22)) {
    return kPortRestrictedCone;
  }
  return kRestrictedCone;
}

} /* namespace */

StunClient::StunClient() : stun_({}) {}

StunClient::StunClient(const char *buff, size_t buff_size) : stun_({}) {
  stun_get_struct(buff, (int)buff_size, &stun_);
}

int StunClient::get_socket_address(SocketAddr &address) {
  ErrorCode ec;
  return get_socket_address(address, ec);
}

int StunClient::get_socket_address(SocketAddr &address, ErrorCode &ec) {
  stun_set_begin(&stun_);
  do {
    AttributeType type;
    if (stun_get_current_type(&stun_, &type)) {
      return -1;
    }
    if (type == kXorMappedAddress || type == kMappedAddress) {
      MappedAddress addr = {};
      if (!stun_get_current_attribute(&stun_, &addr, sizeof(addr))) {
        addr = type == kXorMappedAddress ? stun_get_xor_address(&addr) : addr;
        stun_address_to_socket(&addr, &address);
        return 0;
      }
    }
    if (type == kErrorCode &&
        stun_get_current_attribute(&stun_, &ec, sizeof(ec)) == 0) {
    }
  } while (!stun_next(&stun_));
  return -1;
}

int StunClient::get_other_address(SocketAddr &address) {
  stun_set_begin(&stun_);
  do {
    AttributeType type;
    if (stun_get_current_type(&stun_, &type)) {
      return -1;
    }
    if (type == kChangedAddress || type == kOtherAddress) {
      MappedAddress addr = {};
      if (!stun_get_current_attribute(&stun_, &addr, sizeof(addr))) {
        stun_address_to_socket(&addr, &address);
        return 0;
      }
    }
  } while (!stun_next(&stun_));
  return -1;
}

int StunClient::get_error_code(ErrorCode &ec) {
  stun_set_begin(&stun_);
  do {
    AttributeType type;
    if (stun_get_current_type(&stun_, &type)) {
      return -1;
    }
    if (type == kErrorCode &&
        stun_get_current_attribute(&stun_, &ec, sizeof(ec)) == 0) {

      return 0;
    }
  } while (!stun_next(&stun_));
  return -1;
}

StunClient::NatType StunClient::check_tcp_nat_type(const char *server,
                                                   unsigned short port,
                                                   unsigned short bind_port,
                                                   SocketAddr &outside) {
  char p[16] = {};
  snprintf(p, sizeof(p), "%u", port);
  SocketAddr to(server, p);
  bind_port =
      bind_port > 1024 ? bind_port : rand_num() % (65535 - 49152) + 49152;
  snprintf(p, sizeof(p), "%u", bind_port);

  std::error_code ec;
  // TcpStream tcp = tcp.connect(to, ec);
  TcpStream tcp = TcpListener::bind_port(p, ec);
  tcp.connected(to, ec);
  if (ec) {
    return kNone;
  }
  C<TcpStream> c(tcp);

  std::function<int(const char *, size_t, char *, int &)> call =
      std::bind(tcp_call, tcp, _1, _2, _3, _4);
  auto &socket = tcp;

#define CALL_CHECK_NAT1()                                                      \
  char buff[512] = {};                                                         \
  StunStruct stun;                                                             \
                                                                               \
  socket.set_write_timeout(5000, ec);                                          \
  socket.set_read_timeout(5000, ec);                                           \
                                                                               \
  /* test 1 */                                                                 \
  SocketAddr addr1 = {};                                                       \
  SocketAddr change_addr = {};                                                 \
  if (test1(call, stun, buff, sizeof(buff), addr1, change_addr)) {             \
    return kNone;                                                              \
  }                                                                            \
  SocketAddr local_addr =                                                      \
      SocketAddr::get_local_socket(socket.native_handle(), ec);                \
  if (strcmp(local_addr.get_ip(), addr1.get_ip()) == 0 &&                      \
      local_addr.get_port() == addr1.get_port()) {                             \
    outside = addr1;                                                           \
    return kOpen;                                                              \
  }                                                                            \
  /* step 2 test 2 by test 1 */                                                \
  SocketAddr to2 = change_addr;                                                \
  if (change_addr.get_port() == 0) {                                           \
    outside = addr1;                                                           \
    return kNone;                                                              \
  }

#define CALL_CHECK_NAT2()                                                      \
  SocketAddr addr2 = {};                                                       \
  if (test1(call, stun, buff, sizeof(buff), addr2, change_addr, false)) {      \
    return kStrict;                                                            \
  }                                                                            \
  if (strcmp(addr2.get_ip(), addr1.get_ip()) == 0 &&                           \
      addr2.get_port() == addr1.get_port()) {                                  \
    outside = addr1;                                                           \
    return kModerate;                                                          \
  }                                                                            \
  return kStrict;

  CALL_CHECK_NAT1()

  TcpStream tcp2 = TcpListener::bind_port(p, ec);
  tcp2.connected(to2, ec);
  if (ec) {
    outside = addr1;
    return kNone;
  }
  C<TcpStream> c2(tcp2);
  call = std::bind(tcp_call, tcp2, _1, _2, _3, _4);

  CALL_CHECK_NAT2()
}

StunClient::NatType StunClient::check_udp_nat_type(const char *server,
                                                   unsigned short port,
                                                   unsigned short bind_port,
                                                   SocketAddr &outside) {

  char p[16] = {};
  snprintf(p, sizeof(p), "%u", port);
  SocketAddr to(server, p);
  snprintf(p, sizeof(p), "%u", bind_port);

  std::error_code ec;
  UdpSocket udp =
      bind_port > 1024 ? UdpSocket::bind(p, ec) : UdpSocket::create(kIpV4, ec);
  if (ec) {
    return kNone;
  }
  C<UdpSocket> c(udp);

  std::function<int(const char *, size_t, char *, int &)> call =
      std::bind(udp_call, udp, to, _1, _2, _3, _4);
  auto &socket = udp;

  CALL_CHECK_NAT1()

  call = std::bind(udp_call, udp, to2, _1, _2, _3, _4);

  CALL_CHECK_NAT2()
}

const char *StunClient::get_nat_type(NatType type) {
  switch (type) {
  case kNone:
    return "None";
  case kOpen:
    return "OpenInternet";
  case kModerate:
    return "Moderate";
  case kStrict:
    return "Strict";
  default:
    return "Error NatType";
  }
}
