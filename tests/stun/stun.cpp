
#include "gtest/gtest.h"

#include "sockets/TcpListener.h"
#include "sockets/TcpStream.h"
#include "sockets/UdpSocket.h"
#include "stun/StunClient.h"
#include "utils/log.h"

/*
stun_server_tcp = [
    "fwa.lifesizecloud.com",
    "stun.isp.net.au",
    "stun.freeswitch.org",
    "stun.voip.blackberry.com",
    "stun.nextcloud.com",
    "stun.stunprotocol.org",
    "stun.sipnet.com",
    "stun.radiojar.com",
    "stun.sonetel.com",
    "stun.voipgate.com"
]
stun_server_udp = [
    "stun.miwifi.com",
    "stun.qq.com"
]
*/
constexpr char SERVER_NAME1[] = "stun.isp.net.au";
constexpr char SERVER_NAME2[] = "stun.freeswitch.org";

TEST(StunTest, StunResponse) {
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
             "%x ", (uint8_t)buff[i]);
  }
  LOG_TRACE("response buff[%d]: %s", n, out_buff);
  SocketAddr addr = {};
  StunClient client;
  int err;
  int res = client.response(buff, n, err, addr);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (res >= 0) {
    LOG_TRACE("self buff ip port %s:%d", addr.get_ip(), addr.get_port());
  }
}

TEST(StunTest, StunClientUdp) {
  StunClient client;
  char buff[1024] = {};
  int send_size = client.request(buff, sizeof(buff));
  EXPECT_TRUE(send_size > 0) << "stun get send buff error! " << send_size;

  std::error_code ec;
  LOG_TRACE("begin connect stun server");
  auto stun_addr = SocketAddr::resolve_host(SERVER_NAME1, "3478", ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("connect stun server %s:%d", stun_addr.get_ip(),
            stun_addr.get_port());

  char port_buff[] = "41078";
  auto stun_stream = UdpSocket::create(kIpV4, ec);
  // auto stun_stream = UdpSocket::bind(port_buff, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  stun_stream.set_read_timeout(5000, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("stun client write buff");
  stun_stream.send_to(buff, send_size, stun_addr, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("stun client read");
  std::pair<size_t, SocketAddr> re =
      stun_stream.recv_from(buff, sizeof(buff), ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  size_t n = re.first;
  // char out_buff[1024] = {};
  // for (size_t i = 0; i < n; ++i) {
  //   sprintf(out_buff + strlen(out_buff), "%x ", (uint8_t)buff[i]);
  // }
  // LOG_TRACE("response buff[%d]: %s", n, out_buff);
  SocketAddr local_public_addr = {};
  int err_no = 0;
  int res = client.response(buff, n, err_no, local_public_addr);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (err_no > 0) {
    LOG_TRACE("recv error code %d", err_no);
  }
  if (res >= 0) {
    LOG_TRACE("self buff ip port %s:%d", local_public_addr.get_ip(),
              local_public_addr.get_port());
  }

  // check 1
  {
    send_size = client.request(buff, sizeof(buff));
    EXPECT_TRUE(send_size > 0) << "stun get send buff error! " << send_size;

    auto stun_addr1 = SocketAddr::resolve_host(SERVER_NAME2, "3478", ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    LOG_TRACE("write stun2 server %s:%d", stun_addr1.get_ip(),
              stun_addr1.get_port());
    stun_stream.send_to(buff, send_size, stun_addr1, ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    LOG_TRACE("stun2 client read");
    re = stun_stream.recv_from(buff, sizeof(buff), ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    n = re.first;
    SocketAddr local_public_addr2 = {};
    res = client.response(buff, n, err_no, local_public_addr2);
    EXPECT_TRUE(res >= 0) << "stun2 get response error! " << res;
    if (err_no > 0) {
      LOG_TRACE("recv error code %d", err_no);
    }
    if (res >= 0) {
      LOG_TRACE("self 2 buff ip port %s:%d", local_public_addr2.get_ip(),
                local_public_addr2.get_port());
    }

    EXPECT_TRUE(local_public_addr.get_port() == local_public_addr2.get_port());
  }
  // check 2
  // {
  //   auto stun_check = UdpSocket::create(kIpV4, ec);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   stun_check.set_read_timeout(5000, ec);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   LOG_TRACE("send to public %s:%d ", local_public_addr.get_ip(),
  //             local_public_addr.get_port());
  //   memset(buff, 0, sizeof(buff));
  //   strncpy(buff, "send to public msg!", sizeof(buff));
  //   stun_check.send_to(buff, strlen(buff), local_public_addr, ec);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   memset(buff, 0, sizeof(buff));
  //   re = stun_stream.recv_from(buff, sizeof(buff), ec);
  //   LOG_TRACE("public recv from %s:%d : %s", re.second.get_ip(),
  //             re.second.get_port(), buff);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   memset(buff, 0, sizeof(buff));
  //   strncpy(buff, "send back msg!", sizeof(buff));
  //   stun_stream.send_to(buff, strlen(buff), re.second, ec);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   memset(buff, 0, sizeof(buff));
  //   re = stun_check.recv_from(buff, sizeof(buff), ec);
  //   LOG_TRACE("local recv from %s:%d : %s", re.second.get_ip(),
  //             re.second.get_port(), buff);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();

  //   stun_check.close(ec);
  //   EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  //   ec.clear();
  // }

  stun_stream.close(ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();
}

TEST(StunTest, StunClientTcp) {
  StunClient client;
  char buff[1024] = {};
  int send_size = client.request(buff, sizeof(buff));
  EXPECT_TRUE(send_size > 0) << "stun get send buff error! " << send_size;

  std::error_code ec;
  LOG_TRACE("begin connect stun server");
  auto stun_addr = SocketAddr::resolve_host(SERVER_NAME1, "3478", ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("connect stun server %s:%d", stun_addr.get_ip(),
            stun_addr.get_port());

  char port_buff[] = "41078";
  auto stun_stream = TcpStream::connect(stun_addr, ec);
  // auto stun_stream = TcpListener::bind_port(port_buff, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();
  stun_stream.connected(stun_addr, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("stun client write buff");
  stun_stream.write(buff, send_size, ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  LOG_TRACE("stun client read");
  size_t n = stun_stream.read(buff, sizeof(buff), ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();

  // char out_buff[1024] = {};
  // for (size_t i = 0; i < n; ++i) {
  //   sprintf(out_buff + strlen(out_buff), "%x ", (uint8_t)buff[i]);
  // }
  // LOG_TRACE("response buff[%d]: %s", n, out_buff);
  SocketAddr addr = {};
  int err_no = 0;
  int res = client.response(buff, n, err_no, addr);
  EXPECT_TRUE(res >= 0) << "stun get response error! " << res;
  if (err_no > 0) {
    LOG_TRACE("recv error code %d", err_no);
  }
  if (res >= 0) {
    LOG_TRACE("self buff ip port %s:%d", addr.get_ip(), addr.get_port());
  }

  // check 1
  {
    send_size = client.request(buff, sizeof(buff));
    EXPECT_TRUE(send_size > 0) << "stun get send buff error! " << send_size;

    auto stun_addr1 = SocketAddr::resolve_host(SERVER_NAME2, "3478", ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    LOG_TRACE("connect stun server 2 %s:%d", stun_addr1.get_ip(),
              stun_addr1.get_port());
    // auto stun_stream2 = TcpListener::bind_port(port_buff, ec);
    auto stun_stream2 = TcpStream::connect(stun_addr1, ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();
    stun_stream2.connected(stun_addr1, ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    LOG_TRACE("stun2 client write buff");
    stun_stream2.write(buff, send_size, ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    LOG_TRACE("stun2 client read");
    n = stun_stream2.read(buff, sizeof(buff), ec);
    EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
    ec.clear();

    SocketAddr local_public_addr2 = {};
    res = client.response(buff, n, err_no, local_public_addr2);
    EXPECT_TRUE(res >= 0) << "stun2 get response error! " << res;
    if (err_no > 0) {
      LOG_TRACE("recv error code %d", err_no);
    }
    if (res >= 0) {
      LOG_TRACE("self 2 buff ip port %s:%d", local_public_addr2.get_ip(),
                local_public_addr2.get_port());
    }

    EXPECT_TRUE(addr.get_port() == local_public_addr2.get_port());
  }

  stun_stream.close(ec);
  EXPECT_FALSE(ec) << ec.value() << " : " << ec.message();
  ec.clear();
}
