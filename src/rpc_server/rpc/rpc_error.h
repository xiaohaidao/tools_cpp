
#ifndef RPC_SERVER_RPC_RPC_ERROR_H
#define RPC_SERVER_RPC_RPC_ERROR_H

#include <system_error>

namespace rpc {

enum Error {
  // no 0

  // RpcCall CallResut Ret
  kTimeout,
  kError,

  // RpcCallResult
  kUnkownData,
  kUnkownFunction,
  kUnkownParameter,

};

} // namespace rpc

namespace std {
template <> struct is_error_code_enum<rpc::Error> : true_type {};
} // namespace std

std::error_code make_error_code(rpc::Error);

#endif // RPC_SERVER_RPC_RPC_ERROR_H
