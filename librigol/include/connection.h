#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace rigol
{
    class connection
    {
    private:
        connection(const connection&) = delete;
        connection& operator=(const connection&) = delete;

    protected:
        connection() {};

        virtual std::size_t read(std::uint8_t* buffer, std::size_t max_len) = 0;
        virtual std::size_t write(const std::uint8_t* buffer, std::size_t max_len) = 0;
    public:
        virtual ~connection() {};

        void write(const std::string& s);
        void read_line(std::string& out);
        void read(std::string& out, size_t count);
        std::string read_line();
    };

    class tcp_connection : public connection
    {
        int m_fd;
    public:
        tcp_connection(const std::string& address, std::uint16_t port);
        ~tcp_connection();
    
    protected:
        std::size_t read(std::uint8_t* buffer, std::size_t max_len) override;
        std::size_t write(const std::uint8_t* buffer, std::size_t max_len) override;
    };
}