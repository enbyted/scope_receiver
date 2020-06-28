#include "connection.h"
#include "spdlog/spdlog.h"

#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <system_error>
#include <stdexcept>

namespace rigol
{
    tcp_connection::tcp_connection(const std::string& address, std::uint16_t port)
    {
        struct sockaddr_in scope_addr;
        scope_addr.sin_family = AF_INET;
        scope_addr.sin_port = htons(port);
        
        if (int ret = inet_pton(AF_INET, address.c_str(), &scope_addr.sin_addr); ret == 0)
            throw std::runtime_error("Provided string doesn't contain valid address");
        else if (ret == -1)
            throw std::system_error(errno, std::system_category(), "Provided string contains unsupported address");

        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_fd == -1)
            throw std::system_error(errno, std::system_category(), "Cannot create socket");

        if (connect(m_fd, (struct sockaddr*)&scope_addr, sizeof(scope_addr)) == -1)
            throw std::system_error(errno, std::system_category(), "Cannot connect to scope");
        
        spdlog::info("Connected to scope on {}:{} (file descriptor: {})", address, port, m_fd);
    }

    tcp_connection::~tcp_connection()
    {
        if (m_fd != -1)
        {
            spdlog::info("Closing connection to scope (file descriptor: {})", m_fd);
            close(m_fd);
            m_fd = 1;
        }
    }

    std::size_t tcp_connection::read(std::uint8_t* buffer, std::size_t max_len)
    {
        ssize_t ret = recv(m_fd, buffer, max_len, 0);
        if (ret == -1)
            throw std::system_error(errno, std::system_category(), "Cannot receive from scope");
        
        return (std::size_t) ret;
    }

    std::size_t tcp_connection::write(const std::uint8_t* buffer, std::size_t max_len)
    {
        ssize_t ret = send(m_fd, buffer, max_len, 0);
        if (ret == -1)
            throw std::system_error(errno, std::system_category(), "Cannot send to scope");
        
        return (std::size_t) ret;
    }

}