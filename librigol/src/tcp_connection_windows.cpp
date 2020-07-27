#ifdef __WIN32__
#include "connection.h"
#include "spdlog/spdlog.h"

#include <stdexcept>
#include <system_error>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace
{
    struct winsock_error_category_t : std::error_category
    {
        const char *name() const noexcept override { return "winsock"; }
        std::string message(int ev) const override
        {
            char *s = nullptr;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                          nullptr, ev, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, nullptr);

            return fmt::format("{} (code: {})", s, ev);
        }
    };
    const winsock_error_category_t &winsock_error_category()
    {
        static winsock_error_category_t cat;
        return cat;
    }

    struct startup_guard
    {
        WSADATA wsaData;
        startup_guard() { WSAStartup(MAKEWORD(2, 2), &wsaData); }
        ~startup_guard() { WSACleanup(); }
    };

    const startup_guard the_startup_guard;
} // namespace

namespace rigol
{
    tcp_connection::tcp_connection(const std::string &address, std::uint16_t port)
    {
        struct sockaddr_in scope_addr;
        scope_addr.sin_family = AF_INET;
        scope_addr.sin_port = htons(port);

        if (int ret = inet_pton(AF_INET, address.c_str(), &scope_addr.sin_addr); ret == 0)
            throw std::runtime_error("Provided string doesn't contain valid address");
        else if (ret == -1)
            throw std::system_error(errno, std::generic_category(), "Provided string contains unsupported address");

        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_fd == INVALID_SOCKET)
            throw std::system_error(WSAGetLastError(), winsock_error_category(), "Cannot create socket");

        if (connect(m_fd, (struct sockaddr *)&scope_addr, sizeof(scope_addr)) == -1)
            throw std::system_error(WSAGetLastError(), winsock_error_category(), "Cannot connect to scope");

        spdlog::info("Connected to scope on {}:{} (file descriptor: {})", address, port, m_fd);
    }

    tcp_connection::~tcp_connection()
    {
        if (m_fd != INVALID_SOCKET)
        {
            spdlog::info("Closing connection to scope (file descriptor: {})", m_fd);
            shutdown(m_fd, SD_SEND);
            closesocket(m_fd);
            m_fd = INVALID_SOCKET;
        }
    }

    std::size_t tcp_connection::read(std::uint8_t *buffer, std::size_t max_len)
    {
        int ret = recv(m_fd, (char *)buffer, (int)max_len, 0);
        if (ret == -1)
            throw std::system_error(errno, std::system_category(), "Cannot receive from scope");

        return (std::size_t)ret;
        return 0;
    }

    std::size_t tcp_connection::write(const std::uint8_t *buffer, std::size_t max_len)
    {
        int ret = send(m_fd, (char *)buffer, (int)max_len, 0);
        if (ret == -1)
            throw std::system_error(errno, std::system_category(), "Cannot send to scope");

        return (std::size_t)ret;
        return 0;
    }

} // namespace rigol
#endif