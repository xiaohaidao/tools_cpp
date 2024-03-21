
#include "gtest/gtest.h"

#include "rpc_server/M.h"

namespace test_moand {

#define CALL_MONAD_IMPL(t)                                                     \
  t<int> a(1);                                                                 \
  printf("begin value %d\n", a.value());                                       \
  auto a1 = a.and_then([](int x) {                                             \
               printf("and: %d + 1\n", x);                                     \
               EXPECT_TRUE(x == 1) << "x: " << x;                              \
               /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                \
               return t<int>(x + 1);                                           \
             })                                                                \
                .and_then([](int x) {                                          \
                  printf("and: %d + 1\n", x);                                  \
                  EXPECT_TRUE(x == 2) << "x: " << x;                           \
                  /* NOLINTNEXTLINE(bugprone-macro-parentheses) */             \
                  return t<int>(x + 1);                                        \
                })                                                             \
                .transform([](int x) {                                         \
                  printf("functor: %d + 1\n", x);                              \
                  EXPECT_TRUE(x == 3) << "x: " << x;                           \
                  return x + 1;                                                \
                })                                                             \
                .transform([](int x) {                                         \
                  printf("functor: %d + 1\n", x);                              \
                  EXPECT_TRUE(x == 4) << "x: " << x;                           \
                  return x + 1;                                                \
                })                                                             \
                .transform([](int x) {                                         \
                  printf("functor to double: %d + 1.1\n", x);                  \
                  EXPECT_TRUE(x == 5) << "x: " << x;                           \
                  return x + 1.1;                                              \
                })                                                             \
                .and_then([](double x) {                                       \
                  printf("and: %f + 1.1\n", x);                                \
                  EXPECT_DOUBLE_EQ(x, 6.1) << "x: " << x;                      \
                  /* NOLINTNEXTLINE(bugprone-macro-parentheses) */             \
                  return t<double>(x + 1.1);                                   \
                })                                                             \
                .or_else([]() { printf("else or \n"); })                       \
                .and_then([](double x) {                                       \
                  printf("and to null<int>: %f\n", x);                         \
                  EXPECT_DOUBLE_EQ(x, 7.2) << "x: " << x;                      \
                  /* NOLINTNEXTLINE(bugprone-macro-parentheses) */             \
                  return t<uint32_t>();                                        \
                })                                                             \
                .or_else([]() { printf("else or \n"); })                       \
                .or_else([]() {                                                \
                  printf("else or return int 1\n");                            \
                  /* NOLINTNEXTLINE(bugprone-macro-parentheses) */             \
                  return t<uint32_t>(1);                                       \
                })                                                             \
                .and_then([](uint32_t x) {                                     \
                  printf("and int return double %d + 1000.111\n", x);          \
                  EXPECT_TRUE(x == 1) << "x: " << x;                           \
                  /* NOLINTNEXTLINE(bugprone-macro-parentheses) */             \
                  return t<double>(x + 1000.111);                              \
                });                                                            \
  printf("end value %f\n", a1.value());

TEST(rpc_server, test_monand){CALL_MONAD_IMPL(Mo)}

TEST(rpc_server, test_monand_option) {
  CALL_MONAD_IMPL(option)

  std::optional<int> p1(1);
  EXPECT_TRUE(p1.value() == 1);

  std::optional<std::optional<int>> p2(1);
  if (p2.has_value()) {
    auto p3 = monad::join<true>(p2);
    if (p3.has_value()) {
      EXPECT_TRUE(p3.value() == 1);
    }
    EXPECT_TRUE(p2.value() == 1);
  }
}

template <typename T, typename E> using Expect = option<std::tuple<T, E>>;

TEST(rpc_server, test_monand_tuple) {
  Expect<int, std::string> a({1, ""});
  auto a1 = a.and_then([](const std::tuple<int, std::string> &x) {
    printf("x first : %d:%s\n", std::get<0>(x), std::get<1>(x).c_str());
    return Expect<int, std::string>({2, ""});
  });
  EXPECT_TRUE(std::get<0>(a1.value()) == 2);
  EXPECT_TRUE(std::get<1>(a1.value()).size() == 0);

  option<int> const aa(1);
  auto aa1 = aa.transform([](int x) { return x + 1.1; });
  static_assert(std::is_same_v<decltype(aa1), option<double>>);
  aa1.or_else([]() {});
}

struct CompPair {
  template <typename T, typename E>
  auto operator()(const std::pair<T, E> &p) -> bool {
    return !!p.first;
  }
};
template <typename T, typename E>
using Expected = Mo<std::pair<T, E>, CompPair>;
TEST(rpc_server, test_monand_pair) {
  Expected<int, std::string> const a({1, "str 1"});
  a.and_then([](const auto &x) {
     printf("x first : %d:%s\n", x.first, x.second.c_str());
     EXPECT_TRUE(x.first == 1) << "x.first: " << x.first;
     EXPECT_TRUE(x.second == "str 1") << "x.second: " << x.second;
     return Expected<int, std::string>({2, "str 2"});
   })
      .and_then([](const auto &x) {
        printf("x second : %d:%s\n", x.first, x.second.c_str());
        EXPECT_TRUE(x.first == 2) << "x.first: " << x.first;
        EXPECT_TRUE(x.second == "str 2") << "x.second: " << x.second;
        return Expected<int, std::string>({0, "error"});
      })
      .and_then([](const auto &x) {
        printf("and x third : %d:%s\n", x.first, x.second.c_str());
        EXPECT_TRUE(false) << "and logic error";
        EXPECT_TRUE(x.first != 0) << "x.first: " << x.first;
        EXPECT_TRUE(x.second != "error") << "x.second: " << x.second;
        return Expected<int, std::string>({0, ""});
      })
      .or_else([]() {
        printf("or third\n");
        EXPECT_TRUE(true) << "or_else logic error";
        return Expected<int, std::string>({1, "or error"});
      })
      .and_then([](const auto &x) {
        printf("and x third : %d:%s\n", x.first, x.second.c_str());
        EXPECT_TRUE(x.first == 1) << "x.first: " << x.first;
        EXPECT_TRUE(x.second == "or error") << "x.second: " << x.second;
        return Expected<int, std::string>({0, ""});
      });
}

} // namespace test_moand
