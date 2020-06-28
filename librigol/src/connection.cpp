#include "connection.h"

namespace rigol
{
    void connection::write(const std::string& s)
    {
        auto iter = s.cbegin();
        while (iter != s.cend())
            iter += write((const std::uint8_t*)&*iter, s.cend() - iter);
    }

    std::string connection::read_line()
    {
        std::string ret;
        read_line(ret);
        return ret;
    }

    void connection::read_line(std::string& out)
    {
        char ch;

        while (true)
        {
            if (read((uint8_t*)&ch, 1) == 1)
            {
                if (ch == '\n')
                    return;
                
                out.push_back(ch);
            }
        }
    }

    void connection::read(std::string& out, size_t count)
    {
        out.resize(count);
        auto pos = out.begin();
        while (count > 0)
        {
            size_t cnt = read((uint8_t*)&*pos, count);
            count -= cnt;
            pos += cnt;
        }
    }
}