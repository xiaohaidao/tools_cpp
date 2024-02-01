
#include "gtest/gtest.h"

#include "sockets/TcpListener.h"
#include "sockets/TcpStream.h"
#include "sockets/UdpSocket.h"
#include "stun/StunClient.h"
#include "utils/log.h"

namespace stunclient {

const char *stun_server_tcp_no_support_other[] = {
    "fwa.lifesizecloud.com", "stun.isp.net.au", "stun.voip.blackberry.com",
    "stun.sipnet.com", nullptr};

const char *stun_server_tcp[] = {"stun.freeswitch.org",
                                 "stun.nextcloud.com",
                                 "stunserver.stunprotocol.org",
                                 "stun.radiojar.com",
                                 "stun.sonetel.com",
                                 "stun.voipgate.com",
                                 nullptr};

const char *stun_server_udp[] = {"stun.miwifi.com", "stun.qq.com", nullptr};

TEST(StunTest, StunResponse1) {
  char buff[] = {
      "\x01\x01\x00\x30\x21\x12\xa4\x42\x5a\xdc\x2f\xac\x27\x27\xdb\xbc"
      "\x59\xe8\x2e\x6d\x00\x01\x00\x08\x00\x01\x08\xd8\x71\x58\x0d\x84"
      "\x80\x2b\x00\x08\x00\x01\x0d\x96\x12\xbf\xdf\x0c\x80\x2c\x00\x08"
      "\x00\x01\x0d\x97\x12\xdb\x6e\x12\x00\x20\x00\x08\x00\x01\x29\xca"
      "\x50\x4a\xa9\xc6"}; // 113.88.13.132:2264

  size_t n = sizeof(buff) - 1;
  char out_buff[512] = {};
  for (size_t i = 0; i < n; ++i) {
    snprintf(out_buff + strlen(out_buff), sizeof(out_buff) - strlen(out_buff),
             "%02x ", (uint8_t)buff[i]);
  }
  LOG_TRACE("response buff[%d]: %02s", n, out_buff);
  SocketAddr addr = {};
  StunClient client(buff, n);
  int res = client.get_socket_address(addr);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (res >= 0) {
    EXPECT_TRUE(strcmp(addr.get_ip(), "113.88.13.132") == 0 &&
                addr.get_port() == 2264)
        << "self buff ip port " << addr.get_ip() << ":" << addr.get_port()
        << " vs 113.88.13.132:2264";
  }
}

TEST(StunTest, StunResponse2) {
  char buff[] =
      "\x01\x01\x00\x24\x21\x12\xa4\x42\x3d\xcd\x74\xd3\x8f\xc8\x8f\xb4"
      "\x7d\x4f\xc5\x72\x00\x01\x00\x08\x00\x01\x0a\x98\x71\x57\xa3\xb9"
      "\x00\x04\x00\x08\x00\x01\x0d\x96\xaf\xb2\xb0\xac\x00\x05\x00\x08"
      "\x00\x01\x1f\x40\x2b\x8b\x12\xea"; // 113.87.163.185:2712

  size_t n = sizeof(buff) - 1;
  SocketAddr addr = {};
  StunClient client(buff, n);
  int res = client.get_socket_address(addr);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (res >= 0) {
    EXPECT_TRUE(strcmp(addr.get_ip(), "113.87.163.185") == 0 &&
                addr.get_port() == 2712)
        << "self buff ip port " << addr.get_ip() << ":" << addr.get_port()
        << " vs 113.87.163.185:2712";
    ;
  }
}

TEST(StunTest, StunResponseErrorCode) {
  char buff[] =
      "\x01\x11\x00\x48\x21\x12\xa4\x42\xa9\x02\x47\x25\x11\x22\x60\xae"
      "\x09\x5a\x9a\x04\x00\x09\x00\x34\x00\x00\x04\x14\x57\x72\x6f\x6e"
      "\x67\x20\x72\x65\x71\x75\x65\x73\x74\x3a\x20\x61\x70\x70\x6c\x69"
      "\x63\x61\x62\x6c\x65\x20\x6f\x6e\x6c\x79\x20\x74\x6f\x20\x55\x44"
      "\x50\x20\x70\x72\x6f\x74\x6f\x63\x6f\x6c\x00\x00\x80\x22\x00\x04"
      "\x4e\x6f\x6e\x65\x80\x28\x00\x04\xf5\x5d\x47\x4d"; // error 420

  size_t n = sizeof(buff) - 1;
  ErrorCode ec = {};
  StunClient client(buff, n);
  int res = client.get_error_code(ec);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (res >= 0) {
    EXPECT_TRUE(ec.class_type == 4 && ec.number == 20)
        << "self buff error code " << ec.class_type << "*100:" << ec.number
        << " vs 4*100:20";
  }
}

TEST(StunTest, StunClientUdpCheckNat) {
  SocketAddr hole;
  auto type = StunClient::check_udp_nat_type(stun_server_udp[1], 3478, 0, hole);
  if (type == StunClient::kNone && hole.get_port() != 0) {
    LOG_TRACE("udp stun server not support OTHER-ADDRESS");
  }
  LOG_TRACE("udp nat type is %s", StunClient::get_nat_type(type));
  if (type == StunClient::kModerate || type == StunClient::kOpen) {
    LOG_TRACE("udp hole address %s:%d", hole.get_ip(), hole.get_port());
  }
}

TEST(StunTest, StunClientTcpCheckNat) {
  SocketAddr hole = {};
  auto type = StunClient::check_tcp_nat_type(stun_server_tcp[2], 3478, 0, hole);
  if (type == StunClient::kNone && hole.get_port() != 0) {
    LOG_TRACE("tcp stun server not support OTHER-ADDRESS");
  }
  LOG_TRACE("tcp nat type is %s", StunClient::get_nat_type(type));
  if (type == StunClient::kModerate || type == StunClient::kOpen) {
    LOG_TRACE("tcp hole address %s:%d", hole.get_ip(), hole.get_port());
  }
}

} /* namespace stunclient */
