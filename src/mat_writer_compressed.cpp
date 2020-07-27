#include "mat_writer.h"
#include "mat_writer_p.h"
#include <cassert>
#include <spdlog/spdlog.h>
#include <zlib.h>

namespace mat
{
    struct ZlibDeflate
    {
        z_stream strm;

        ZlibDeflate(int level)
        {
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;

            int ret = deflateInit(*this, level);
            if (ret != Z_OK)
                throw std::runtime_error("Cannot initialize zlib deflate");
        }

        ~ZlibDeflate() { deflateEnd(*this); }

        void setInBuffer(const std::uint8_t *buffer, std::size_t size)
        {
            strm.avail_in = size;
            strm.next_in = const_cast<std::uint8_t *>(buffer);
        }

        std::size_t inBufferSpace() const { return strm.avail_in; }

        void setOutBuffer(std::uint8_t *buffer, std::size_t size)
        {
            strm.avail_out = size;
            strm.next_out = buffer;
        }

        std::size_t outBufferSpace() const { return strm.avail_out; }

        operator z_stream *() { return &strm; }
    };

    class compressed_section_priv
    {
      public:
        size_t written = 0;
        std::array<uint8_t, 16384> buffer_in;
        std::array<uint8_t, 16384> buffer_out;
        ZlibDeflate zlib;
        int ret;

        compressed_section_priv(int level) : zlib{level}
        {
            zlib.setInBuffer(buffer_in.cbegin(), 0);
            zlib.setOutBuffer(buffer_out.begin(), buffer_out.size());
        }

        bool has_data_to_read() { return zlib.outBufferSpace() != buffer_out.size(); }
        bool is_out_buffer_full() { return zlib.outBufferSpace() == 0; }
        bool finished() { return ret == Z_STREAM_END; }

        void readout_data(std::vector<uint8_t> &buffer)
        {
            size_t to_read = buffer_out.size() - zlib.outBufferSpace();
            if (to_read == 0)
                return;

            buffer.insert(buffer.end(), buffer_out.cbegin(), buffer_out.cbegin() + to_read);
            zlib.setOutBuffer(buffer_out.begin(), buffer_out.size());
        }

        bool process_buffer(bool finish)
        {
            if (zlib.inBufferSpace() == 0 && written > 0)
            {
                zlib.setInBuffer(buffer_in.cbegin(), written);
                written = 0;
            }
            ret = deflate(zlib, finish ? Z_FINISH : Z_NO_FLUSH);

            switch (ret)
            {
            case Z_STREAM_ERROR:
                throw std::runtime_error("ZLIB stream error");
            case Z_NEED_DICT:
                throw std::runtime_error("ZLIB need dict");
            case Z_DATA_ERROR:
                throw std::runtime_error("ZLIB data error");
            case Z_MEM_ERROR:
                throw std::runtime_error("ZLIB memory error");
            }

            if (ret == Z_STREAM_END)
                return false;

            return zlib.outBufferSpace() == 0;
        }

        bool process_byte(uint8_t ch)
        {
            assert(written < buffer_in.size());
            buffer_in[written++] = ch;
            return (written == buffer_in.size());
        }
    };

    compressed_section::compressed_section(int level)
        : std::ostream(this), m_data(std::make_unique<compressed_section_priv>(level))
    {
    }

    compressed_section::~compressed_section() {}

    void compressed_section::finish()
    {
        while (!m_data->finished())
        {
            while (!m_data->is_out_buffer_full() && !m_data->finished())
                m_data->process_buffer(true);

            m_data->readout_data(m_buffer);
        }
    }

    int compressed_section::overflow(int c)
    {
        if (m_data->finished())
            throw std::logic_error("Tried to insert more data into finished buffer");

        if (m_data->process_byte(c))
        {
            while (m_data->process_buffer(false))
                m_data->readout_data(m_buffer);

            assert(m_data->zlib.inBufferSpace() == 0);
        }

        return c;
    }

    uint32_t compressed_section::byte_size() const
    {
        if (!m_data->finished())
            throw std::logic_error("Tried to get size of not finished buffer");

        return m_buffer.size();
    }

    void compressed_section::write(std::ostream &os) const
    {
        if (!m_data->finished())
            throw std::logic_error("Tried to get size of not finished buffer");

        os.write((const char *)&*m_buffer.begin(), m_buffer.size());
    }

} // namespace mat
