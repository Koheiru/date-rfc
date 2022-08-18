//
// The MIT License (MIT)
//
// Copyright (c) 2019 Yury Prostov
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
#include <type_traits>
#include <iterator>
#include "iterator_traits.h"

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
template <class T, class = void>
struct input_traits
{
    enum : bool { need_cache = false };
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct input_traits<std::istreambuf_iterator<Args...>, void>
{
    enum : bool { need_cache = true };
};

// ----------------------------------------------------------------------------
template <class ...Args>
struct input_traits<std::istream_iterator<Args...>, void>
{
    enum : bool { need_cache = true };
};

// ----------------------------------------------------------------------------
//                         input wrapper: raw access
// ----------------------------------------------------------------------------
template <class Iterator, std::size_t MaxLength = 0, bool NeedCache = false>
class input_wrapper_t
{
public:
    input_wrapper_t(Iterator& pos, const Iterator& end) : m_pos(pos), m_end(end) {}
    ~input_wrapper_t() = default;

    Iterator& begin() 
    {
        return m_pos;
    }

    const Iterator& end() const
    {
        return m_end;
    }

private:
    Iterator& m_pos;
    const Iterator& m_end;
};

// ----------------------------------------------------------------------------
//                       input wrapper: cached access
// ----------------------------------------------------------------------------
template <class Iterator>
class iterator_proxy : public std::iterator<std::forward_iterator_tag, typename iterator_traits<Iterator>::value_type>
{
public:
    using parent_type = std::iterator<std::forward_iterator_tag, typename iterator_traits<Iterator>::value_type>;
    using value_type = typename parent_type::value_type;
    using difference_type = typename parent_type::difference_type;
    using pointer = typename parent_type::pointer;
    using reference = typename parent_type::reference;
    using iterator_category = typename parent_type::iterator_category;

public:
    iterator_proxy(Iterator& srcIt, const Iterator& srcEnd, value_type* cache, std::size_t& cacheSize)
        : m_srcIt(srcIt)
        , m_srcEnd(srcEnd)
        , m_cache(cache)
        , m_cacheSize(cacheSize)
        , m_position((srcIt == srcEnd) ? static_cast<std::size_t>(-1) : 0)
    {
        if (m_position != static_cast<std::size_t>(-1))
            m_cache[m_cacheSize++] = *(srcIt++);
    }

    iterator_proxy(const iterator_proxy<Iterator>& other)
        : m_srcIt(other.m_srcIt)
        , m_srcEnd(other.m_srcEnd)
        , m_cache(other.m_cache)
        , m_cacheSize(other.m_cacheSize)
        , m_position(other.m_position)
    {}

    iterator_proxy(iterator_proxy<Iterator>&& other)
        : m_srcIt(std::move(other.m_srcIt))
        , m_srcEnd(std::move(other.m_srcEnd))
        , m_cache(other.m_cache)
        , m_cacheSize(other.m_cacheSize)
        , m_position(other.m_position)
    {}

    iterator_proxy& operator=(const iterator_proxy<Iterator>& other)
    {
        m_srcIt = other.m_srcIt;
        m_srcEnd = other.m_srcEnd;
        m_cache = other.m_cache;
        m_cacheSize = other.m_cacheSize;
        m_position = other.m_position;
        return *this;
    }

    iterator_proxy& operator=(iterator_proxy<Iterator>&& other)
    {
        m_srcIt = std::move(other.m_srcIt);
        const_cast<Iterator&>(m_srcEnd) = std::move(other.m_srcEnd);
        m_cache = other.m_cache;
        m_cacheSize = other.m_cacheSize;
        m_position = other.m_position;
        return *this;
    }

    iterator_proxy& operator++()
    {
        if ((m_position == static_cast<std::size_t>(-1)) || (++m_position < m_cacheSize))
            return *this;

        if (m_srcIt == m_srcEnd)
        {
            m_position = static_cast<std::size_t>(-1);
            return *this;
        }

        m_cache[m_cacheSize++] = *(m_srcIt++);
        return *this;
    }

    bool operator==(const iterator_proxy& rhs) const 
    {
        return (rhs.m_position == m_position);
    }

    bool operator!=(const iterator_proxy& rhs) const
    {
        return (rhs.m_position != m_position);
    }
    
    value_type operator*() const 
    {
        return m_cache[m_position];
    }

private:
    Iterator& m_srcIt;
    const Iterator& m_srcEnd;
    value_type* m_cache;
    std::size_t& m_cacheSize;
    std::size_t m_position;
};

// ----------------------------------------------------------------------------
template <class Iterator, std::size_t MaxLength>
class input_wrapper_t<Iterator, MaxLength, true>
{
public:
    using char_type = typename iterator_traits<Iterator>::value_type;
    enum : std::size_t { cache_capacity = MaxLength };

public:
    input_wrapper_t(Iterator& pos, const Iterator& end)
        : m_cacheSize(0)
        , m_cache{}
        , m_proxyIt{ pos, end, &m_cache[0], m_cacheSize }
        , m_proxyEnd{ const_cast<Iterator&>(end), end, &m_cache[0], m_cacheSize }
    {}

    iterator_proxy<Iterator>& begin()
    {
        return m_proxyIt;
    }

    const iterator_proxy<Iterator>& end() const
    {
        return m_proxyEnd;
    }

private:
    std::size_t m_cacheSize;
    char_type m_cache[cache_capacity];
    iterator_proxy<Iterator> m_proxyIt;
    const iterator_proxy<Iterator> m_proxyEnd;
};

} // namespace date

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
