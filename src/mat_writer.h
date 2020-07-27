#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <tuple>
#include <vector>

namespace mat
{
    struct header
    {
    };
    enum class data_type
    {
        int8 = 1,
        uint8 = 2,
        int16 = 3,
        uint16 = 4,
        int32 = 5,
        uint32 = 6,
        float_single = 7,
        float_double = 9,
        int64 = 12,
        uint64 = 13,
        matrix = 14,
        compressed = 15,
        utf8 = 16,
        utf16 = 17,
        utf32 = 18,
    };

    struct data_element
    {
        friend std::ostream &operator<<(std::ostream &str, const data_element &d);

        virtual ~data_element() {}

        uint32_t aligned_size() const { return ((byte_size() + 7) / 8) * 8; }

      protected:
        virtual data_type type() const = 0;
        constexpr uint32_t header_size() const { return 8; }
        virtual uint32_t byte_size() const = 0;
        virtual void write(std::ostream &os) const = 0;
    };

    class matrix : public data_element
    {
        const std::string &m_name;
        const std::vector<std::pair<double, double>> &m_data;
        std::array<int32_t, 2> m_dimensions_array;

      protected:
        data_type type() const override { return data_type::matrix; }
        uint32_t byte_size() const override;
        void write(std::ostream &os) const override;

      public:
        matrix(const std::string &name, const std::vector<std::pair<double, double>> &data)
            : m_name(name), m_data(data), m_dimensions_array{2, (int32_t)data.size()}
        {
        }
    };

    class compressed_section_priv;
    class compressed_section : public std::streambuf, public std::ostream, public data_element
    {
        std::vector<uint8_t> m_buffer;
        std::unique_ptr<compressed_section_priv> m_data;

      protected:
        int overflow(int c) override;

        data_type type() const override { return data_type::compressed; };
        uint32_t byte_size() const override;
        void write(std::ostream &os) const override;

      public:
        void finish();

        compressed_section(int level);
        ~compressed_section();
    };

    std::ostream &operator<<(std::ostream &str, const header &);
    std::ostream &operator<<(std::ostream &str, const data_element &);
    // std::ostream &operator<<(std::ostream &str, const matrix &matrix);
    // std::ostream &operator<<(std::ostream &str, const compressed_section &);
} // namespace mat