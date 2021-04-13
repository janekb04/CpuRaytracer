//
// Created by Jan on 12-Apr-21.
//

#ifndef ENGINE_ARRAY_WRAPPER_H
#define ENGINE_ARRAY_WRAPPER_H

#include <utility>

namespace detail {
    template<std::size_t, typename T> using alwaysT = T;
    template<typename T, typename Seq>
    struct array_wrapper_impl;
    template<typename T, std::size_t N>
    using array_wrapper = array_wrapper_impl<T, std::make_index_sequence<N>>;

    template<typename T, size_t First, std::size_t... Is>
    class array_wrapper_impl<T, std::index_sequence<First, Is...>> {
    public:
        constexpr static size_t rank = sizeof...(Is) + 1;
        T *const data;
        const size_t product;
        const size_t extents[rank];

        explicit array_wrapper_impl(T *data, size_t first, alwaysT<Is, size_t>... _extents) :
                data{data},
                product{(rank == 1 ? -1 : 0) + (_extents * ... * 1)},
                extents{first, _extents...} {
        }

        decltype(auto) operator[](size_t idx) {
            if constexpr(rank == 1) {
                return data[idx];
            } else {
                return array_wrapper<T, rank - 1>{data + product * idx, extents[Is]...};
            }
        }
    };
}

template<typename T, std::size_t N>
using array_wrapper = detail::array_wrapper<T, N>;

#endif //ENGINE_ARRAY_WRAPPER_H
