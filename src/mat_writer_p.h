#pragma once

#include "mat_writer.h"
#include <array>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

namespace mat
{
    template <typename T> struct type_tag;

#define T(type, tag)                                                                                                   \
    template <> struct type_tag<type>                                                                                  \
    {                                                                                                                  \
        using Type = type;                                                                                             \
        static constexpr data_type Tag = data_type::tag;                                                               \
    }

    T(char, int8);
    T(int8_t, int8);
    T(uint8_t, uint8);
    T(int16_t, int16);
    T(uint16_t, uint16);
    T(int32_t, int32);
    T(uint32_t, uint32);
    T(float, float_single);
    T(double, float_double);

#undef T

    template <typename T> struct element : public data_element
    {
        const T *start;
        size_t count;

        element(const T *start, size_t count) : start(start), count(count) {}

        data_type type() const override { return type_tag<T>::Tag; }

        constexpr uint32_t byte_size() const override { return count * sizeof(T); }

        void write(std::ostream &os) const override
        {
            for (size_t i = 0; i < count; i++)
            {
                const char *p = (char *)(&start[i]);
                os.write(p, sizeof(T));
            }

            char zeros[8] = {0};
            os.write(zeros, (8 - byte_size()) % 8);
        }
    };

    template <typename T, typename T2> inline element<T> make_element(const T2 &);

    template <typename T, size_t N> constexpr element<T> make_element(const std::array<T, N> &arr)
    {
        return element<T>{(T *)arr.begin(), arr.size()};
    }

    template <> inline element<char> make_element<char, std::string>(const std::string &str)
    {
        return element<char>{str.data(), str.size()};
    }

    template <typename T> inline element<T> make_element(const std::vector<std::pair<T, T>> &data)
    {
        return element<T>{(T *)&*data.begin(), data.size() * 2};
    }
} // namespace mat
