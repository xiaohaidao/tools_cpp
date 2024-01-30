
#ifndef CODEC_MAKE_STRUCT_CODEC_H
#define CODEC_MAKE_STRUCT_CODEC_H

#include <vector>

namespace codec {

#include "detail/MakeDecode.h"
#include "detail/MakeEncode.h"
#include "detail/MakeLength.h"

// encode

struct MakeStructEncode {
  template <size_t index, typename T> struct MakeStructEncodeT {
    int operator()(Buff &buff, T const &v) {
      return make_encode<index, T>(buff, v);
    }
  }; /* struct MakeStructEncodeT */

  template <size_t index, typename T>
  struct MakeStructEncodeT<index, std::vector<T> const &> {
    int operator()(Buff &buff, std::vector<T> const &v) {
      uint32_t len = v.size();
      int ret = 0;
      if ((ret = make_encode<index, uint32_t>(buff, len)) < 0) {
        return ret;
      }
      for (auto const &i : v) {
        if ((ret = MakeStructEncodeT<index, T const &>{}(buff, i)) < 0) {
          return ret;
        }
      }
      return 0;
    }
  }; /* struct MakeStructEncodeT */

  template <size_t index, size_t N>
  struct MakeStructEncodeT<index, char const (&)[N]> {
    int operator()(Buff &buff, char const (&v)[N]) {
      Slice vs{v, N};
      return make_encode<index, Slice>(buff, vs);
    }
  }; /* struct MakeStructEncodeT */

  template <size_t index, typename T, size_t N>
  struct MakeStructEncodeT<index, T const (&)[N]> {
    int operator()(Buff &buff, T const (&v)[N]) {
      for (size_t i = 0; i < N; ++i) {
        int ret = 0;
        if ((ret = MakeStructEncodeT<index, T const &>{}(buff, v[i])) < 0) {
          return ret;
        }
      }
      return 0;
    }
  }; /* struct MakeStructEncodeT */

  template <size_t index = 1, typename T> int operator()(Buff &buff, T &&v) {
    return MakeStructEncodeT<index, T>{}(buff, std::forward<T>(v));
  }

  template <size_t index = 1, typename T, typename... Ts>
  int operator()(Buff &buff, T &&v, Ts &&... vs) {
    int ret = 0;
    if ((ret = operator()<index, T>(buff, std::forward<T>(v))) < 0) {
      return ret;
    }
    return operator()<index + 1, Ts...>(buff, std::forward<Ts>(vs)...);
  }
};

// decode

struct MakeStructDecode {
  template <size_t index, typename T> struct MakeStructDecodeT {
    int operator()(CBuff &buff, T &v) { return make_decode<index, T>(buff, v); }
  }; /* MakeStructDecodeT */

  template <size_t index, typename T>
  struct MakeStructDecodeT<index, std::vector<T> &> {
    int operator()(CBuff &buff, std::vector<T> &v) {
      uint32_t len;
      int ret = 0;
      if ((ret = make_decode<index, uint32_t>(buff, len)) < 0) {
        return ret;
      }
      for (size_t i = 0; i < len; ++i) {
        T c;
        if ((ret = MakeStructDecodeT<index, T &>{}(buff, c)) < 0) {
          return ret;
        }
        v.push_back(std::move(c));
      }
      return 0;
    }
  }; /* MakeStructDecodeT */

  template <size_t index, size_t N>
  struct MakeStructDecodeT<index, char (&)[N]> {
    int operator()(CBuff &buff, char (&v)[N]) {
      Slice vs = {};
      int ret = 0;
      if ((ret = make_decode<index, Slice>(buff, vs)) < 0) {
        return ret;
      }
      if (vs.size != N) {
        return (int)(-1 * index);
      }
      memcpy(v, vs.buff, vs.size);
      return 0;
    }
  }; /* struct MakeStructDecodeT */

  template <size_t index, typename T, size_t N>
  struct MakeStructDecodeT<index, T (&)[N]> {
    int operator()(CBuff &buff, T (&v)[N]) {
      for (size_t i = 0; i < N; ++i) {
        int ret = 0;
        if ((ret = MakeStructDecodeT<index, T &>{}(buff, v[i])) < 0) {
          return ret;
        };
      }
      return 0;
    }
  }; /* struct MakeStructDecodeT */

  template <size_t index = 1, typename T> int operator()(CBuff &buff, T &&v) {
    return MakeStructDecodeT<index, T>{}(buff, std::forward<T>(v));
  }

  template <size_t index = 1, typename T, typename... Ts>
  int operator()(CBuff &buff, T &&v, Ts &&... vs) {
    int ret = 0;
    if ((ret = operator()<index, T>(buff, std::forward<T>(v))) < 0) {
      return ret;
    }
    return operator()<index + 1, Ts...>(buff, std::forward<Ts>(vs)...);
  }
};

// make length

struct MakeStructLength {
  template <size_t index, typename T> struct MakeStructLengthT {
    uint32_t operator()(T const &v) { return make_length<index, T>(v); }
  }; /* MakeStructLengthT */

  template <size_t index, typename T>
  struct MakeStructLengthT<index, std::vector<T> const &> {
    uint32_t operator()(std::vector<T> const &v) {
      uint32_t size = make_length<index, uint32_t>(v.size());
      for (auto const &i : v) {
        size += MakeStructLengthT<index, T const &>{}(i);
      }
      return size;
    }
  }; /* MakeStructLengthT */

  template <size_t index, size_t N>
  struct MakeStructLengthT<index, char const (&)[N]> {
    uint32_t operator()(char const (&v)[N]) {
      Slice vs = {v, N};
      return make_length<index, Slice>(vs);
    }
  }; /* struct MakeStructLengthT */

  template <size_t index, typename T, size_t N>
  struct MakeStructLengthT<index, T const (&)[N]> {
    uint32_t operator()(T const (&v)[N]) {
      uint32_t size = 0;
      for (size_t i = 0; i < N; ++i) {
        size += MakeStructLengthT<index, T const &>{}(v[i]);
      }
      return size;
    }
  }; /* struct MakeStructLengthT */

  template <size_t index = 1, typename T> uint32_t operator()(T &&v) {
    return MakeStructLengthT<index, T>{}(std::forward<T>(v));
  }

  template <size_t index = 1, typename T, typename... Ts>
  uint32_t operator()(T &&v, Ts &&... vs) {
    return operator()<index, T>(std::forward<T>(v)) +
           operator()<index + 1, Ts...>(std::forward<Ts>(vs)...);
  }
};

} /* namespace codec */

// macro

#define STRUCT_DECLARE_DEFINE_FUNC(struct_type, s, ...)                        \
  inline int make_struct_encode(codec::Buff &buff, const struct_type &s) {     \
    return codec::MakeStructEncode{}(buff, __VA_ARGS__);                       \
  }                                                                            \
  inline int make_struct_decode(codec::CBuff &buff, struct_type &s) {          \
    return codec::MakeStructDecode{}(buff, __VA_ARGS__);                       \
  }                                                                            \
  inline uint32_t make_struct_length(struct_type const &s) {                   \
    return codec::MakeStructLength{}(__VA_ARGS__);                             \
  }                                                                            \
  template <size_t index, typename T>                                          \
  int make_encode(codec::Buff &buff, struct_type const &v) {                   \
    codec::Slice slice = {nullptr, make_struct_length(v)};                     \
                                                                               \
    int ret = 0;                                                               \
    if ((ret = codec::MakeEncode{}(slice, index, buff)) < 0) {                 \
      return ret;                                                              \
    }                                                                          \
    codec::Buff b = {buff.buff + buff.offset - slice.size, slice.size, 0};     \
    return make_struct_encode(b, v);                                           \
  }                                                                            \
  template <size_t index, typename T>                                          \
  int make_decode(codec::CBuff &buff, struct_type &v) {                        \
    codec::Slice slice = {};                                                   \
    int ret = 0;                                                               \
    if ((ret = codec::MakeDecode{}(slice, index, buff)) < 0) {                 \
      return ret;                                                              \
    }                                                                          \
    codec::CBuff b = {slice.buff, slice.size, 0};                              \
    return make_struct_decode(b, v);                                           \
  }                                                                            \
  template <size_t index, typename T>                                          \
  uint32_t make_length(struct_type const &v) {                                 \
    codec::Slice slice = {nullptr, make_struct_length(v)};                     \
    return codec::MakeLength{}(slice, index);                                  \
  }

#define struct_encode(buff, obj) make_struct_encode(buff, obj)
#define struct_decode(buff, obj) make_struct_decode(buff, obj)
#define struct_length(obj) make_struct_length(obj)

#endif /* CODEC_MAKE_STRUCT_CODEC_H */
