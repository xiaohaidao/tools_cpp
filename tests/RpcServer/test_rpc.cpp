
#include "gtest/gtest.h"

#include "rpc_server/rpc/RpcBind.h"
#include "rpc_server/rpc/RpcCall.h"

namespace test_codec_rpc {

int add(int a, int b) { return a + b; }
int add1(const int &a, const int &b) { return a + b; }

TEST(rpc_server, test_rpc) {
  // using namespace rpc;
  RpcCall call;
  RpcBind ss;
  call.bind_send([&](const char *in_buff, size_t in_size, char *out_buff,
                     size_t out_size) -> int {
    // codec::CBuff s = {in_buff, in_size, 0};
    // codec::Buff re = {out_buff, out_size, 0};
    ss.call(in_buff, in_size, out_buff, out_size);
    return 0;
  });
  ss.bind("add", add);
  ss.bind("add1", add1);

  int r = 0;
  call(r, "add", 1, 2);
  EXPECT_TRUE(r == 3) << "r value:" << r;
  r = 1;
  call(r, "add1", 1, 2);
  EXPECT_TRUE(r == 3) << "r value:" << r;
}

} // namespace test_codec_rpc
