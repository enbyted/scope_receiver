#pragma once

#include <memory>
#include <vector>
#include <ostream>
#include "connection.h"

namespace rigol
{
    enum class trigger_state
    {
        TG,
        WAIT,
        RUN,
        AUTO,
        STOP
    };

    enum class channel
    {
        CHANNEL_1,
        CHANNEL_2,
        CHANNEL_3,
        CHANNEL_4,
    };

    inline std::ostream& operator<<(std::ostream& str, channel ch)
    {
        switch (ch)
        {
            case channel::CHANNEL_1: str << "CHANNEL_1"; return str;
            case channel::CHANNEL_2: str << "CHANNEL_2"; return str;
            case channel::CHANNEL_3: str << "CHANNEL_3"; return str;
            case channel::CHANNEL_4: str << "CHANNEL_4"; return str;
        }
        str << "Unknown channel";
        return str;
    }

    class scope
    {
        std::unique_ptr<connection> m_connection;
    public:
        scope(std::unique_ptr<connection>&& connection);
        
        void run();
        void stop();
        void single();

        trigger_state get_trigger_state();

        void select_channel(channel ch);

        void read_buffer(std::vector<float>& buffer);
        void read_buffer(std::vector<uint8_t>& buffer);

        double x_origin();
        double x_increment();
        double x_reference();
        double y_origin();
        double y_increment();
        double y_reference();
    };
}