
#ifndef RPC_SERVER_RPC_RPCBIND_H
#define RPC_SERVER_RPC_RPCBIND_H

#include <unordered_map>

#include "FuncTypeHelp.h"
#include "RpcCallName.h"
#include "RpcCallResult.h"
#include "codec/codec.h"

class RpcBind {
public:
  /**
   * @brief
   *
   * @tparam F
   * @param name
   * @param f
   * @return constexpr auto
   */
  template <typename F> constexpr auto bind(const char *name, F &&f) {
    using Func = FuncTypeHelp<std::decay_t<decltype(f)>>;
    RpcCallName index = {codec::Slice{name, (uint32_t)strlen(name)},
                         std::tuple_size<typename Func::args_t>::value};
    if (maps_.find(index) != maps_.end()) {
      // TODO:(ERROR)
      return -1;
    }

    std::function<int(codec::CBuff &, codec::Buff &)> bind_func =
        [f = std::function<std::remove_pointer_t<std::decay_t<decltype(f)>>>(f),
         this](codec::CBuff &in, codec::Buff &out) {
          using Func = FuncTypeHelp<std::decay_t<decltype(f)>>;
          typename Func::decay_args_t params = {};
          int ret =
              get_param(in, params,
                        std::make_index_sequence<
                            std::tuple_size<typename Func::args_t>::value>());
          if (ret < 0) {
            make_encode<0, size_t>(out, (size_t)kUnkownParameter);
            return ret;
          }
          make_encode<0, size_t>(out, (size_t)kOk);
          auto re =
              call_func(f, params,
                        std::make_index_sequence<
                            std::tuple_size<typename Func::args_t>::value>());
          return make_encode<0, decltype(re)>(out, re);
        };

    maps_[std::move(index)] = std::move(bind_func);
    return 0;
  }

  /**
   * @brief
   *
   * @param in_buff
   * @param in_size
   * @param out_buff
   * @param out_size
   */
  void call(const char *in_buff, size_t in_size, char *out_buff,
            size_t out_size) const {
    codec::CBuff in = {in_buff, (uint32_t)in_size, 0};
    codec::Buff out = {out_buff, (uint32_t)out_size, 0};
    RpcCallName call_name = {};
    int re = struct_decode(in, call_name);
    if (re < 0) {
      make_encode<0, size_t>(out, (size_t)kUnkownData);
      return;
    }
    auto f = maps_.find(call_name);
    if (f == maps_.end()) {
      make_encode<0, size_t>(out, (size_t)kUnkownFunction);
      return;
    }
    if (f->second(in, out) < 0) {
      return;
    }
  }

private:
  std::unordered_map<RpcCallName,
                     std::function<int(codec::CBuff &in, codec::Buff &out)>,
                     RpcCallNameHash>
      maps_;

  template <typename F, typename Tuple, size_t... I>
  constexpr auto call_func(F &&f, Tuple &&params, std::index_sequence<I...>) {
    return f(std::get<I>(params)...);
  }

  template <typename Tuple, size_t... I>
  constexpr auto get_param(codec::CBuff &in, Tuple &&param,
                           std::index_sequence<I...>) {
    int ret = 0;
    ((ret = ret < 0 ? ret
                    : make_decode<0, typename std::tuple_element<
                                         I, std::decay_t<Tuple>>::type>(
                          in, std::get<I>(std::forward<Tuple>(param)))),
     ...);
    return ret;
  }
};

#endif /* RPC_SERVER_RPC_RPCBIND_H */
