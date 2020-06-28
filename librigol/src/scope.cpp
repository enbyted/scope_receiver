#include "scope.h"
#include "scpi_command.h"
#include <stdexcept>
#include <string_view>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <charconv>

namespace rigol
{
    scope::scope(std::unique_ptr<connection>&& connection)
        : m_connection(std::move(connection))
    {

    }

    void scope::run()
    {
        no_response_scpi_command({"RUN"}).run_on(*m_connection);
    }

    void scope::stop()
    {
        no_response_scpi_command({"STOP"}).run_on(*m_connection);
    }

    void scope::single()
    {
        no_response_scpi_command({"SING"}).run_on(*m_connection);
    }

    trigger_state scope::get_trigger_state()
    {
        text_query_scpi_command cmd{"TRIG", "STAT"};
        cmd.run_on(*m_connection);

        if (cmd.last_response() == "TG")
            return trigger_state::TG;
        
        if (cmd.last_response() == "WAIT")
            return trigger_state::WAIT;
        
        if (cmd.last_response() == "RUN")
            return trigger_state::RUN;
        
        if (cmd.last_response() == "AUTO")
            return trigger_state::AUTO;
        
        if (cmd.last_response() == "STOP")
            return trigger_state::STOP;
        
        throw std::logic_error(fmt::format("Unknown trigger state response '{}'", cmd.last_response()));
    }

    void scope::select_channel(channel ch)
    {   
        switch (ch)
        {
            case channel::CHANNEL_1: 
                no_response_scpi_command({"WAV", "SOUR"}, "CHAN1").run_on(*m_connection);
                return;
            case channel::CHANNEL_2: 
                no_response_scpi_command({"WAV", "SOUR"}, "CHAN2").run_on(*m_connection);
                return;
            case channel::CHANNEL_3: 
                no_response_scpi_command({"WAV", "SOUR"}, "CHAN3").run_on(*m_connection);
                return;
            case channel::CHANNEL_4: 
                no_response_scpi_command({"WAV", "SOUR"}, "CHAN4").run_on(*m_connection);
                return;
        }

        throw std::logic_error("Invalid channel");
    }

    void scope::read_buffer(std::vector<float>& buffer)
    {
        text_query_scpi_command get_memory_depth{"ACQ", "MDEP"};
        get_memory_depth.run_on(*m_connection);

        if (get_memory_depth.last_response() == "AUTO")
            throw std::logic_error("Cannot read buffer with 'AUTO' memory depth");

        std::size_t memory_depth = (std::size_t)std::atol(get_memory_depth.last_response().c_str());

        buffer.clear();
        buffer.reserve(memory_depth);

        text_query_scpi_command get_data{"WAV", "DATA"};
        no_response_scpi_command({"WAV", "MODE"}, "RAW").run_on(*m_connection);
        no_response_scpi_command({"WAV", "FORM"}, "ASC").run_on(*m_connection);

        constexpr std::size_t BATCH_SIZE = 15625;
        for (std::size_t i = 0; i < memory_depth; i += BATCH_SIZE)
        {
            const std::size_t to_read = std::min(BATCH_SIZE, memory_depth - i);

            no_response_scpi_command({"WAV", "START"}, fmt::format("{}", i + 1)).run_on(*m_connection);
            no_response_scpi_command({"WAV", "STOP"}, fmt::format("{}", i + to_read)).run_on(*m_connection);
            get_data.run_on(*m_connection);
            const std::string& resp = get_data.last_response();
            const std::string_view header{&*resp.begin(), 2};
            const std::string_view s_count{&*resp.begin()+2, 9};
            std::string_view rest{&*resp.begin() + 11, resp.size() - 11};

            if (header != "#9")
                throw std::logic_error(fmt::format("Invalid data header, expected #9. Whole line: {}", resp));
            
            std::size_t temp = 0;
            while (! rest.empty())
            {
                auto comma = rest.find_first_of(',');
                std::string_view value = rest;
                if (comma != std::string_view::npos)
                {
                    value = value.substr(0, comma);
                    rest = rest.substr(comma + 1);
                }
                else
                {
                    rest = std::string_view{};
                }
                temp++;
                float f_value = (float)strtod(value.cbegin(), nullptr);
                buffer.push_back(f_value);
            }
            spdlog::debug("Read {} floats", temp);

        }
    }

    void scope::read_buffer(std::vector<uint8_t>& buffer)
    {
        text_query_scpi_command get_memory_depth{"ACQ", "MDEP"};
        get_memory_depth.run_on(*m_connection);

        if (get_memory_depth.last_response() == "AUTO")
            throw std::logic_error("Cannot read buffer with 'AUTO' memory depth");

        std::size_t memory_depth = (std::size_t)std::atol(get_memory_depth.last_response().c_str());

        buffer.clear();
        buffer.reserve(memory_depth);

        no_response_scpi_command({"WAV", "MODE"}, "RAW").run_on(*m_connection);
        no_response_scpi_command({"WAV", "FORM"}, "BYTE").run_on(*m_connection);

        constexpr std::size_t BATCH_SIZE = 250000;
        std::size_t count = 0;
        for (std::size_t i = 0; i < memory_depth; i += count)
        {
            const std::size_t to_read = std::min(BATCH_SIZE, memory_depth - i);

            no_response_scpi_command({"WAV", "START"}, fmt::format("{}", i + 1)).run_on(*m_connection);
            no_response_scpi_command({"WAV", "STOP"}, fmt::format("{}", i + to_read)).run_on(*m_connection);

            no_response_scpi_command({"WAV", "DATA?"}).run_on(*m_connection);
            std::string resp;
            m_connection->read(resp, 11);
            const std::string_view header{&*resp.begin(), 2};
            const std::string_view s_count{&*resp.begin()+2, 9};

            if (header != "#9")
                throw std::logic_error(fmt::format("Invalid data header, expected #9. Whole line: {}", resp));

            auto ret = std::from_chars(s_count.begin(), s_count.end(), count, 10);
            if (ret.ec != std::errc())
                throw std::system_error((int)ret.ec, std::generic_category(), "Cannot interpter number of bytes to read");

            m_connection->read(resp, count+1);
            std::string_view rest{&*resp.begin(), resp.size() - 1};

            buffer.insert(buffer.end(), rest.cbegin(), rest.cend());
            spdlog::debug("Read {} uint8_t's", count);
        }
    }

    double scope::x_origin()
    {
        text_query_scpi_command cmd{"WAV", "XOR"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }

    double scope::x_increment()
    {
        text_query_scpi_command cmd{"WAV", "XINC"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }

    double scope::x_reference()
    {
        text_query_scpi_command cmd{"WAV", "XREF"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }

    double scope::y_origin()
    {
        text_query_scpi_command cmd{"WAV", "YOR"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }

    double scope::y_increment()
    {
        text_query_scpi_command cmd{"WAV", "YINC"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }

    double scope::y_reference()
    {
        text_query_scpi_command cmd{"WAV", "YREF"};
        cmd.run_on(*m_connection);
        return strtod(&*cmd.last_response().begin(), nullptr);
    }
}