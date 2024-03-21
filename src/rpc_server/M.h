
#ifndef M_H
#define M_H

#include <optional>

#if __cplusplus < 202302L

namespace monad {

// join
template <bool same_m, typename M> constexpr auto join(M &&m) {
  if constexpr (same_m) {
    return std::forward<M>(m).value();
  } else {
    return std::forward<M>(m);
  }
}

// Functor :: m a -> (a - b) -> m b
template <typename Mb, typename Ma, typename F>
constexpr auto functor(Ma &&m, F &&f) {
  return Mb(std::forward<F>(f)(std::forward<Ma>(m).value()));
}

// >>= :: m a -> (a -> m b) -> m b
template <typename M, typename F> constexpr auto chain(M &&m, F &&f) {
  return std::forward<F>(f)(std::forward<M>(m).value());
}

// >> :: m a -> m b -> m b
template <typename M, typename Mb> constexpr auto chain_right(M &&m, Mb &&b) {
  return std::forward<Mb>(b);
}

// return :: a -> m a
template <typename M, typename A> constexpr auto inject(A &&a) {
  using result_t = std::remove_reference_t<std::remove_cv_t<M>>;
  return result_t(std::forward<A>(a));
}
}; // namespace monad

template <typename T, typename JT, typename M, typename Comp = void>
struct Monadic {
  using value_type = JT;

  constexpr explicit Monadic() : v() {}
  constexpr explicit Monadic(value_type &&va)
      : v(std::forward<value_type>(va)) {}
  constexpr explicit Monadic(std::nullopt_t) : v() {}

  const value_type &value() const & {
    if constexpr (!std::is_same_v<T, value_type>) {
      return v.value();
    } else {
      return v;
    }
  }
  value_type &value() & {
    if constexpr (!std::is_same_v<T, value_type>) {
      return v.value();
    } else {
      return v;
    }
  }
  const value_type &&value() const && {
    if constexpr (!std::is_same_v<T, value_type>) {
      return std::move(v).value();
    } else {
      return std::move(v);
    }
  }
  value_type &&value() && {
    if constexpr (!std::is_same_v<T, value_type>) {
      return std::move(v).value();
    } else {
      return std::move(v);
    }
  }

  bool has_value() const {
    if constexpr (std::is_void_v<Comp>) {
      return !!v;
    } else {
      return Comp{}(v);
    }
  }

  // :: (a -> b) -> m b
  template <typename F> constexpr auto transform(F &&f) & {
    using result_t = std::invoke_result_t<F, value_type &>;
    using m_t = std::invoke_result_t<M, result_t>;
    return monad::functor<m_t>(*this, std::forward<F>(f));
  }
  template <typename F> constexpr auto transform(F &&f) const & {
    using result_t = std::invoke_result_t<F, const value_type &>;
    using m_t = std::invoke_result_t<M, result_t>;
    return monad::functor<m_t>(*this, std::forward<F>(f));
  }
  template <typename F> constexpr auto transform(F &&f) && {
    using result_t = std::invoke_result_t<F, value_type>;
    using m_t = std::invoke_result_t<M, result_t>;
    return monad::functor<m_t>(*this, std::forward<F>(f));
  }
  template <typename F> constexpr auto transform(F &&f) const && {
    using result_t = std::invoke_result_t<F, const value_type>;
    using m_t = std::invoke_result_t<M, result_t>;
    return monad::functor<m_t>(*this, std::forward<F>(f));
  }

  // :: (a -> m b) -> m b
  template <typename F> constexpr auto and_then(F &&f) & {
    using result_t = std::invoke_result_t<F, value_type &>;
    return has_value() ? monad::chain(*this, std::forward<F>(f)) : result_t();
  }
  template <typename F> constexpr auto and_then(F &&f) const & {
    using result_t = std::invoke_result_t<F, const value_type &>;
    return has_value() ? monad::chain(*this, std::forward<F>(f)) : result_t();
  }
  template <typename F> constexpr auto and_then(F &&f) && {
    using result_t = std::invoke_result_t<F, value_type>;
    return has_value() ? monad::chain(std::move(*this), std::forward<F>(f))
                       : result_t();
  }
  template <typename F> constexpr auto and_then(F &&f) const && {
    using result_t = std::invoke_result_t<F, const value_type>;
    return has_value() ? monad::chain(std::move(*this), std::forward<F>(f))
                       : result_t();
  }

  // :: (a -> m a) -> m a
  template <typename F> constexpr auto or_else(F &&f) const & {
    if (!has_value()) {
      using result_t = std::invoke_result_t<F>;
      if constexpr (std::is_void_v<result_t>) {
        std::forward<F>(f)();
      } else {
        return std::forward<F>(f)();
      }
    }
    return *this;
  }
  template <typename F> constexpr auto or_else(F &&f) && {
    if (!has_value()) {
      using result_t = std::invoke_result_t<F>;
      if constexpr (std::is_void_v<result_t>) {
        std::forward<F>(f)();
      } else {
        return std::forward<F>(f)();
      }
    }
    return std::move(*this);
  }

private:
  T v;
};

// Mo<T, Comp>
struct MakeMo;
template <class T, class C = void> using Mo = Monadic<T, T, MakeMo, C>;
struct MakeMo {
  template <typename T> auto operator()(T &&t) {
    return Mo<T>(std::forward<T>(t));
  }
};

// option<T>
struct MakeOption;
template <class T> using option = Monadic<std::optional<T>, T, MakeOption>;
struct MakeOption {
  template <typename T> auto operator()(T &&t) {
    return option<T>(std::forward<T>(t));
  }
};

#else

template <class T> using option = std::optional<T>;

#endif // 202302L

#endif /* M_H */
