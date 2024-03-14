
#ifndef RPC_SERVER_RPC_RUNCTYPEHELP_H
#define RPC_SERVER_RPC_RUNCTYPEHELP_H

#include <functional>

template <class F> struct FuncTypeHelp;

template <class R, class... A> struct FuncTypeHelp<R(A...)> {
  using result_t = R;
  using args_t = std::tuple<A...>;
  using decay_args_t = std::tuple<std::decay_t<A>...>;
};

template <class R, class... A>
struct FuncTypeHelp<R (*)(A...)> : public FuncTypeHelp<R(A...)> {};

template <class R, class... A>
struct FuncTypeHelp<R (&)(A...)> : public FuncTypeHelp<R(A...)> {};

template <class R, class... A>
struct FuncTypeHelp<std::function<R(A...)>> : public FuncTypeHelp<R(A...)> {};

#endif /* RPC_SERVER_RPC_RUNCTYPEHELP_H */
