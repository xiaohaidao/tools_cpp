
#ifndef RPC_SERVER_RPC_RPCCALLNAME_H
#define RPC_SERVER_RPC_RPCCALLNAME_H

#include <string_view>

#include "codec/codec.h"

struct RpcCallName {
  codec::Slice name; /// function name
  size_t params;     /// parameter size

  /**
   * @brief order by container
   *
   * @param s
   * @return true equal, otherwise not equal
   */
  bool operator==(const RpcCallName &s) const noexcept {
    if (params != s.params) {
      return false;
    }
    if (name.size != s.name.size) {
      return false;
    }
    return memcmp(name.buff, s.name.buff, name.size) == 0;
  }

  bool operator<(const RpcCallName &s) const noexcept {
    if (memcmp(name.buff, s.name.buff, (std::min)(name.size, s.name.size)) <
        0) {
      return true;
    }
    return params < s.params;
  }
};

STRUCT_DECLARE_DEFINE_FUNC(RpcCallName, s, s.name, s.params);

struct RpcCallNameHash {
  size_t operator()(const RpcCallName &s) const noexcept {
    return std::hash<std::string_view>{}(
               std::string_view(s.name.buff, s.name.size)) ^
           s.params;
  }
};

// template <> struct std::hash<RpcCallName> {
// };

#endif /* RPC_SERVER_RPC_RPCCALLNAME_H */
