
#ifndef CODEC_PARTIAL_H
#define CODEC_PARTIAL_H

#include <functional>
#include <tuple>

#if __cplusplus < 201703L // C++17 {

namespace detail {

// less than 201103L C++11 not support

#if __cplusplus < 201402L // C++14 {
namespace std14 {
template <typename T, T... Ints> struct integer_sequence {
  typedef T value_type;
  static constexpr std::size_t size() { return sizeof...(Ints); }
};

template <std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template <typename T, std::size_t N, T... Is>
struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...> {};

template <typename T, T... Is>
struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...> {};

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
} // namespace std14

template <std::size_t N>
using make_index_sequence = std14::make_index_sequence<N>;
template <std::size_t... Ints>
using index_sequence = std14::index_sequence<Ints...>;
template <typename... T>
using index_sequence_for = std14::index_sequence_for<T...>;

#else // C++14 } {

template <std::size_t N>
using make_index_sequence = std::make_index_sequence<N>;
template <std::size_t... Ints>
using index_sequence = std::index_sequence<Ints...>;
template <typename... T>
using index_sequence_for = std::index_sequence_for<T...>;

#endif /* __cplusplus  < 201402L // C++14 } */

template <typename F, typename... Args> struct PartialBinder {
  F func;
  std::tuple<Args...> args;

  template <std::size_t... Indices, typename... RestArgs>
  auto call_helper(index_sequence<Indices...>, RestArgs &&...rest) const
      -> decltype(func(std::get<Indices>(args)...,
                       std::forward<RestArgs>(rest)...)) {
    return func(std::get<Indices>(args)..., std::forward<RestArgs>(rest)...);
  }

  template <typename... RestArgs>
  auto operator()(RestArgs &&...rest) const
      -> decltype(call_helper(index_sequence_for<Args...>(),
                              std::forward<RestArgs>(rest)...)) {
    return call_helper(index_sequence_for<Args...>(),
                       std::forward<RestArgs>(rest)...);
  }
};

} /* namespace detail */

template <typename F, typename... Args>
auto partial(F &&f, Args &&...args)
    -> decltype(detail::PartialBinder<F, Args...>{
        std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...)}) {
  return detail::PartialBinder<F, Args...>{
      std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...)};
}

#else //  C++17 } {

template <typename F, typename... Args> auto partial(F &&f, Args &&...args) {
  return [f = std::forward<F>(f),
          args = std::make_tuple(std::forward<Args>(args)...)](auto &&...rest) {
    return std::apply(
        f, std::tuple_cat(
               args, std::make_tuple(std::forward<decltype(rest)>(rest)...)));
  };
}

#endif //  C++17 }

#endif /* CODEC_PARTIAL_H */
