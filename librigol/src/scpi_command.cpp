#include "scpi_command.h"
#include "connection.h"

#include <cstddef>
#include <numeric>
#include <spdlog/spdlog.h>

namespace rigol
{
    no_response_scpi_command::no_response_scpi_command(const std::string &command) : m_command(command) {}

    no_response_scpi_command::no_response_scpi_command(const std::initializer_list<std::string> &parts,
                                                       const std::string &args)
    {
        const std::size_t len = std::accumulate(parts.begin(), parts.end(), 0,
                                                [](std::size_t siz, const std::string &s) { return siz + s.size(); });
        const std::size_t args_size = args.size();
        m_command.reserve(len + parts.size() + (args_size > 0 ? args_size + 1 : 0) + 1);
        for (const std::string &s : parts)
        {
            m_command.push_back(':');
            m_command.insert(m_command.end(), s.cbegin(), s.cend());
        }

        if (args_size > 0)
        {
            m_command.push_back(' ');
            m_command.insert(m_command.end(), args.cbegin(), args.cend());
        }
        m_command.push_back('\n');
    }

    void no_response_scpi_command::run_on(connection &connection)
    {
        spdlog::debug("Sending command: {}", std::string_view(m_command).substr(0, m_command.size() - 1));
        connection.write(m_command);
    }

    text_query_scpi_command::text_query_scpi_command(const std::string &command) : m_command(command) {}

    text_query_scpi_command::text_query_scpi_command(const std::initializer_list<std::string> &parts)
    {
        const std::size_t len = std::accumulate(parts.begin(), parts.end(), 0,
                                                [](std::size_t siz, const std::string &s) { return siz + s.size(); });
        m_command.reserve(len + parts.size() + 2);
        for (const std::string &s : parts)
        {
            m_command.push_back(':');
            m_command.insert(m_command.end(), s.cbegin(), s.cend());
        }

        m_command.push_back('?');
        m_command.push_back('\n');
    }

    void text_query_scpi_command::run_on(connection &connection)
    {
        spdlog::debug("Sending query: {}", std::string_view(m_command).substr(0, m_command.size() - 1));
        connection.write(m_command);
        m_last_response.clear();
        connection.read_line(m_last_response);
        spdlog::debug("Got response: {}", m_last_response);
    }
} // namespace rigol