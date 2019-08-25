//
// This solution is based on answer from stackoverflow: https://stackoverflow.com/a/32223343
//
#pragma once
#include <type_traits>

// ---------------------------------------------------------------------------
namespace date {
namespace std_impl {

// ---------------------------------------------------------------------------
template <std::size_t... Ints>
struct index_sequence
{
    using type = index_sequence;
    using value_type = std::size_t;
    static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
};

// --------------------------------------------------------------
namespace details {

// --------------------------------------------------------------
template <class Sequence1, class Sequence2>
struct merge_and_renumber;

// --------------------------------------------------------------
template <std::size_t... I1, std::size_t... I2>
struct merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
    : index_sequence<I1..., (sizeof...(I1) + I2)...>
{};

} // namespace details

// --------------------------------------------------------------
template <size_t N>
struct make_index_sequence
    : details::merge_and_renumber<typename make_index_sequence<N / 2>::type, typename make_index_sequence<N - N / 2>::type>
{};

template<> struct make_index_sequence<0> : index_sequence<> { };
template<> struct make_index_sequence<1> : index_sequence<0> { };

} // namespace std_impl
} // namespace date
