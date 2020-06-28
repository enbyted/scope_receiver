#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>

#include "connection.h"
#include "scope.h"
#include "mat_writer.h"

#include <fstream>
#include <sstream>
#include <spdlog/fmt/ostr.h>
#include <cxxopts.hpp>

void read_channel_data(rigol::scope& scope, rigol::channel ch, std::vector<std::pair<double, double>>& data)
{
    std::vector<uint8_t> buffer;
    scope.select_channel(ch);
    scope.read_buffer(buffer);

    double x_origin = scope.x_origin();
    double x_increment = scope.x_increment();
    double x_reference = scope.x_reference();

    double y_origin = scope.y_origin();
    double y_increment = scope.y_increment();
    double y_reference = scope.y_reference();

    data.resize(buffer.size());

    for (size_t i = 0; i < buffer.size(); i++)
    {
        data[i].first = x_origin + (i - x_reference)*x_increment;
        data[i].second = (buffer[i] - y_reference - y_origin) * y_increment;
    }
    spdlog::info("Read {} items", data.size());
}

enum class trigger_mode
{
    STOP,
    SINGLE,
};

void wait_for_trigger(rigol::scope& scope, trigger_mode trigger)
{
    switch (trigger)
    {
        case trigger_mode::STOP:
            spdlog::info("Stopping the scope");
            scope.stop();
            break;

        case trigger_mode::SINGLE:
            spdlog::info("Arming the scope");
            scope.single();
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            spdlog::info("Waiting for trigger");
            while (scope.get_trigger_state() != rigol::trigger_state::STOP)
                ;
            
            break;
    }
}

int main(int argc, char**argv)
{
    cxxopts::Options options("scope_receiver", "Data receiver from Rigol DS1054 scope that saves data in MATLAB mat file format");

    options.add_options()
        ("silent", "Print only errors to the console")
        ("v,verbose", "Enable verbose output")
        ("f,outfile", "Output filename", cxxopts::value<std::string>())
        ("s,scopeip", "Scope's IP address", cxxopts::value<std::string>())
        ("p,scopeport", "Scope's port number", cxxopts::value<uint16_t>()->default_value("5555"))
        ("c,channels", "Channels to read, list (not separated) of one or more of: 1, 2, 3, 4", cxxopts::value<std::string>()->default_value("1234"))
        ("t,trigger", "Trigger mode, one of: stop, single", cxxopts::value<std::string>())
        ("h,help", "Print usage")
    ;

    spdlog::set_level(spdlog::level::info);

    try
    {
        auto parsed_options = options.parse(argc, argv);

        if (parsed_options.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (parsed_options.count("verbose"))
            spdlog::set_level(spdlog::level::debug);

        if (parsed_options.count("silent"))
            spdlog::set_level(spdlog::level::warn);

        if (! parsed_options.count("scopeip"))
            throw cxxopts::OptionParseException("argument --scopeip is required");

        if (! parsed_options.count("outfile"))
            throw cxxopts::OptionParseException("argument --outfile is required");

        trigger_mode trigger;

        if (! parsed_options.count("trigger"))
            throw cxxopts::OptionParseException("argument --trigger is required");
        else if (auto trigger_val = parsed_options["trigger"].as<std::string>(); trigger_val == "stop")
            trigger = trigger_mode::STOP;
        else if (trigger_val == "single")
            trigger = trigger_mode::SINGLE;
        else
            throw cxxopts::OptionParseException(fmt::format("'{}' is not a valid trigger mode, expected stop or single", trigger_val));

        std::vector<rigol::channel> channels;

        {
            std::string value = parsed_options["channels"].as<std::string>();
            for (char ch : value)
            {
                switch (ch)
                {
                    case '1': channels.push_back(rigol::channel::CHANNEL_1); break;
                    case '2': channels.push_back(rigol::channel::CHANNEL_2); break;
                    case '3': channels.push_back(rigol::channel::CHANNEL_3); break;
                    case '4': channels.push_back(rigol::channel::CHANNEL_4); break;
                    default: throw cxxopts::OptionParseException(fmt::format("'{}' is not a valid channel specifier, expected on of: 1,2,3,4", ch));
                }
            }
        }

        rigol::scope scope(std::make_unique<rigol::tcp_connection>(
            parsed_options["scopeip"].as<std::string>(), 
            parsed_options["scopeport"].as<uint16_t>()
        ));

        wait_for_trigger(scope, trigger);

        std::ofstream file(parsed_options["outfile"].as<std::string>(), std::ios::binary | std::ios::trunc | std::ios::out);
        file << mat::header{};

        std::vector<std::pair<double, double>> buffer;
        for (auto ch : channels) 
        {
            spdlog::info("Reading data for {}", ch);
            read_channel_data(scope, ch, buffer);

            spdlog::info("Saving data for {}", ch);
            file << mat::matrix{fmt::format("{}", ch), buffer};
        }

        spdlog::info("Done");
    }
    catch (const cxxopts::OptionParseException& ex)
    {
        spdlog::error("{}" , ex.what());
        std::cout << options.help() << std::endl;
        return 1;
    }
    catch (const std::exception& ex)
    {
        spdlog::error("Error during execution: {}", ex.what());
        return 2;
    }
    
    return 0;
}