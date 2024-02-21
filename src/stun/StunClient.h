
#ifndef STUN_STUNCLIENT_H
#define STUN_STUNCLIENT_H

#include "stun/stun.h"

class TcpStream;
class UdpSocket;

class StunClient {
public:
  StunClient();
  StunClient(const char *buff, size_t buff_size);

  int get_socket_address(SocketAddr &address);
  int get_socket_address(SocketAddr &address, ErrorCode &ec);
  int get_other_address(SocketAddr &address);
  int get_error_code(ErrorCode &ec);

  // Open Internet, Symmetric Firewall
  // Full Cone, Restricted Cone, Port-Restricted Cone,
  // Symmetric NAT
  enum NatType { kNone, kOpen, kModerate, kStrict };
  static const char *get_nat_type(NatType type);

  static NatType check_tcp_nat_type(unsigned short port, const char *server,
                                    unsigned short bind_port,
                                    SocketAddr &outside);
  static NatType check_udp_nat_type(unsigned short port, const char *server,
                                    unsigned short bind_port,
                                    SocketAddr &outside);

private:
  StunStruct stun_;

}; /* class StunClient */

#endif // STUN_STUNCLIENT_H
