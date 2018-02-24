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
#include <vector>
#include <array>
#include <cctype>
#include <istream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <type_traits>
#include <tuple>
#include <cassert>
#include "integer_sequence.h"

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wpedantic"
# if __GNUC__ < 5
//! GCC 4.9 Bug 61489 Wrong warning with -Wmissing-field-initializers.
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
# endif
#endif

// ----------------------------------------------------------------------------
namespace date
{

// ----------------------------------------------------------------------------
//                                details
// ----------------------------------------------------------------------------
namespace details
{
    
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4351)
#endif // defined(_MSC_VER)

// ----------------------------------------------------------------------------
//                               string
// ---------------------------------------------------------------------------
template <class DstT, size_t DstN, class SrcT, unsigned SrcN>
constexpr DstT character_cvt(const SrcT(&str)[SrcN], int index)
{
    return (index >= SrcN) ? static_cast<DstT>(0) : static_cast<DstT>(str[index]);
}

template<class DstT, size_t DstN, class SrcT, unsigned SrcN, std::size_t... Indexes>
constexpr std::array<DstT, DstN> make_literal_array_helper(const SrcT(&str)[SrcN], std_impl::index_sequence<Indexes...>)
{
    return {{ character_cvt<DstT, DstN>(str, Indexes)... }};
}

template<class DstT, size_t DstN, class SrcT, unsigned SrcN>
constexpr std::array<DstT, DstN> make_literal_array(const SrcT(&str)[SrcN])
{
    static_assert(DstN >= SrcN, "Source string is too long");
    return make_literal_array_helper<DstT, DstN>(str, std_impl::make_index_sequence<DstN>{});
}

// ----------------------------------------------------------------------------
template <class CharT, unsigned Length>
class string_literal
{
public:
    typedef CharT value_type;

public:
    template <unsigned Count>
    constexpr string_literal(const CharT(&value)[Count])
        : m_data(make_literal_array<CharT, Length>(value))
        , m_size(Count - 1)
    {
        static_assert(Length >= Count, "Passed string is too long");
    }

    template <unsigned Count, class U = CharT, class = typename std::enable_if<2 <= sizeof(U)>::type>
    constexpr string_literal(const char(&value)[Count])
        : m_data(make_literal_array<CharT, Length>(value))
        , m_size(Count - 1)
    {
        static_assert(Length >= Count, "Passed string is too long");
    }

    constexpr const value_type* c_str() const { return m_data.data(); }
    constexpr const value_type& operator[](size_t index) const { return m_data[index]; }
    constexpr size_t size() const  { return m_size; }
    constexpr bool empty() const { return (m_size == 0); }

private:
    const std::array<CharT, Length> m_data;
    const size_t m_size;
};

template <class CharT, class Traits, class Allocator, unsigned Length>
inline bool operator==(const string_literal<CharT, Length>& lhs, const std::basic_string<CharT, Traits, Allocator>& rhs)
{
    return (rhs.compare(lhs.c_str()) == 0);
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // defined(_MSC_VER)

// ----------------------------------------------------------------------------
//                               constants
// ----------------------------------------------------------------------------
template <class CharT>
constexpr std::array<string_literal<CharT, 4>, 7> weekday_names_short()
{
    typedef string_literal<CharT, 4> s;
    return { s("Sun"), s("Mon"), s("Tue"), s("Wed"), s("Thu"), s("Fri"), s("Sat") };
}

// ----------------------------------------------------------------------------
template <class CharT>
constexpr std::array<string_literal<CharT, 4>, 12> month_names_short()
{
    typedef string_literal<CharT, 4> s;
    return { s("Jan"), s("Feb"), s("Mar"), s("Apr"), s("May"), s("Jun"), s("Jul"), s("Aug"), s("Sep"), s("Oct"), s("Nov"), s("Dec") };
}

// ----------------------------------------------------------------------------
template <class CharT>
constexpr std::array<string_literal<CharT, 4>, 10> zone_names_rfc822()
{
    //! Not including military codes (each code encoded by one symbol from 'A' to 'Z').
    typedef string_literal<CharT, 4> s;
    return { s("GMT"), s("UT"), s("EST"), s("EDT"), s("CST"), s("CDT"), s("MST"), s("MDT"), s("PST"), s("PDT") };
}

// ----------------------------------------------------------------------------
//                       reading implementation
// ----------------------------------------------------------------------------
template <class T, class ...Args>
struct args_traits;

template <class T>
struct args_traits<T>
{
    enum : unsigned { min_length = T::min_length };
    enum : unsigned { max_length = T::max_length };
    enum : bool     { need_cache = T::need_cache };
};

template <class T, class ...Args>
struct args_traits
{
    enum : unsigned { min_length = T::min_length + args_traits<Args...>::min_length };
    enum : unsigned { max_length = T::max_length + args_traits<Args...>::max_length };
    enum : bool     { need_cache = T::need_cache | args_traits<Args...>::need_cache };
};

// ----------------------------------------------------------------------------
enum : bool { SignOptional = false, SignRequired = true };

template <bool SignRequired>
struct sign_traits;

template <>
struct sign_traits<SignOptional>
{
    enum : unsigned { min_length = 0 };
    enum : unsigned { max_length = 1 };
};

template <>
struct sign_traits<SignRequired>
{
    enum : unsigned { min_length = 1 };
    enum : unsigned { max_length = 1 };
};

// ----------------------------------------------------------------------------
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wenum-compare"
#endif  // __GNUC__

template <class T, class ...Args>
struct cases_traits;

template <class T1, class T2>
struct cases_traits<T1, T2>
{
    typedef typename std::conditional<(T1::min_length < T2::min_length), T1, T2>::type min_type;
    typedef typename std::conditional<(T1::max_length > T2::max_length), T1, T2>::type max_type;

    enum : unsigned { min_length = min_type::min_length };
    enum : unsigned { max_length = max_type::max_length };
};

template <class T, class ...Args>
struct cases_traits
{
    typedef typename cases_traits<Args...>::min_type _args_min_type;
    typedef typename cases_traits<Args...>::max_type _args_max_type;
    typedef typename std::conditional<(T::min_length < _args_min_type::min_length), T, _args_min_type>::type min_type;
    typedef typename std::conditional<(T::max_length > _args_max_type::max_length), T, _args_max_type>::type max_type;

    enum : unsigned { min_length = min_type::min_length };
    enum : unsigned { max_length = max_type::max_length };
};

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif  // __GNUC__

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
template <class SignedT, unsigned MinLength, unsigned MaxLength, bool SignMandatory>
struct rs_t
{
    typedef SignedT value_type;

    enum : bool     { sign_mandatory = SignMandatory };
    enum : unsigned { min_length = MinLength + sign_traits<SignMandatory>::min_length };
    enum : unsigned { max_length = MaxLength + sign_traits<SignMandatory>::max_length };
    enum : bool     { need_cache = false };

    value_type& value;
};

template <unsigned MinLength, unsigned MaxLength, bool SignMandatory, class SignedT>
static rs_t<SignedT, MinLength, MaxLength, SignMandatory> rs(SignedT& value)
{
    return rs_t<SignedT, MinLength, MaxLength, SignMandatory>{ value };
}

// ----------------------------------------------------------------------------
template <class IndexT, class StringT, unsigned Count, unsigned MinLength, unsigned MaxLength>
struct ra_t
{
    typedef IndexT                      index_type;
    typedef std::array<StringT, Count>  values_type;

    enum : unsigned { min_length = MinLength };
    enum : unsigned { max_length = MaxLength };
    enum : bool     { need_cache = false };
    
    ra_t(index_type& _index, const values_type& _values)
        : index(_index), values(_values) 
    {}

    index_type&         index;
    const values_type&  values;
};

template <unsigned MinLength, unsigned MaxLength, class IndexT, class StringT, unsigned Count>
static ra_t<IndexT, StringT, Count, MinLength, MaxLength> ra(IndexT& index, const std::array<StringT, Count>& values)
{
    return ra_t<IndexT, StringT, Count, MinLength, MaxLength>{ index, values };
}

// ----------------------------------------------------------------------------
template <class ...Args>
struct optional_t
{
    enum : unsigned { min_length = 0 };
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
template <class ...Args>
struct branch_t
{
    enum : unsigned { min_length = args_traits<Args...>::min_length };
    enum : unsigned { max_length = args_traits<Args...>::max_length };
    enum : bool     { need_cache = false };

    branch_t(Args&& ...args) 
        : elements(std::forward<Args>(args)...) {}

    std::tuple<Args...> elements;
};

template <class ...Args>
branch_t<Args...> branch(Args&& ...args)
{
    return branch_t<Args...>(std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class ...Args>
struct cases_t
{
    enum : unsigned { min_length = cases_traits<Args...>::min_length };
    enum : unsigned { max_length = cases_traits<Args...>::max_length };
    enum : bool     { need_cache = true };

    cases_t(Args&& ...args) 
        : cases(std::forward<Args>(args)...) {}

    std::tuple<Args...> cases;
};

template <class ...Args>
cases_t<Args...> cases(Args&& ...args)
{
    return cases_t<Args...>(std::forward<Args>(args)...);
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
SignedT read_signed(std::basic_istream<CharT, Traits>& stream, size_t& pos, unsigned min_len, unsigned max_len, bool sign_required)
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
    else if (sign_required)
    {
        throw std::logic_error(std::string("invalid signed format at ") + std::to_string(pos));
    }
        
    const auto x = read_unsigned<SignedT>(stream, pos, min_len, max_len);
    return (positive) ? x : -x;
}

// ----------------------------------------------------------------------------
template <class IndexT, class StringT, unsigned Count, class CharT, class Traits = std::char_traits<CharT>>
IndexT read_abbr(std::basic_istream<CharT, Traits>& stream, size_t& pos, const std::array<StringT, Count>& values, unsigned max_len)
{
    std::array<const CharT*, Count> str_list;
    std::array<bool, Count> is_match;

    for (size_t index = 0; index < Count; ++index)
    {
        str_list[index] = values[index].c_str();
        is_match[index] = (str_list[index] != CharT{'\0'});
    }

    for (size_t i = 0; i < max_len; ++i)
    {
        const auto ic = stream.peek();
        if (Traits::eq_int_type(ic, Traits::eof()))
            throw std::logic_error(std::string("unexpected eof at ") + std::to_string(pos));

        (void)stream.get(); ++pos;
        const auto ch = Traits::to_char_type(ic);

        for (size_t index = 0; index < Count; ++index)
        {
            if (!is_match[index])
                continue;

            const CharT* str = str_list[index];
            if (*str != ch)
            {
                is_match[index] = false;
                continue;
            }

            ++str;
            if (*str == CharT{'\0'})
                return static_cast<IndexT>(index);
            
            str_list[index] = str;
        }
    }

    throw std::logic_error(std::string("unknown abbr format at ") + std::to_string(pos));
}

// ----------------------------------------------------------------------------
template<unsigned N>
struct opts_reader_t
{
    template <class CharT, class Traits, typename... TupleT>
    static void read(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& opts)
    {
        opts_reader_t<N - 1>::read(stream, pos, opts);
        read_impl(stream, pos, std::get<N - 1>(opts));
    }
};

template<>
struct opts_reader_t<0>
{
    template <class CharT, class Traits, typename... TupleT>
    static void read(std::basic_istream<CharT, Traits>&, size_t&, std::tuple<TupleT...>&)
    {}
};

template <class CharT, class Traits, typename... TupleT>
void read_opts(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& opts)
{
    const size_t begin = pos;
    try
    {
        typedef opts_reader_t<sizeof...(TupleT)> opts_reader;
        opts_reader::read(stream, pos, opts);
    }
    catch (const std::logic_error&)
    {
        for (; pos > begin; --pos)
            stream.unget();
    }
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, typename... TupleT>
void read_branch(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& elements)
{
    typedef opts_reader_t<sizeof...(TupleT)> branch_reader;
    branch_reader::read(stream, pos, elements);
}

// ----------------------------------------------------------------------------
template<unsigned Count, unsigned N>
struct cases_reader_t
{
    template <class CharT, class Traits, typename... TupleT>
    static void read(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& cases)
    {
        const std::size_t begin = pos;
        try
        {
            read_impl(stream, pos, std::get<Count - N>(cases));
        }
        catch (const std::logic_error&)
        {
            for (; pos > begin; --pos)
                stream.unget();

            cases_reader_t<Count, N - 1>::read(stream, pos, cases);
        }
    }
};

template<unsigned Count>
struct cases_reader_t<Count, 0>
{
    template <class CharT, class Traits, typename... TupleT>
    static void read(std::basic_istream<CharT, Traits>&, size_t& pos, std::tuple<TupleT...>&)
    {
        throw std::logic_error(std::string("no matching cases at ") + std::to_string(pos));
    }
};

template <class CharT, class Traits, typename... TupleT>
void read_cases(std::basic_istream<CharT, Traits>& stream, size_t& pos, std::tuple<TupleT...>& cases)
{
    typedef cases_reader_t<sizeof...(TupleT), sizeof...(TupleT)> cases_reader;
    cases_reader::read(stream, pos, cases);
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
template <class CharT, class Traits, class SignedT, unsigned MinLength, unsigned MaxLength, bool SignRequired, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, rs_t<SignedT, MinLength, MaxLength, SignRequired> a0, Args&& ...args)
{
    a0.value = read_signed<SignedT>(stream, pos, MinLength, MaxLength, SignRequired);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class IndexT, class StringT, unsigned Count, unsigned MinLength, unsigned MaxLength, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, ra_t<IndexT, StringT, Count, MinLength, MaxLength> a0, Args&& ...args)
{
    a0.index = read_abbr<IndexT>(stream, pos, a0.values, MaxLength);
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
template <class CharT, class Traits, class ...BranchArgs>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, branch_t<BranchArgs...> a0)
{
    read_branch(stream, pos, a0.elements);
}

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class ...CasesArgs, class ...Args>
void read_impl(std::basic_istream<CharT, Traits>& stream, size_t& pos, cases_t<CasesArgs...> a0, Args&& ...args)
{
    read_cases(stream, pos, a0.cases);
    read_impl(stream, pos, std::forward<Args>(args)...);
}

// ----------------------------------------------------------------------------
//                           custom stream buffers
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
    {}

    cached_streambuf(const cached_streambuf&) = delete;
    cached_streambuf& operator=(const cached_streambuf&) = delete;

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
struct stream_wrapper_t
{
    explicit stream_wrapper_t(std::basic_istream<CharT, Traits>& base_stream) 
        : stream(base_stream) 
    {}

    stream_wrapper_t(const stream_wrapper_t&) = delete;
    stream_wrapper_t& operator=(const stream_wrapper_t&) = delete;

    std::basic_istream<CharT, Traits>& stream;
};

// ----------------------------------------------------------------------------
template <class CharT, class Traits, unsigned MaxLength>
struct stream_wrapper_t<CharT, Traits, MaxLength, true>
{
    explicit stream_wrapper_t(std::basic_istream<CharT, Traits>& base_stream)
        : cache(base_stream) 
        , stream(&cache) 
    {}

    stream_wrapper_t(const stream_wrapper_t&) = delete;
    stream_wrapper_t& operator=(const stream_wrapper_t&) = delete;

    cached_streambuf<CharT, Traits, MaxLength> cache;
    std::basic_istream<CharT, Traits> stream;
};

// ----------------------------------------------------------------------------
template <class CharT, class Traits, class ...Args>
void read(std::basic_istream<CharT, Traits>& stream, size_t& pos, Args&& ...args)
{
    enum : bool     { need_cache = args_traits<Args...>::need_cache };
    enum : unsigned { max_length = args_traits<Args...>::max_length };

    stream_wrapper_t<CharT, Traits, max_length, need_cache> wrapper(stream);
    read_impl(wrapper.stream, pos, std::forward<Args>(args)...);
}

} // namespace details

// ----------------------------------------------------------------------------
template <class T>
struct array_size;

template <class T, unsigned N>
struct array_size<std::array<T, N>>
{
    enum { value = N };
};

// ----------------------------------------------------------------------------
//                                common
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
struct dt_traits
{
    template <class CharT, class Traits, class Alloc = std::allocator<CharT>>
    static std::time_t join(const dt_parts& parts, std::basic_string<CharT, Traits, Alloc>* zone, int32_t* offset);
};

// ----------------------------------------------------------------------------
template <class Format, class Date>
struct format_rfc
{
    typedef Format format_type;
    typedef Date   date_type;

    format_rfc(date_type& value, const format_type& format = format_type()) 
        : m_value(value), m_format(format) {}
    
    template <typename CharT, typename Traits, class Alloc = std::allocator<CharT>>
    std::basic_istream<CharT, Traits>& from_stream(std::basic_istream<CharT, Traits>& stream)
    {
        typedef Format dt_format;
        typedef std::basic_string<CharT, Traits, Alloc> dt_zone;

        dt_parts parts{};
        dt_zone  zone{};
        int      offset{};

        parts = dt_format::read(stream, m_format, &zone, &offset);
        m_value = dt_traits<Date>::join(parts, &zone, &offset);
    
        return stream;
    }

    template <class T, class U, typename CharT, typename Traits, class Alloc = std::allocator<CharT>>
    friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& stream, format_rfc<T, U>& formatter);

private:
    date_type& m_value;
    format_type m_format;
};

// ----------------------------------------------------------------------------
template <class Format, class Date, typename CharT, typename Traits, class Alloc = std::allocator<CharT>>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& stream, format_rfc<Format, Date>& formatter)
{
    return formatter.from_stream(stream);
}

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
