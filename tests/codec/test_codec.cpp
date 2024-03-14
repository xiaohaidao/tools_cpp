
#include "gtest/gtest.h"

#include "codec/codec.h"

namespace test_codecs {

auto print_byte = [](const char *buff, size_t size) {
  printf("size %zu\n", size);
  for (size_t i = 0; i < size; ++i) {
    printf("0x%00X ", (unsigned char)buff[i]);
  }
  printf("\n");
};

TEST(codec, test_var_int) {
  using namespace codec;

  VarInt v = 150u;
  EXPECT_TRUE(v.size == 2);
  EXPECT_TRUE(v.buff[0] == 0x96);
  EXPECT_TRUE(v.buff[1] == 0x01);
  EXPECT_TRUE((uint32_t)v == 150);

  VarInt dev(0);
  dev.byte_from(v.buff);
  EXPECT_TRUE(dev.size == 2);
  EXPECT_TRUE(dev.buff[0] == 0x96);
  EXPECT_TRUE(dev.buff[1] == 0x01);
  EXPECT_TRUE((uint32_t)dev == 150);

  v = 150;
  EXPECT_TRUE(v.size == 2);
  EXPECT_TRUE(v.buff[0] == 0xAC);
  EXPECT_TRUE(v.buff[1] == 0x02);
  EXPECT_TRUE((int32_t)v == 150);

  dev.byte_from(v.buff);
  EXPECT_TRUE(dev.size == 2);
  EXPECT_TRUE(dev.buff[0] == 0xAC);
  EXPECT_TRUE(dev.buff[1] == 0x02);
  EXPECT_TRUE((int32_t)dev == 150);
}

TEST(codec, test_var_fixed) {
  using namespace codec;

  float const ff32 = (float)3.525;
  double const ff64 = 1.524;
  I32 f32(ff32);
  EXPECT_TRUE(ff32 == (float)f32) << (float)f32;
  EXPECT_TRUE(f32 == ff32) << (float)f32;
  EXPECT_TRUE(ff32 == f32) << (float)f32;
  EXPECT_FALSE(1.0 == f32);

  I64 f64(ff64);
  EXPECT_TRUE(ff64 == (double)f64) << (double)f64;
  EXPECT_TRUE(f64 == ff64) << (double)f64;
  EXPECT_TRUE(ff64 == f64) << (double)f64;
  EXPECT_FALSE(f64 == 1.0);
}

TEST(codec, test_make_len) {
  using namespace codec;

  EXPECT_TRUE((make_length<0, float>((float)3.525) == 5));
  EXPECT_TRUE((make_length<16, float>((float)3.525) == 6));
  EXPECT_TRUE((make_length<0, double>(3.525) == 9));
  EXPECT_TRUE((make_length<16, double>(3.525) == 10));

  EXPECT_TRUE((make_length<0, uint16_t>(10) == 2));
  EXPECT_TRUE((make_length<0, uint16_t>(11) == 2));
  EXPECT_TRUE((make_length<0, uint32_t>(10) == 2));
  EXPECT_TRUE((make_length<0, uint32_t>(11) == 2));
  EXPECT_TRUE((make_length<0, uint64_t>(10) == 2));
  EXPECT_TRUE((make_length<0, uint64_t>(11) == 2));
  EXPECT_TRUE((make_length<16, uint16_t>(10) == 3));
  EXPECT_TRUE((make_length<16, uint16_t>(11) == 3));
  EXPECT_TRUE((make_length<16, uint32_t>(10) == 3));
  EXPECT_TRUE((make_length<16, uint32_t>(11) == 3));
  EXPECT_TRUE((make_length<16, uint64_t>(10) == 3));
  EXPECT_TRUE((make_length<16, uint64_t>(11) == 3));

  EXPECT_TRUE((make_length<0, Slice>({nullptr, 10}) == 12));
  EXPECT_TRUE((make_length<16, Slice>({nullptr, 10}) == 13));
}

struct Temp {
  int i1;
  uint32_t i2;
  float i3;
  double i4;
  // std::string i5;
};

STRUCT_DECLARE_DEFINE_FUNC(Temp, s, s.i1, s.i2, s.i3, s.i4)

struct Temp1 {
  int i1;
  Temp i2;
};

STRUCT_DECLARE_DEFINE_FUNC(Temp1, s, s.i1, s.i2)

struct Temp2 {
  int i1;
};
STRUCT_DECLARE_DEFINE_FUNC(Temp2, s, s.i1)

TEST(codec, test_struct) {
  using namespace codec;
  // init
  Temp t1;
  char b[1024];
  Buff buff{b, sizeof(b), 0};
  t1.i1 = 0;
  t1.i2 = 1;
  t1.i3 = 2.0;
  t1.i4 = 3.0;

  {
    int const v = 1000;
    buff.offset = 0;
    // write data
    int ret = make_encode<1, int>(buff, v);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    int rv = 0;
    CBuff cbuff{buff.buff, buff.offset, 0};
    ret = make_decode<1, int>(cbuff, rv);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    EXPECT_TRUE(rv == v);
  }
  {
    // write data
    buff.offset = 0;
    int ret = struct_encode(buff, t1);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    // get data
    Temp t2 = {};
    CBuff cbuff{buff.buff, buff.offset, 0};
    ret = struct_decode(cbuff, t2);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }

    EXPECT_TRUE(t1.i1 == t2.i1) << t1.i1 << t2.i1;
    EXPECT_TRUE(t1.i2 == t2.i2) << t1.i2 << t2.i2;
    EXPECT_TRUE(t1.i3 == t2.i3) << t1.i3 << t2.i3;
    EXPECT_TRUE(t1.i4 == t2.i4) << t1.i4 << t2.i4;

    EXPECT_TRUE(make_struct_length(t1) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp>(t1)) == buff.offset + 2);
  }

  // multiple struct
  Temp1 t3 = {};
  t3.i1 = -1;
  t3.i2 = t1;
  {
    EXPECT_TRUE(make_struct_length(t3.i2) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp>(t3.i2)) == buff.offset + 2);

    // write data
    buff.offset = 0;
    int ret = struct_encode(buff, t3);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    // get data
    Temp1 t2 = {};
    CBuff cbuff{buff.buff, buff.offset, 0};
    ret = struct_decode(cbuff, t2);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }

    EXPECT_TRUE(t3.i1 == t2.i1) << t3.i1 << t2.i1;
    EXPECT_TRUE(t3.i2.i1 == t2.i2.i1) << t3.i2.i1 << t2.i2.i1;
    EXPECT_TRUE(t3.i2.i2 == t2.i2.i2) << t3.i2.i2 << t2.i2.i2;
    EXPECT_TRUE(t3.i2.i3 == t2.i2.i3) << t3.i2.i3 << t2.i2.i3;
    EXPECT_TRUE(t3.i2.i4 == t2.i2.i4) << t3.i2.i4 << t2.i2.i4;

    EXPECT_TRUE(make_struct_length(t3) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp1>(t3)) == buff.offset + 2);
  }
  // single struct
  Temp2 t5 = {};
  t5.i1 = 1;
  {
    // write data
    buff.offset = 0;
    int ret = struct_encode(buff, t5);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    // get data
    Temp2 t2 = {};
    CBuff cbuff{buff.buff, buff.offset, 0};
    ret = struct_decode(cbuff, t2);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }

    EXPECT_TRUE(t5.i1 == t2.i1) << t5.i1 << t2.i1;

    EXPECT_TRUE(make_struct_length(t5) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp2>(t5)) == buff.offset + 2);
  }
}

TEST(codec, test_struct_namespace) {
  EXPECT_TRUE((codec::make_length<0, float>((float)3.525) == 5));
  EXPECT_TRUE((codec::make_length<16, float>((float)3.525) == 6));
  EXPECT_TRUE((codec::make_length<0, double>(3.525) == 9));
  EXPECT_TRUE((codec::make_length<16, double>(3.525) == 10));

  Temp t1;
  char b[1024];
  codec::Buff buff{b, sizeof(b), 0};
  t1.i1 = 0;
  t1.i2 = 1;
  t1.i3 = 2.0;
  t1.i4 = 3.0;

  {
    // write data
    int ret = struct_encode(buff, t1);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    // get data
    Temp t2 = {};
    codec::CBuff cbuff{buff.buff, buff.offset, 0};
    ret = struct_decode(cbuff, t2);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }

    EXPECT_TRUE(t1.i1 == t2.i1) << t1.i1 << t2.i1;
    EXPECT_TRUE(t1.i2 == t2.i2) << t1.i2 << t2.i2;
    EXPECT_TRUE(t1.i3 == t2.i3) << t1.i3 << t2.i3;
    EXPECT_TRUE(t1.i4 == t2.i4) << t1.i4 << t2.i4;

    EXPECT_TRUE(make_struct_length(t1) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp>(t1)) == buff.offset + 2);
  }
}

struct TempVec {
  int i1;
  uint32_t i2;
  float i3;
  double i4;
  std::vector<uint32_t> i5;
  std::vector<std::vector<uint32_t>> i6;
  char i7[12];
  int i8[5];
};
STRUCT_DECLARE_DEFINE_FUNC(TempVec, s, s.i1, s.i2, s.i3, s.i4, s.i5, s.i6, s.i7,
                           s.i8)

TEST(codec, test_struct_vec) {
  using namespace codec;
  // init
  TempVec t1;
  char b[1024];
  Buff buff{b, sizeof(b), 0};
  t1.i1 = 0;
  t1.i2 = 1;
  t1.i3 = 2.0;
  t1.i4 = 3.0;
  t1.i5 = {1, 2, 3, 4, 5};
  t1.i6 = {{1, 2, 3, 4, 5}, {6, 5, 4, 3, 2, 1}};
  snprintf(t1.i7, sizeof(t1.i7), "testvalue");
  t1.i8[0] = 1;
  t1.i8[1] = 2;
  t1.i8[2] = 3;
  t1.i8[3] = 4;
  t1.i8[4] = 5;
  {
    // write data
    int ret = struct_encode(buff, t1);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }
    // get data
    TempVec t2 = {};
    CBuff cbuff{buff.buff, buff.offset, 0};
    ret = struct_decode(cbuff, t2);
    if (ret < 0) {
      EXPECT_FALSE(ret < 0) << "ret value " << ret;
    }

    EXPECT_TRUE(t1.i1 == t2.i1) << t1.i1 << t2.i1;
    EXPECT_TRUE(t1.i2 == t2.i2) << t1.i2 << t2.i2;
    EXPECT_TRUE(t1.i3 == t2.i3) << t1.i3 << t2.i3;
    EXPECT_TRUE(t1.i4 == t2.i4) << t1.i4 << t2.i4;
    EXPECT_TRUE(t1.i5.size() == t2.i5.size())
        << t1.i5.size() << ":" << t2.i5.size();
    if (t1.i5.size() == t2.i5.size()) {
      for (size_t i = 0; i < t1.i5.size(); ++i) {
        EXPECT_TRUE(t1.i5[i] == t2.i5[i]) << i << t1.i5[i] << t2.i5[i];
      }
    }

    EXPECT_TRUE(t1.i6.size() == t2.i6.size())
        << t1.i6.size() << ":" << t2.i6.size();
    if (t1.i6.size() == t2.i6.size()) {
      for (size_t i = 0; i < t1.i6.size(); ++i) {
        EXPECT_TRUE(t1.i6[i].size() == t2.i6[i].size())
            << t1.i6[i].size() << ":" << t2.i6[i].size();
        if (t1.i6[i].size() == t2.i6[i].size()) {
          for (size_t j = 0; j < t1.i6[i].size(); ++j) {
            EXPECT_TRUE(t1.i6[i][j] == t2.i6[i][j])
                << "ij:" << i << i << ":" << t1.i6[i][j] << ":" << t2.i6[i][j];
          }
        }
      }
    }
    EXPECT_TRUE(memcmp(t1.i7, t2.i7, sizeof(t1.i7)) == 0)
        << t1.i7 << ":" << t2.i7;

    for (size_t i = 0; i < sizeof(t1.i8) / sizeof(*t1.i8); ++i) {
      EXPECT_TRUE(t1.i8[i] == t2.i8[i]) << (t1.i8[i]) << ":" << (t2.i8[i]);
    }

    EXPECT_TRUE(make_struct_length(t1) == buff.offset);
    EXPECT_TRUE((make_length<1, Temp>(t1)) == buff.offset + 2);
  }
}

namespace test_codec_rpc {

#include "rpc_server/rpc/RpcBind.h"
#include "rpc_server/rpc/RpcCall.h"

int add(int a, int b) { return a + b; }
int add1(const int &a, const int &b) { return a + b; }

TEST(codec, test_rpc) {
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

} /* namespace test_codecs */
