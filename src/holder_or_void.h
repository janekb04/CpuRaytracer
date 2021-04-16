#ifndef HOLDER_OR_VOID_H
#define HOLDER_OR_VOID_H
#include <type_traits>
#include <functional>

template <typename T>
struct holder_or_void
{
private:
    T data;
public:
    template <typename Invocable, typename... Args>
    constexpr holder_or_void(Invocable&& i, Args&&... args) noexcept(noexcept(T{ std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...) })) :
        data{ std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...) }
    {
    }
    [[nodiscard]] constexpr decltype(auto) operator()() noexcept {
        return data;
    };
    [[nodiscard]] constexpr decltype(auto) operator()() const noexcept {
        return data;
    };
    template <typename Invocable, typename... Args>
    constexpr decltype(auto) invoke(Invocable&& i, Args&&... args) noexcept(noexcept(std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)..., data)))
    {
        return std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)..., data);
    }
    template <typename Invocable, typename... Args>
    constexpr decltype(auto) invoke(Invocable&& i, Args&&... args) const noexcept(noexcept(std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)..., data)))
    {
        return std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)..., data);
    }
};

template<>
struct holder_or_void<void>
{
    template <typename Invocable, typename... Args>
    constexpr holder_or_void(Invocable&& i, Args&&... args) noexcept(noexcept(std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...)))
    {
        std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...);
    }
    constexpr void operator()() const noexcept {}
    template <typename Invocable, typename... Args>
    constexpr decltype(auto) invoke(Invocable&& i, Args&&... args) const noexcept(noexcept(std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...)))
    {
        return std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...);
    }
};

template <typename Invocable, typename... Args>
holder_or_void(Invocable&& i, Args&&... args) -> holder_or_void<decltype(std::invoke(std::forward<Invocable>(i), std::forward<Args>(args)...))>;

#endif // HOLDER_OR_VOID_H
