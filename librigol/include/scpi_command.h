#pragma once

#include <string>
#include <initializer_list>

namespace rigol
{
    class connection;

    class scpi_command
    {
    protected:
        scpi_command() {};

    public:
        virtual ~scpi_command() {};

        virtual void run_on(connection& connection) = 0;
    };

    class no_response_scpi_command : public scpi_command
    {
        std::string m_command;
    public:
        no_response_scpi_command(const std::string& command);
        no_response_scpi_command(const std::initializer_list<std::string>& parts, const std::string& args = {});

        void run_on(connection& connection) override;
    };

    class text_query_scpi_command : public scpi_command
    {
        std::string m_command;
        std::string m_last_response;
    public:
        text_query_scpi_command(const std::string& command);
        text_query_scpi_command(const std::initializer_list<std::string>& parts);

        void run_on(connection& connection) override;
        const std::string& last_response() const { return m_last_response; };
    };
}