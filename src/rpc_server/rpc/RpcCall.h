
#ifndef RPC_SERVER_RPC_RPCCALL_H
#define RPC_SERVER_RPC_RPCCALL_H

#include <functional>

#include "RpcCallName.h"
#include "RpcCallResult.h"
#include "codec/codec.h"

struct RpcCall {

  struct CallResut {
    enum Ret {
      kOk,
      kTimeout,
      kError,
    } ret;
    RpcCallResult call_result;
  };

  /**
   * @brief 
   * 
   * @tparam Args 
   * @param name 
   * @param args 
   * @return constexpr int 
   */
  template <typename... Args>
  constexpr int operator()(const char *name, Args &&...args) {
    char data[4096];
    codec::Buff buff = {data, sizeof(data), 0};
    codec::CBuff cbuff = {data, sizeof(data), 0};
    RpcCallName call_name = {};
    call_name.name = {name, (uint32_t)strlen(name)};
    call_name.params = sizeof...(Args);
    int r = struct_encode(buff, call_name);
    if (r < 0) {
      return r;
    }
    return encode_call(cbuff, buff, std::forward<Args>(args)...);
  }

  /**
   * @brief 
   * 
   * @tparam R 
   * @tparam Args 
   * @param re 
   * @param name 
   * @param args 
   * @return constexpr int 
   */
  template <typename R, typename... Args>
  constexpr int operator()(R &re, const char *name, Args &&...args) {
    char data[4096];
    codec::Buff buff = {data, sizeof(data), 0};
    codec::CBuff cbuff = {data, sizeof(data), 0};
    RpcCallName call_name = {};
    call_name.name = {name, (uint32_t)strlen(name)};
    call_name.params = sizeof...(Args);
    int r = struct_encode(buff, call_name);
    if (r < 0) {
      return r;
    }
    r = encode_call(cbuff, buff, std::forward<Args>(args)...);
    if (r < 0) {
      return r;
    }
    return make_decode<0, R>(cbuff, re);
  }

  /**
   * @brief 
   * 
   * @param f 
   */
  void
  bind_send(const std::function<int(const char *, size_t, char *, size_t)> &f) {
    send_ = f;
  }

private:
  int encode_call(codec::CBuff &cbuff, codec::Buff &buff) {
    if (!send_) {
      return -1;
    }
    int re = send_((const char *)buff.buff, buff.offset, (char *)cbuff.buff,
                   cbuff.size);
    if (re < 0) {
      // TODO: error
      printf("err\n");
      return re;
    }
    size_t r_call = {}; // RpcCallResult
    re = make_decode<0, size_t>(cbuff, r_call);
    if (re < 0) {
      return re;
    }
    switch ((RpcCallResult)r_call) {
    case kUnkownData:
      printf("call error kUnkownData %d\n", r_call);
    case kUnkownFunction:
      printf("call error kUnkownFunction %d\n", r_call);
    case kUnkownParameter:
      printf("call error kUnkownParameter %d\n", r_call);
      return -1;
    default:
      break;
    }
    return re;
  }

  template <typename T, typename... Args>
  constexpr int encode_call(codec::CBuff &cbuff, codec::Buff &buff, T &&t,
                  Args &&...args) {
    int const r = make_encode<0, T>(buff, std::forward<T>(t));
    if (r < 0) {
      return r;
    }
    return encode_call(cbuff, buff, std::forward<Args>(args)...);
  }

  std::function<int(const char *, size_t, char *, size_t)> send_;
};

#endif /* RPC_SERVER_RPC_RPCCALL_H */
