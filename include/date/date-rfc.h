//
// The MIT License (MIT)
//
// Copyright (c) 2018 Yury Prostov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once
#include <string>
#include <cctype>
#include <istream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <type_traits>
#include <tuple>
#include <cassert>

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                common
// ----------------------------------------------------------------------------
template <typename Format, typename Date, typename Char, typename Traits, class Alloc = std::allocator<Char>>
std::basic_istream<Char, Traits>& from_stream(std::basic_istream<Char, Traits>& stream, const Format& fmt, Date& value)
{
    typedef Format dt_format;
    typedef std::basic_string<Char, Traits, Alloc> dt_zone;

    dt_parts parts{};
    dt_zone  zone{};
    int      offset{};

    parts = dt_format::read<Char, Traits, Alloc>(stream, fmt, &zone, &offset);
    value = dt_traits<Date>::join<>(parts, &zone, &offset);
    return stream;
}

// ----------------------------------------------------------------------------
//                                traits
// ----------------------------------------------------------------------------
struct dt_parts
{
    uint32_t nanosecond;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  weekday;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
};

// ----------------------------------------------------------------------------
template <class Date>
struct dt_traits;

// ----------------------------------------------------------------------------
//                                details
// ----------------------------------------------------------------------------
namespace details
{

// ----------------------------------------------------------------------------
//                               constants
// ----------------------------------------------------------------------------
inline std::pair<const char* const*, const char* const*> weekday_names_short()
{
    static const char* values[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    return std::make_pair(values, values + sizeof(values) / sizeof(values[0]));
}

// ----------------------------------------------------------------------------
inline std::pair<const char* const*, const char* const*> month_names_short()
{
    static const char* values[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    return std::make_pair(values, values + sizeof(values) / sizeof(values[0]));
}

// ----------------------------------------------------------------------------
inline std::pair<const char* const*, const char* const*> zone_names_rfc822()
{
    //! Not including military codes (each code encoded by one symbol from 'A' to 'Z').
    static const char* values[] = { "GMT", "UT", "EST", "EDT", "CST", "CDT", "MST", "MDT", "PST", "PDT" };
    return std::make_pair(values, values + sizeof(values) / sizeof(values[0]));
}

// ----------------------------------------------------------------------------
//                       reading implementation
// ----------------------------------------------------------------------------
template <class T, class ...Args>
struct args_traits;

template <class T>
struct args_traits<T>
{
    enum : unsigned { max_length = T::max_length };
    enum : bool     { need_cache = T::need_cache };
};

template <class T, class ...Args>
struct args_traits
{
    enum : unsigned { max_length = T::max_length + args_traits<Args...>::max_length };
    enum : bool     { need_cache = T::need_cache | args_traits<Args...>::need_cache };
};

// ----------------------------------------------------------------------------
template <class CharT>
struct rc_t
{
    typedef CharT value_type;
    enum : unsigned { min_length = 1 };
    enum : unsigned { max_length = 1 };
    enum : bool     { need_cache = false };

    CharT value;
};

template <class CharT>
static rc_t<CharT> rc(CharT value)
{
    return rc_t<CharT>{ value };
}

// ----------------------------------------------------------------------------
template <class UnsignedT, unsigned MinLength, unsigned MaxLength>
struct ru_t
{
    typedef UnsignedT value_type;
    enum : unsigned { min_length = MinLength };
    enum : unsigned { max_length = MaxLength };
    enum : bool     { need_cache = false };

    value_type& value;
};

template <unsigned MinLength, unsigned MaxLength, class UnsignedT>
static ru_t<UnsignedT, MinLength, MaxLength> ru(UnsignedT& value)
{
    return ru_t<UnsignedT, MinLength, MaxLength>{ value };
}

// ----------------------------------------------------------------------------
template <class SignedT, unsigned MinLength, unsigned MaxLength>
struct rs_t
{
    typedef SignedT value_type;
    enum : unsigned { min_length = MinLength };
    enum : unsigned { max_length = MaxLength };
    enum : bool     { need_cache = false };

    value_type& value;
};

template <unsigned MinLength, unsigned MaxLength, class SignedT>
static rs_t<SignedT, MinLength, MaxLength> rs(SignedT& value)
{
    return rs_t<SignedT, MinLength, MaxLength>{ value };
}

// ----------------------------------------------------------------------------
template <class IndexT, class IteratorT, unsigned MaxLength>
struct ra_t
{
    typedef IndexT      index_type;
    typedef IteratorT   iterator_type;
    enum : unsigned { max_length = MaxLength };
    enum : bool     { need_cache = false };

    ra_t(index_type& _index, const iterator_type& _values_begin, const iterator_type& _values_end)
        : index(_index), values_begin(_values_begin), values_end(_values_end) {}

    index_type&   index;
    iterator_type values_begin;
    iterator_type values_end;
};

template <unsigned MaxLength, class IndexT, class IteratorT>
static ra_t<IndexT, IteratorT, MaxLength> ra(IndexT& index, const IteratorT& values_begin, const IteratorT& values_end)
{
    return ra_t<IndexT, IteratorT, MaxLength>{ index, values_begin, values_end };
}

template <unsigned MaxLength, class IndexT, class IteratorT>
static ra_t<IndexT, IteratorT, MaxLength> ra(IndexT& index, const std::pair<IteratorT, IteratorT>& values_range)
{
    return ra<MaxLength>(index, values_range.first, values_range.second);
}

// ----------------------------------------------------------------------------
template <class ...Args>
struct optional_t
{
    enum : unsigned { max_length = args_traits<Args...>::max_length };
    enum : bool     { need_cache = true };

    optional_t(Args&& ...args) 
        : opts(std::forward<Args>(args)...) {}

    std::tuple<Args...> opts;
};

template <class ...Args>
optional_t<Args...> optional(Args&& ...args)
{
    return optional_t<Args...>(std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits>
void read_char(std::basic_istream<CharT, Traits>& stream, size_t& pos, CharT fmt)
{
    const auto ic = stream.get();
    ++pos;

    if (Traits::eq_int_type(ic, Traits::eof()) || !Traits::eq(Traits::to_char_type(ic), fmt))
        throw std::logic_error(std::string("invalid char format at ") + std::to_string(pos));
}

// ----------------------------------------------------------------------------
template <class UnsignedT, class CharT, class Traits>
UnsignedT read_unsigned(std::basic_istream<CharT, Traits>& stream, size_t& pos, unsigned min_len, unsigned max_len)
{
    UnsignedT x = 0;
    unsigned count = 0;
    while (true)
    {
        const auto ic = stream.peek();
        if (Traits::eq_int_type(ic, Traits::eof()))
            break;

        const auto c = static_cast<char>(Traits::to_char_type(ic));
        if (!('0' <= c && c <= '9'))
            break;
        
        (void)stream.get();
        ++pos;

        x = 10 * x + static_cast<UnsignedT>(c - '0');
        if (++count == max_len)
            break;
    }

    if (count < min_len)
        throw std::logic_error(std::string("invalid unsigned format at ") + std::to_string(pos));

    return x;
}

// ----------------------------------------------------------------------------
template <class SignedT, class CharT, class Traits>
SignedT read_signed(std::basic_istream<CharT, Traits>& stream, size_t& pos, unsigned min_len, unsigned max_len)
{
    const auto ic = stream.peek();
    if (Traits::eq_int_type(ic, Traits::eof()))
    {
        if (min_len > 0)
            throw std::logic_error(std::string("invalid signed format at ") + std::to_string(pos));
        return 0;
    }
    
    const auto c = static_cast<char>(Traits::to_char_type(ic));
    const bool positive = (c != '-');
    if (c == '-' || c == '+')
    {
        (void)stream.get();
        ++pos;
    }
        
    const auto x = read_unsigned<SignedT>(stream, pos, min_len, max_len);
    return (positive) ? x : -x;
}

// ----------------------------------------------------------------------------
enum : unsigned { abbr_max_len = 32 };

template <class IndexT, class IteratorT, class CharT, class Traits, class Alloc = std::allocator<CharT>>
IndexT read_abbr(std::basic_istream<CharT, Traits>& stream, size_t& pos, const IteratorT& values_begin, const IteratorT& values_end, unsigned max_len)
{
    CharT buffer[abbr_max_len];
    std::memset(buffer, 0, sizeof(buffer[0]) * (max_len + 1));
    
    for (size_t index = 0; index < max_len; ++index)
    {
        const auto ic = stream.peek();
        if (Traits::eq_int_type(ic, Traits::eof()))
            throw std::logic_error(std::string("unexpected eof at ") + std::to_string(pos));

        (void)stream.get(); ++pos;
        buffer[index] = Traits::to_char_type(ic);

        const auto it = std::find(values_begin, values_end, std::basic_string<CharT, Traits, Alloc>(buffer));
        if (it != values_end)
            return static_cast<IndexT>(it - values_begin);
    }
    
    throw std::logic_error(std::string("invalid abbr format at ") + std::to_string(pos));
}

// ----------------------------------------------------------------------------
template<unsigned N>
struct read_opts_t
{
    template <class CharT, class Traits, typename... TupleT>
    static void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& opts)
    {
        details::read_opts_t<N - 1>::read_impl(stream, pos, opts);
        details::read_impl(stream, pos, std::get<N - 1>(opts));
    }
};

template<>
struct read_opts_t<0>
{
    template <class CharT, class Traits, typename... TupleT>
    static void read_impl(std::basic_istream<CharT, Traits>&, size_t&, std::tuple<TupleT...>&)
    {}
};

template<class CharT, class Traits, typename... TupleT>
void read_opts(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& opts)
{
    const size_t begin = pos;
    try
    {
        read_opts_t<sizeof...(TupleT)>::read_impl(stream, pos, opts);
    }
    catch (const std::logic_error&)
    {
        for (; pos > begin; --pos)
            stream.unget();
    }
}

// ----------------------------------------------------------------------------
template <class Char, class Traits>
void skip_spaces(std::basic_istream<Char, Traits>& stream, size_t& pos)
{
    for (;;)
    {
        const auto ic = stream.peek();
        if (stream.fail())
            throw std::logic_error(std::string("unexpected fail at ") + std::to_string(pos));
        
        const auto i = static_cast<int>(Traits::to_int_type(ic));
        if (i != 0x20) //!< Space (SPC) symbol.
            break;

        (void)stream.get();
        ++pos;
    }
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos)
{}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, rc_t<CharT> a0, Args&& ...args)
{
    if (a0.value != CharT{})
        read_char(stream, pos, a0.value);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class UnsignedT, unsigned MinLength, unsigned MaxLength, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, ru_t<UnsignedT, MinLength, MaxLength> a0, Args&& ...args)
{
    a0.value = read_unsigned<UnsignedT>(stream, pos, MinLength, MaxLength);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class SignedT, unsigned MinLength, unsigned MaxLength, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, rs_t<SignedT, MinLength, MaxLength> a0, Args&& ...args)
{
    a0.value = read_signed<SignedT>(stream, pos, MinLength, MaxLength);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class IndexT, class IteratorT, unsigned MaxLength, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, ra_t<IndexT, IteratorT, MaxLength> a0, Args&& ...args)
{
    static_assert(MaxLength < abbr_max_len, "Exceeded the allowed length for ra object passed into read operation.");
    a0.index = read_abbr<IndexT>(stream, pos, a0.values_begin, a0.values_end, MaxLength);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class ...OptArgs, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, optional_t<OptArgs...> a0, Args&& ...args)
{
    read_opts(stream, pos, a0.opts);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
//                              cached stream
// ----------------------------------------------------------------------------
template <class CharT, class Traits, std::size_t CacheSize>
class cached_streambuf : public std::basic_streambuf<CharT, Traits>
{
public:
	typedef CharT                           char_type;
	typedef Traits                          traits_type;
	typedef typename traits_type::int_type  int_type;
	typedef typename traits_type::pos_type  pos_type;
	typedef typename traits_type::off_type  off_type;

public:
    explicit cached_streambuf(std::basic_istream<CharT, Traits>& stream) 
        : m_stream(stream)
        , m_pos(0)
        , m_len(0)
    {
        std::memset(m_cache, 0, sizeof(int_type) * CacheSize);
    }

protected:
    int_type overflow(int_type c) override final
    {
        return traits_type::eof();
    }
    
protected:
    std::streamsize showmanyc() override final
    {
        return (m_len - m_pos);
    }

    int_type underflow() override final
    {
        if (m_pos == CacheSize) 
            return traits_type::eof();
        if (m_pos == m_len)
            m_cache[m_len++] = traits_type::to_int_type(m_stream.get());
        return m_cache[m_pos];
    }

    int_type uflow() override final
    {
        if (m_pos == CacheSize) 
            return traits_type::eof();
        if (m_pos == m_len)
            m_cache[m_len++] = traits_type::to_int_type(m_stream.get());
        return m_cache[m_pos++];
    }

    int_type pbackfail(int_type ch) override final
    {
        if (m_pos == 0 || !(ch == traits_type::eof() || ch == m_cache[m_pos - 1]))
            return traits_type::eof();
        return m_cache[--m_pos];
    }

private:
    std::basic_istream<CharT, Traits>& m_stream;
    int_type m_cache[CacheSize];
    std::streamsize m_pos;
    std::streamsize m_len;
};

// ----------------------------------------------------------------------------
//                           reading entry point
// ----------------------------------------------------------------------------
template <class CharT, class Traits, unsigned MaxLength = 0, bool NeedCache = false>
struct stream_wrapper
{
    explicit stream_wrapper(std::basic_istream<CharT, Traits>& _stream) 
        : stream(_stream) 
    {}

    std::basic_istream<CharT, Traits>& stream;
};

// ----------------------------------------------------------------------------
template <class CharT, class Traits, unsigned MaxLength>
struct stream_wrapper<CharT, Traits, MaxLength, true>
{
    explicit stream_wrapper(std::basic_istream<CharT, Traits>& _stream)
        : cache(_stream) 
        , stream(&cache) 
    {}

    cached_streambuf<CharT, Traits, MaxLength> cache;
    std::basic_istream<CharT, Traits> stream;
};

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class ...Args>
void read(std::basic_istream<CharT, Traits>& stream, size_t& pos, Args&& ...args)
{
    enum : unsigned { max_length = args_traits<Args...>::max_length };
    enum : bool     { need_cache = args_traits<Args...>::need_cache };

    stream_wrapper<CharT, Traits, max_length, need_cache> wrapper(stream);
    read_impl(wrapper.stream, pos, std::forward<Args>(args)...);
}

} // namespace details

// ----------------------------------------------------------------------------
//                                rfc1123
// ----------------------------------------------------------------------------
namespace format
{
    
// ----------------------------------------------------------------------------
struct rfc1123
{
    template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
    static dt_parts read(std::basic_istream<CharT, Traits>& stream, const rfc1123& fmt, std::basic_string<CharT, Traits, Alloc>* _zone, int* _offset)
    {
        using namespace details;
        using details::read;
        
        typedef std::remove_pointer<decltype(_offset)>::type dt_offset;
        typedef std::remove_pointer<decltype(_zone)>::type   dt_zone;

        dt_parts  parts{};
        dt_offset offset{};
        dt_zone   zone{};

        size_t pos = 0;
        skip_spaces(stream, pos);
        if (stream.eof())
            throw std::logic_error(std::string("unexpected eof at ") + std::to_string(pos));

        read(stream, pos, 
            optional(ra<3>(parts.weekday, weekday_names_short()), rc(','), rc(' ')),
            ru<1,2>(parts.day), rc(' '), ra<3>(parts.month, month_names_short()), rc(' '), ru<2,4>(parts.year), rc(' '), 
            ru<2,2>(parts.hour), rc(':'), ru<2,2>(parts.minute), optional(rc(':'), ru<2,2>(parts.second)), rc(' '));
        
        auto ic = stream.peek();
        auto c = static_cast<char>(Traits::to_char_type(ic));
        if (c == '+' || c == '-')
        {
            int offset_hour{};
            int offset_minute{};
            read(stream, pos, rs<2,2>(offset_hour), ru<2,2>(offset_minute));
            offset = (offset_hour * 60 + offset_minute) * 60;
        }
        else
        {
            size_t zone_index{};
            const auto zone_names = zone_names_rfc822();
            read(stream, pos, ra<3>(zone_index, zone_names));
            zone = std::move(dt_zone(*(zone_names.first + zone_index)));
        }

        if (_offset)
            *_offset = offset;
        if (_zone)
            *_zone = std::move(zone);

        return parts;
    }
};

} // namespace format

} // namespace date
