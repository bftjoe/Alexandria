#pragma once

#include <memory>

namespace fixed_containers::memory
{
// Similar to https://en.cppreference.com/w/cpp/memory/construct_at
// but uses references and correctly handles types that overload operator&
template <typename T, typename ... Args>
constexpr auto construct_at_address_of(T& p, Args&&... args)
{
    return std::construct_at(std::addressof(p), std::forward<Args>(args)...);
}

// Similar to https://en.cppreference.com/w/cpp/memory/destroy_at
// but uses references and correctly handles types that overload operator&
template <typename T>
constexpr void destroy_at_address_of(T& p)
{
    std::destroy_at(std::addressof(p));
}

}  // namespace fixed_containers::memory

#include <memory>

namespace fixed_containers::algorithm
{
// Similar to https://en.cppreference.com/w/cpp/memory/uninitialized_move
// but also destroys the source range
template <class FwdIt1, class FwdIt2>
constexpr FwdIt2 uninitialized_relocate(FwdIt1 first, FwdIt1 last, FwdIt2 d_first)
{
    while (first != last)
    {
        memory::construct_at_address_of(*d_first, std::move(*first));
        memory::destroy_at_address_of(*first);
        ++d_first;
        ++first;
    }
    return d_first;
}

// Similar to https://en.cppreference.com/w/cpp/memory/uninitialized_move
// but also destroys the source range
template <class BidirIt1, class BidirIt2>
constexpr BidirIt2 uninitialized_relocate_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last)
{
    while (first != last)
    {
        --d_last;
        --last;
        memory::construct_at_address_of(*d_last, std::move(*last));
        memory::destroy_at_address_of(*last);
    }
    return d_last;
}
}  // namespace fixed_containers::algorithm

#include <concepts>
#include <iterator>
#include <type_traits>

// NOTE: A concept X is accompanied by NotX and should be used instead of (!X) for proper
// subsumption. De Morgan's law also does not apply for subsumption.
// More info: https://en.cppreference.com/w/cpp/language/constraints
namespace fixed_containers
{
template <typename...>
concept AlwaysFalseV = false;

// Enforces a specific type, preventing all implicit conversions
// Example usage:
// double cos(Strict<double> auto rad) {/*...*/}
template <class ValueType, typename StrictType>
concept Strict = std::same_as<StrictType, ValueType>;

template <class T>
concept Trivial = std::is_trivial_v<T>;
template <class T>
concept NotTrivial = not Trivial<T>;

template <class T>
concept StandardLayout = std::is_standard_layout_v<T>;
template <class T>
concept NotStandardLayout = not StandardLayout<T>;

template <class T>
concept DefaultConstructible = std::is_default_constructible_v<T>;
template <class T>
concept NotDefaultConstructible = not DefaultConstructible<T>;

template <class T>
concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;
template <class T>
concept NotTriviallyDefaultConstructible = not TriviallyDefaultConstructible<T>;

template <class T>
concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;
template <class T>
concept NotTriviallyCopyConstructible = not TriviallyCopyConstructible<T>;

template <class T>
concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;
template <class T>
concept NotTriviallyMoveConstructible = not TriviallyMoveConstructible<T>;

template <class T>
concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;
template <class T>
concept NotTriviallyCopyAssignable = not TriviallyCopyAssignable<T>;

template <class T>
concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;
template <class T>
concept NotTriviallyMoveAssignable = not TriviallyMoveAssignable<T>;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
template <class T>
concept NotTriviallyCopyable = not TriviallyCopyable<T>;

template <class T>
concept CopyConstructible = std::is_copy_constructible_v<T>;
template <class T>
concept NotCopyConstructible = not CopyConstructible<T>;

template <class T>
concept MoveConstructible = std::is_move_constructible_v<T>;
template <class T>
concept NotMoveConstructible = not MoveConstructible<T>;

template <class T>
concept CopyAssignable = std::is_copy_assignable_v<T>;
template <class T>
concept NotCopyAssignable = not CopyAssignable<T>;

template <class T>
concept MoveAssignable = std::is_move_assignable_v<T>;
template <class T>
concept NotMoveAssignable = not MoveAssignable<T>;

// As of C++20, even if all copy/move ctors/assignment ops are deleted,
// the type still counts as trivially copyable. Example: std::atomic<int>
// https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable
template <class T>
concept TriviallyCopyableWithAtLeastOneNonDeleted =
    TriviallyCopyable<T> &&
    (CopyConstructible<T> || MoveConstructible<T> || CopyAssignable<T> || MoveAssignable<T>);
template <class T>
concept NotTriviallyCopyableWithAtLeastOneNonDeleted =
    NotTriviallyCopyable<T> || (NotCopyConstructible<T> && NotMoveConstructible<T> &&
                                NotCopyAssignable<T> && NotMoveAssignable<T>);

template <class T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;
template <class T>
concept NotTriviallyDestructible = not TriviallyDestructible<T>;

template <class T>
concept Aggregate = std::is_aggregate_v<T>;
template <class T>
concept NotAggregate = not Aggregate<T>;

template <class T>
concept IsReference = std::is_reference_v<T>;
template <class T>
concept IsNotReference = not IsReference<T>;

template <typename T>
concept ConstexprDefaultConstructible = requires(T t) {
    {
        std::bool_constant<(T{}, true)>()
    } -> std::same_as<std::true_type>;
};
template <class T>
concept NotConstexprDefaultConstructible = not ConstexprDefaultConstructible<T>;

template <typename T, auto... CONSTRUCTOR_ARGS>
concept IsStructuralType = requires() { std::integral_constant<T, T{CONSTRUCTOR_ARGS...}>(); };
template <typename T>
concept IsNotStructuralType = not IsStructuralType<T>;

// NOTE: this doesn't exactly match https://en.cppreference.com/w/cpp/iterator/input_iterator
template <class Iterator>
concept InputIterator =
    std::is_convertible_v<typename std::iterator_traits<Iterator>::iterator_category,
                          std::input_iterator_tag>;

template <typename T>
concept IsStdPair = std::same_as<T, std::pair<typename T::first_type, typename T::second_type>>;

// The member type `is_transparent` is a convention that indicates to the user that this function
// object is a transparent function object: it accepts arguments of arbitrary types and uses perfect
// forwarding, which avoids unnecessary copying and conversion when the function object is used in
// heterogeneous context, or with rvalue arguments.
//
// An example usage is transparent comparators:
// ```
// std::map<std::string, int, std::less<void>> my_map{};
// ```
// (https://en.cppreference.com/w/cpp/utility/functional/less_void)
// When using transparent comparators, lookups on this map can happen with
// `std::string_view` or `const char*` without having to convert them to `std::string`.
template <class T>
concept IsTransparent = requires() { typename T::is_transparent; };

template <class T>
concept HasValueType = requires() { typename T::value_type; };

template <auto... /*VALUES_TO_BE_PRINTED*/>
struct CompileTimeValuePrinter
{
};

// a "void-like" type, but without the hassle
// e.g. EmptyValue& is a valid type
struct EmptyValue
{
    static constexpr bool THIS_IS_EMPTY = true;
    constexpr EmptyValue() = delete;
};

template <class T>
concept IsEmpty = T::THIS_IS_EMPTY || std::is_void_v<T>;

template <class T>
concept IsNotEmpty = (!IsEmpty<T>);

}  // namespace fixed_containers

namespace fixed_containers::consteval_compare
{
// These will show the values if it fails to compile
template <auto LHS, auto RHS>
inline constexpr bool equal = LHS == RHS;
template <auto LHS, auto RHS>
inline constexpr bool not_equal = LHS != RHS;

template <auto LHS, auto RHS>
inline constexpr bool less = LHS < RHS;
template <auto LHS, auto RHS>
inline constexpr bool less_or_equal = LHS <= RHS;

template <auto LHS, auto RHS>
inline constexpr bool greater = LHS > RHS;
template <auto LHS, auto RHS>
inline constexpr bool greater_or_equal = LHS >= RHS;
}  // namespace fixed_containers::consteval_compare

#include <iterator>
#include <type_traits>

namespace fixed_containers
{

// Names are not just "MUTABLE" and "CONSTANT" to avoid collision with macros
enum class IteratorConstness : bool
{
    MUTABLE_ITERATOR = false,
    CONSTANT_ITERATOR = true,
};

enum class IteratorDirection : bool
{
    FORWARD = false,
    REVERSE = true,
};

// Using std::reverse_iterator fails to compile with the error message below.
// Only applies to maps, because they leverage operator->
//
/**
/workspace/fixed_containers/test/enum_map_test.cpp:807:19: error: static_assert expression is not an
integral constant expression static_assert(s1.crbegin()->first() == TestEnum1::FOUR);
                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/workspace/fixed_containers/test/enum_map_test.cpp:807:33: note: member call on variable whose
lifetime has ended static_assert(s1.crbegin()->first() == TestEnum1::FOUR);
                                ^
/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/stl_iterator.h:368:20: note:
declared here _S_to_pointer(_Tp __t)
                          ^
/workspace/fixed_containers/test/enum_map_test.cpp:808:19: error: static_assert expression is not an
integral constant expression static_assert(s1.crbegin()->second() == 10);
                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
/workspace/fixed_containers/test/enum_map_test.cpp:808:33: note: member call on variable whose
lifetime has ended static_assert(s1.crbegin()->second() == 10);
                                ^
/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/stl_iterator.h:368:20: note:
declared here _S_to_pointer(_Tp __t)
                          ^
2 errors generated.
 */
// clang-format on
//
// Source code for `std::reverse_iterator`:
// https://github.com/llvm/llvm-project/blob/llvmorg-13.0.0-rc1/libcxx/include/__iterator/reverse_iterator.h#L127-L130
// gcc libstdc++:
// https://github.com/gcc-mirror/gcc/blob/releases/gcc-11.2.0/libstdc++-v3/include/bits/stl_iterator.h#L252-L264
//
// The problem is that both implementations define a local variable of the base iterator type and
// then call `operator*()` or `operator->()` on that. This is a problem with Fixed Container
// iterators, that return a pointer to a data member of a local variable, which is gone by the time
// any value_type member function is called on it.
//
// Note that the `ArrowProxy` strategy would probably have issues with `std::reverse_iterator` as
// well. The llvm implementation would call std::addressof() on a temporary. The gcc implementation
// could work if pointer was defined to be `ArrowProxy`.
//
// Therefore, Fixed Container iterators provide native support for reverse iterators.
// Native support is a bit faster than std::reverse_iterator as the latter does
// a copy + decrement on every dereference, whereas a non-wrapper doesn't need to do that.
// clang-format off

}  // namespace fixed_containers

#include <concepts>
#include <cstddef>

namespace fixed_containers
{
// Useful in generic code as std and other containers do not have `static_max_size()`
// For std containers, this would be specialized to yield
// `std::numeric_limits<difference_type>::max()`.
template <typename T>
struct max_size;

template <typename T>
inline constexpr std::size_t max_size_v = max_size<T>::value;

template <typename T>
concept has_static_sizet_static_max_size_void = requires() {
    {
        T::static_max_size()
    } -> std::same_as<std::size_t>;
};

template <has_static_sizet_static_max_size_void T>
struct max_size<T> : std::integral_constant<std::size_t, T::static_max_size()>
{
};

}  // namespace fixed_containers

#include <functional>
#include <type_traits>

namespace fixed_containers::reference_storage_detail
{
template <IsReference T>
struct ReferenceStorage
{
    using T0 = std::remove_reference_t<T>;
    std::reference_wrapper<T0> value;

    explicit constexpr ReferenceStorage(T v)
      : value{v}
    {
    }

    constexpr const T& get() const { return value.get(); }
    constexpr T& get() { return value.get(); }
};
}  // namespace fixed_containers::value_storage_detail

#include <type_traits>
#include <utility>

namespace fixed_containers::optional_storage_detail
{
struct OptionalStorageDummyT
{
};

template <class T>
union OptionalStorage
{
    OptionalStorageDummyT dummy_generic;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_generic{} { }
    explicit constexpr OptionalStorage(const T& v) : value{v} { }
    explicit constexpr OptionalStorage(T&& v) : value{std::move(v)} { }
    template <class... Args>
    explicit constexpr OptionalStorage(std::in_place_t, Args&&... args) : value(std::forward<Args>(args)...) { }

    constexpr OptionalStorage(const OptionalStorage&) requires TriviallyCopyConstructible<T> = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept requires TriviallyMoveConstructible<T> = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) requires TriviallyCopyAssignable<T> = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept requires TriviallyMoveAssignable<T> = default;
    // clang-format on

    constexpr OptionalStorage(const OptionalStorage& other)
      : value{other.value}
    {
    }
    constexpr OptionalStorage(OptionalStorage&& other) noexcept
      : value{std::move(other.value)}
    {
    }

    // CAUTION: we can't assign as we don't know whether value is the active union member.
    // Users are responsible for knowing that and calling the destructor appropriately.
    constexpr OptionalStorage& operator=(const OptionalStorage&) = delete;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = delete;
    // CAUTION: Users must manually call the destructor of T as they are the ones that keep
    // track of which member is active.
    constexpr ~OptionalStorage() noexcept {}

    constexpr const T& get() const { return value; }
    constexpr T& get() { return value; }
};

// OptionalStorage<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should OptionalStorage<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// WORKAROUND due to destructors: manually do the split with template specialization.
// NOTE: we branch on TriviallyCopyable instead of TriviallyDestructible because it needs all
// special functions to be trivial. The NonTriviallyCopyable flavor handles triviality separately
// for each special function (except the destructor).

template <TriviallyCopyable T>
union OptionalStorage<T>
{
    OptionalStorageDummyT dummy_trivially_copyable;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_trivially_copyable{} { }
    explicit constexpr OptionalStorage(const T& v) : value{v} { }
    explicit constexpr OptionalStorage(T&& v) : value{std::move(v)} { }
    template <class... Args>
    explicit constexpr OptionalStorage(std::in_place_t, Args&&... args) : value(std::forward<Args>(args)...) { }

    // clang-format on
    constexpr OptionalStorage(const OptionalStorage&) = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = default;
    constexpr ~OptionalStorage() noexcept = default;

    constexpr const T& get() const { return value; }
    constexpr T& get() { return value; }
};

template <IsReference T>
union OptionalStorage<T>
{
    OptionalStorageDummyT dummy_ref;
    reference_storage_detail::ReferenceStorage<T> value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_ref{} { }
    constexpr OptionalStorage(T v) : value{v} { }
    constexpr OptionalStorage(std::in_place_t, T v) : value(v) { }

    // clang-format on
    constexpr OptionalStorage(const OptionalStorage&) = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = default;
    constexpr ~OptionalStorage() noexcept = default;

    constexpr const T& get() const { return value.get(); }
    constexpr T& get() { return value.get(); }
};

template <typename T>
constexpr const auto& get(const OptionalStorage<T>& value)
{
    return value.get();
}

template <typename T>
constexpr auto& get(OptionalStorage<T>& value)
{
    return value.get();
}

template <typename T>
constexpr T&& get(T&& value)
{
    return value;
}

// "Transparent" here means there will be no wrapping for simple types.
template <typename T>
using OptionalStorageTransparent =
    std::conditional_t<Trivial<T> && StandardLayout<T> &&
                           /*for deleted*/ TriviallyDefaultConstructible<T>,
                       T,
                       OptionalStorage<T>>;

}  // namespace fixed_containers::optional_storage_detail

namespace fixed_containers::preconditions
{
constexpr bool test(const bool condition)
{
    if (!condition) [[unlikely]]
    {
        return true;
    }

    return false;
}
}  // namespace fixed_containers::preconditions

#include <compare>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
template <class ConstIterator,
          class MutableIterator,
          class ConstReferenceUnaryFunction,
          class MutableReferenceUnaryFunction,
          IteratorConstness CONSTNESS>
class RandomAccessIteratorTransformer
{
    static constexpr IteratorConstness NEGATED_CONSTNESS = IteratorConstness(!bool(CONSTNESS));

    using Self = RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 CONSTNESS>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = RandomAccessIteratorTransformer<ConstIterator,
                                                    MutableIterator,
                                                    ConstReferenceUnaryFunction,
                                                    MutableReferenceUnaryFunction,
                                                    NEGATED_CONSTNESS>;

    // Give Sibling access to private members
    friend class RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 NEGATED_CONSTNESS>;

    using IteratorType = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
                                            ConstIterator,
                                            MutableIterator>;
    using UnaryFunction = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
                                             ConstReferenceUnaryFunction,
                                             MutableReferenceUnaryFunction>;

public:
    static_assert(std::random_access_iterator<ConstIterator>);
    static_assert(std::random_access_iterator<MutableIterator>);

    using reference = decltype(std::declval<UnaryFunction>()(*std::declval<IteratorType>()));
    using value_type = std::remove_cvref_t<reference>;
    using pointer = std::add_pointer_t<reference>;
    using difference_type = typename std::iterator_traits<IteratorType>::difference_type;
    using iterator_category = std::conditional_t<std::contiguous_iterator<IteratorType>,
                                                 std::contiguous_iterator_tag,
                                                 std::random_access_iterator_tag>;
    using element_type = std::conditional_t<std::contiguous_iterator<IteratorType>,
                                            std::remove_reference_t<reference>,
                                            void>;

private:
    IteratorType iterator_;
    UnaryFunction unary_function_;

public:
    constexpr RandomAccessIteratorTransformer()
      : iterator_()
      , unary_function_()
    {
    }

    constexpr RandomAccessIteratorTransformer(IteratorType it, UnaryFunction unary_function)
      : iterator_(it)
      , unary_function_(unary_function)
    {
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr RandomAccessIteratorTransformer(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONSTANT_ITERATOR)
      : iterator_(other.iterator_)
      , unary_function_(other.unary_function_)
    {
    }

    constexpr reference operator*() const noexcept { return unary_function_(*iterator_); }

    constexpr pointer operator->() const noexcept
    {
        return std::addressof(unary_function_(*iterator_));
    }

    constexpr reference operator[](difference_type off) const
    {
        return unary_function_(*std::next(iterator_, off));
    }

    constexpr Self& operator++() noexcept
    {
        std::advance(iterator_, 1);
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        std::advance(iterator_, 1);
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        std::advance(iterator_, -1);
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        std::advance(iterator_, -1);
        return tmp;
    }

    constexpr Self& operator+=(difference_type off)
    {
        std::advance(iterator_, off);
        return *this;
    }

    constexpr Self operator+(difference_type off) const
    {
        return Self(std::next(iterator_, off), unary_function_);
    }

    friend constexpr Self operator+(difference_type off, const Self& other)
    {
        return Self(std::next(other.iterator_, off), other.unary_function_);
    }

    constexpr Self& operator-=(difference_type off)
    {
        std::advance(iterator_, -off);
        return *this;
    }

    constexpr Self operator-(difference_type off) const
    {
        return Self(std::prev(iterator_, off), unary_function_);
    }

    constexpr difference_type operator-(const Self& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr difference_type operator-(const Sibling& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr std::strong_ordering operator<=>(const Self& other) const
    {
        return this->iterator_ <=> other.iterator_;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return this->iterator_ == other.iterator_;
    }
};

}  // namespace fixed_containers

#include <version>

// Just __has_include was true for {clang with gcc's stdlib} but the actual type did not exist.
#if __has_include(<source_location>) and defined(__cpp_lib_source_location)

#include <source_location>

namespace fixed_containers::std_transition
{
using source_location = std::source_location;
}

#elif __has_include(<experimental/source_location>)

#include <experimental/source_location>

namespace fixed_containers::std_transition
{
using source_location = std::experimental::source_location;
}

// for {clang with clang's libcxx}
// https://reviews.llvm.org/D120634
// https://reviews.llvm.org/D120159
#else

namespace fixed_containers::std_transition
{
class source_location
{
public:
    static constexpr source_location current(
        std::uint_least32_t line = __builtin_LINE(),
        std::uint_least32_t column = __builtin_COLUMN(),
        const char* file_name = __builtin_FILE(),
        const char* function_name = __builtin_FUNCTION()) noexcept
    {
        return source_location{line, column, file_name, function_name};
    }

private:
    std::uint_least32_t line_;
    std::uint_least32_t column_;
    const char* file_name_;
    const char* function_name_;

private:
    constexpr source_location()
      : line_(0)
      , column_(0)
      , file_name_(nullptr)
      , function_name_(nullptr)
    {
    }

private:
    constexpr source_location(std::uint_least32_t line,
                              std::uint_least32_t column,
                              const char* file_name,
                              const char* function_name)
      : line_(line)
      , column_(column)
      , file_name_(file_name)
      , function_name_(function_name)
    {
    }

public:
    [[nodiscard]] constexpr std::uint_least32_t line() const noexcept { return line_; }
    [[nodiscard]] constexpr std::uint_least32_t column() const noexcept { return column_; }
    [[nodiscard]] constexpr const char* file_name() const noexcept { return file_name_; }
    [[nodiscard]] constexpr const char* function_name() const noexcept { return function_name_; }
};
}  // namespace fixed_containers::std_transition

#endif

#include <cstdlib>

namespace fixed_containers
{
// In contrast to assert(), works in Release mode
constexpr void assert_or_abort(bool b)
{
    if (!b)
    {
        std::abort();
    }
}
}  // namespace fixed_containers

#include <cstddef>
#include <string_view>

namespace fixed_containers
{
/**
 * Fully constexpr implementation of a compile-time literal null-terminated string.
 * StringLiteral is trivially_copyable and standard_layout. Furthermore, all functions are
 * constexpr.
 *
 * Compare:
 * <ul>
 * <li> static constexpr const char* s = "blah"; // strlen==4, sizeof==8
 * <li> static constexpr const char s[5] = "blah";  // strlen==4, sizeof==5 (null terminator)
 * <li> static constexpr StringLiteral s = "blah";  // constexpr size()==4
 * </ul>
 *
 * StringLiteral is cleaner to use, no confusion about size (null terminator), constant time
 * size(), safe to use as a return type, size not hardcoded in the type (which would make it
 * hard to change the string) and is implicitly convertible to std::string_view and c_str for
 * convenient use in existing APIs.
 */
class StringLiteral
{
public:
    template <std::size_t N>
    explicit(false) consteval StringLiteral(const char (&str)[N]) noexcept
      : size_(N - 1)
      , cstr_(str)
    {
        assert_or_abort(*std::next(cstr_, N - 1) == '\0');
    }

    constexpr StringLiteral() noexcept
      : size_(0)
      , cstr_("")
    {
    }

    [[nodiscard]] constexpr std::size_t size() const { return size_; }

    [[nodiscard]] constexpr const char* c_str() const { return cstr_; }
    explicit(false) constexpr operator const char*() const { return c_str(); }

    [[nodiscard]] constexpr std::string_view as_view() const { return {cstr_, size_}; }
    explicit(false) constexpr operator std::string_view() const { return as_view(); }

private:
    std::size_t size_;
    const char* cstr_;
};

}  // namespace fixed_containers

#include <string_view>

namespace fixed_containers
{
/**
 * Gets the canonical type name of T. The name includes cv-qualification and is
 * always fully-qualified (includes namespaces).
 *
 * Adapted from:
 * https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
 * License: CC BY-SA 3.0 (https://stackoverflow.com/posts/81870/timeline)
 * License link: https://creativecommons.org/licenses/by-sa/3.0/legalcode
 */
template <typename T>
constexpr auto type_name()
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto fixed_containers::type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto fixed_containers::type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl fixed_containers::type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}
}  // namespace fixed_containers

#include <cstddef>
#include <cstdlib>

namespace fixed_containers::customize
{
template <class T>
concept SequenceContainerChecking = requires(std::size_t i,
                                             std::size_t s,
                                             const StringLiteral& error_message,
                                             const std_transition::source_location& loc) {
    T::out_of_range(i, s, loc);  // ~ std::out_of_range
    T::length_error(s, loc);     // ~ std::length_error
    T::empty_container_access(loc);
    T::invalid_argument(error_message, loc);  // ~ std::invalid_argument
};

template <typename T, std::size_t /*MAXIMUM_SIZE*/>
struct SequenceContainerAbortChecking
{
    // TYPE_NAME and MAXIMUM_SIZE are not used, but are meant as an example
    // for Checking implementations that will utilize this information.
    static constexpr auto TYPE_NAME = fixed_containers::type_name<T>();

    [[noreturn]] static void out_of_range(const std::size_t /*index*/,
                                          const std::size_t /*size*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void length_error(const std::size_t /*target_capacity*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void empty_container_access(const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void invalid_argument(
        const fixed_containers::StringLiteral& /*error_message*/,
        const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};
}  // namespace fixed_containers::customize

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>

namespace fixed_containers::fixed_vector_detail
{
template <class T, class FixedVectorType>
class FixedVectorBuilder
{
public:
    constexpr FixedVectorBuilder() {}

    constexpr FixedVectorBuilder& push_back(const T& key) & noexcept
    {
        vector_.push_back(key);
        return *this;
    }
    constexpr FixedVectorBuilder&& push_back(const T& key) && noexcept
    {
        return std::move(push_back(key));
    }

    constexpr FixedVectorBuilder& push_back_all(std::initializer_list<T> list) & noexcept
    {
        push_back_all(list.begin(), list.end());
        return *this;
    }
    constexpr FixedVectorBuilder&& push_back_all(std::initializer_list<T> list) && noexcept
    {
        return std::move(push_back_all(list));
    }

    template <InputIterator InputIt>
    constexpr FixedVectorBuilder& push_back_all(InputIt first, InputIt last) & noexcept
    {
        vector_.insert(vector_.end(), first, last);
        return *this;
    }
    template <InputIterator InputIt>
    constexpr FixedVectorBuilder&& push_back_all(InputIt first, InputIt last) && noexcept
    {
        return std::move(push_back_all(first, last));
    }

    template <class Container>
    constexpr FixedVectorBuilder& push_back_all(const Container& container) & noexcept
    {
        push_back_all(container.cbegin(), container.cend());
        return *this;
    }
    template <class Container>
    constexpr FixedVectorBuilder&& push_back_all(const Container& container) && noexcept
    {
        return std::move(push_back_all(container.cbegin(), container.cend()));
    }

    constexpr FixedVectorType build() const& { return vector_; }
    constexpr FixedVectorType&& build() && { return std::move(vector_); }

private:
    FixedVectorType vector_;
};

// FixedVector<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should FixedVector<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// [WORKAROUND-1] due to destructors: manually do the split with template specialization.
// FixedVectorBase is only used for avoiding too much duplication for the destructor split
template <typename T, std::size_t MAXIMUM_SIZE, customize::SequenceContainerChecking CheckingType>
class FixedVectorBase
{
    /*
     * Use OptionalStorageTransparent, to properly support constexpr .data() for simple types.
     *
     * In order to operate on a pointer at compile-time (e.g. increment), it needs to be a
     * consecutive block of T's. If we use the OptionalStorage wrapper (non-transparent), we instead
     * have a consecutive block of OptionalStorage<T>. The compiler would figure that out at
     * compile time and reject it even though they have the same layout. In that case,
     * vector.data()[0] would be accessible at constexpr, but vector.data()[1] would be rejected.
     */
    using OptionalT = optional_storage_detail::OptionalStorageTransparent<T>;
    static_assert(consteval_compare::equal<sizeof(OptionalT), sizeof(T)>);
    // std::vector has the following restrictions too
    static_assert(IsNotReference<T>, "References are not allowed");
    static_assert(std::same_as<std::remove_cv_t<T>, T>,
                  "Vector must have a non-const, non-volatile value_type");
    using Checking = CheckingType;
    using Array = std::array<OptionalT, MAXIMUM_SIZE>;

    struct Mapper
    {
        constexpr T& operator()(OptionalT& opt_storage) const noexcept
        {
            return optional_storage_detail::get(opt_storage);
        }
        constexpr const T& operator()(const OptionalT& opt_storage) const noexcept
        // requires(not std::is_const_v<OptionalT>)
        {
            return optional_storage_detail::get(opt_storage);
        }
    };

    template <IteratorConstness CONSTNESS>
    using IteratorImpl = RandomAccessIteratorTransformer<typename Array::const_iterator,
                                                         typename Array::iterator,
                                                         Mapper,
                                                         Mapper,
                                                         CONSTNESS>;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using const_iterator = IteratorImpl<IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = IteratorImpl<IteratorConstness::MUTABLE_ITERATOR>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

private:
    static constexpr void check_target_size(size_type target_size,
                                            const std_transition::source_location& loc)
    {
        if (preconditions::test(target_size <= MAXIMUM_SIZE))
        {
            Checking::length_error(target_size, loc);
        }
    }

public:  // Public so this type is a structural type and can thus be used in template parameters
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    Array IMPLEMENTATION_DETAIL_DO_NOT_USE_array_;

public:
    constexpr FixedVectorBase() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{0}
    // Don't initialize the array
    {
        // A constexpr context requires everything to be initialized.
        // The OptionalStorage wrapper takes care of that, but for unwrapped objects
        // while also being in a constexpr context, initialize array.
        if constexpr (!std::same_as<OptionalT, optional_storage_detail::OptionalStorage<T>>)
        {
            if (std::is_constant_evaluated())
            {
                memory::construct_at_address_of(array());
            }
        }
    }

    constexpr FixedVectorBase(std::size_t count,
                              const T& value,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase()
    {
        check_target_size(count, loc);
        set_size(count);
        for (std::size_t i = 0; i < count; i++)
        {
            place_at(i, value);
        }
    }

    constexpr explicit FixedVectorBase(std::size_t count,
                                       const std_transition::source_location& loc =
                                           std_transition::source_location::current()) noexcept
      : FixedVectorBase(count, T(), loc)
    {
    }

    template <InputIterator InputIt>
    constexpr FixedVectorBase(InputIt first,
                              InputIt last,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase()
    {
        insert(cend(), first, last, loc);
    }

    constexpr FixedVectorBase(std::initializer_list<T> list,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase(list.begin(), list.end(), loc)
    {
    }

    /**
     * Resizes the container to contain `count` elements.
     * If the current size is greater than count, the container is reduced to its first count
     * elements. If the current size is less than count, additional elements are appended
     * (default/copy initialized).
     */
    constexpr void resize(
        size_type count,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->resize(count, T{}, loc);
    }
    constexpr void resize(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);

        // Reinitialize the new members if we are enlarging
        while (size() < count)
        {
            place_at(end_index(), v);
            increment_size();
        }
        // Destroy extras if we are making it smaller.
        while (size() > count)
        {
            destroy_at(back_index());
            decrement_size();
        }
    }

    /**
     * Appends the given element value to the end of the container.
     * Calling push_back on a full container is undefined.
     */
    constexpr void push_back(
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(v);
    }
    constexpr void push_back(
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(std::move(v));
    }
    /**
     * Emplace the given element at the end of the container.
     * Calling emplace_back on a full container is undefined.
     */
    template <class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        emplace_at(end_index(), std::forward<Args>(args)...);
        increment_size();
        return this->back();
    }

    /**
     * Removes the last element of the container.
     * Calling pop_back on an empty container is undefined.
     */
    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(back_index());
        decrement_size();
    }

    /**
     * Inserts elements at the iterator-specified location in the container.
     * Calling insert on a full container is undefined.
     */
    constexpr iterator insert(
        const_iterator it,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        auto entry_it = advance_all_after_iterator_by_n(it, 1);
        memory::construct_at_address_of(*entry_it, v);
        return entry_it;
    }
    constexpr iterator insert(
        const_iterator it,
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        auto entry_it = advance_all_after_iterator_by_n(it, 1);
        memory::construct_at_address_of(*entry_it, std::move(v));
        return entry_it;
    }
    template <InputIterator InputIt>
    constexpr iterator insert(
        const_iterator it,
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            typename std::iterator_traits<InputIt>::iterator_category{}, it, first, last, loc);
    }
    constexpr iterator insert(
        const_iterator it,
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            std::random_access_iterator_tag{}, it, ilist.begin(), ilist.end(), loc);
    }

    /**
     * Emplace element at the iterator-specified location in the container.
     * Calling emplace on a full container is undefined.
     */
    template <class... Args>
    constexpr iterator emplace(const_iterator it, Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        auto entry_it = advance_all_after_iterator_by_n(it, 1);
        memory::construct_at_address_of(*entry_it, std::forward<Args>(args)...);
        return entry_it;
    }

    /**
     * Replaces the contents with count copies of a given value
     */
    constexpr void assign(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);
        this->clear();
        this->resize(count, v);
    }

    /**
     * Replaces the contents with copies of those in range [first, last)
     */
    template <InputIterator InputIt>
    constexpr void assign(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(cend(), first, last, loc);
    }

    constexpr void assign(
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(cend(), ilist, loc);
    }

    /**
     * Erases the specified range of elements from the container.
     */
    constexpr iterator erase(const_iterator first,
                             const_iterator last,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(first <= last))
        {
            Checking::invalid_argument("first > last, range is invalid", loc);
        }
        if (preconditions::test(first >= cbegin() && last <= cend()))
        {
            Checking::invalid_argument("iterators exceed container range", loc);
        }

        const auto entry_count_to_move = std::distance(last, cend());
        const auto entry_count_to_remove = std::distance(first, last);

        iterator read_start_it = const_to_mutable_it(last);
        iterator read_end_it = std::next(read_start_it, entry_count_to_move);
        iterator write_start_it = const_to_mutable_it(first);

        if (!std::is_constant_evaluated())
        {
            // We can only use this when `!is_constant_evaluated`, since otherwise Clang
            // complains about objects being accessed outside their lifetimes.

            // Clean out the gap
            destroy_range(write_start_it, std::next(write_start_it, entry_count_to_remove));

            // Do the relocation
            algorithm::uninitialized_relocate(read_start_it, read_end_it, write_start_it);
        }
        else
        {
            // Do the move
            iterator write_end_it = std::move(read_start_it, read_end_it, write_start_it);

            // Clean out the tail
            destroy_range(write_end_it, read_end_it);
        }

        decrement_size(static_cast<std::size_t>(entry_count_to_remove));
        return write_start_it;
    }

    /**
     * Erases the specified element from the container.
     */
    constexpr iterator erase(const_iterator it,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        return erase(it, std::next(it), loc);
    }

    /**
     * Erases all elements from the container. After this call, size() returns zero.
     */
    constexpr void clear() noexcept
    {
        destroy_range(begin(), end());
        set_size(0);
    }

    /**
     * Regular accessors.
     */
    constexpr reference operator[](size_type i) noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }
    constexpr const_reference operator[](size_type i) const noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }

    constexpr reference at(size_type i,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(i < size()))
        {
            Checking::out_of_range(i, size(), loc);
        }
        return unchecked_at(i);
    }
    constexpr const_reference at(size_type i,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) const noexcept
    {
        if (preconditions::test(i < size()))
        {
            Checking::out_of_range(i, size(), loc);
        }
        return unchecked_at(i);
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(front_index());
    }
    constexpr const_reference front(const std_transition::source_location& loc =
                                        std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(front_index());
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(back_index());
    }
    constexpr const_reference back(const std_transition::source_location& loc =
                                       std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(back_index());
    }

    constexpr value_type* data() noexcept
    {
        return std::addressof(optional_storage_detail::get(*array().data()));
    }
    constexpr const value_type* data() const noexcept
    {
        return std::addressof(optional_storage_detail::get(*array().data()));
    }

    /**
     * Iterators
     */
    constexpr iterator begin() noexcept { return create_iterator(front_index()); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(front_index());
    }
    constexpr iterator end() noexcept { return create_iterator(end_index()); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(end_index()); }

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    /**
     * Size
     */
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return max_size(); }
    constexpr void reserve(const std::size_t new_capacity,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(new_capacity <= MAXIMUM_SIZE))
        {
            Checking::length_error(new_capacity, loc);
        }
        // Do nothing
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    /**
     * Equality.
     */
    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedVectorBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        if constexpr (MAXIMUM_SIZE == MAXIMUM_SIZE_2)
        {
            if (this == &other)
            {
                return true;
            }
        }

        return std::ranges::equal(*this, other);
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr auto operator<=>(const FixedVectorBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return std::lexicographical_compare_three_way(
            cbegin(), cend(), other.cbegin(), other.cend());
    }

private:
    constexpr iterator advance_all_after_iterator_by_n(const const_iterator it, const std::size_t n)
    {
        const std::ptrdiff_t value_count_to_move = std::distance(it, cend());
        increment_size(n);  // Increment now so iterators are all within valid range

        auto read_start_it = const_to_mutable_it(it);
        auto read_end_it = std::next(read_start_it, value_count_to_move);
        auto write_end_it =
            std::next(read_start_it, static_cast<std::ptrdiff_t>(n) + value_count_to_move);
        algorithm::uninitialized_relocate_backward(read_start_it, read_end_it, write_end_it);

        return read_start_it;
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::forward_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const auto entry_count_to_add = static_cast<std::size_t>(std::distance(first, last));
        check_target_size(size() + entry_count_to_add, loc);

        auto write_it = advance_all_after_iterator_by_n(it, entry_count_to_add);
        for (auto w_it = write_it; first != last; std::advance(first, 1), std::advance(w_it, 1))
        {
            memory::construct_at_address_of(*w_it, *first);
        }
        return write_it;
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::input_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        auto first_it = const_to_mutable_it(it);
        auto middle_it = end();

        // Place everything at the end
        for (; first != last && size() < max_size(); ++first)
        {
            push_back_internal(*first);
        }

        if (first != last)  // Reached capacity
        {
            std::size_t excess_element_count = 0;
            for (; first != last; ++first)
            {
                excess_element_count++;
            }

            Checking::length_error(MAXIMUM_SIZE + excess_element_count, loc);
        }

        // Rotate into the correct places
        std::rotate(first_it, middle_it, end());

        return first_it;
    }

    constexpr iterator create_iterator(const std::size_t offset_from_start) noexcept
    {
        auto array_it =
            std::next(std::begin(array()), static_cast<difference_type>(offset_from_start));
        return iterator{array_it, Mapper{}};
    }

    constexpr const_iterator create_const_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        auto array_it =
            std::next(std::begin(array()), static_cast<difference_type>(offset_from_start));
        return const_iterator{array_it, Mapper{}};
    }

private:
    constexpr iterator const_to_mutable_it(const_iterator it)
    {
        return std::next(begin(), std::distance(cbegin(), it));
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(size() < MAXIMUM_SIZE))
        {
            Checking::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }
    constexpr void check_not_empty(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!empty()))
        {
            Checking::empty_container_access(loc);
        }
    }

    [[nodiscard]] constexpr std::size_t front_index() const { return 0; }
    [[nodiscard]] constexpr std::size_t back_index() const { return end_index() - 1; }
    [[nodiscard]] constexpr std::size_t end_index() const { return size(); }

    constexpr const Array& array() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_; }
    constexpr Array& array() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_; }

    constexpr void increment_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }
    constexpr void set_size(const std::size_t size)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = size;
    }

    constexpr const T& unchecked_at(const std::size_t i) const
    {
        return optional_storage_detail::get(array()[i]);
    }
    constexpr T& unchecked_at(const std::size_t i)
    {
        return optional_storage_detail::get(array()[i]);
    }

    constexpr void destroy_at(std::size_t)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_at(std::size_t i)
        requires NotTriviallyDestructible<T>
    {
        memory::destroy_at_address_of(unchecked_at(i));
    }

    constexpr void destroy_range(iterator /*first*/, iterator /*last*/)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_range(iterator first, iterator last)
        requires NotTriviallyDestructible<T>
    {
        for (; first != last; ++first)
        {
            memory::destroy_at_address_of(*first);
        }
    }

    constexpr void place_at(const std::size_t i, const value_type& v)
    {
        memory::construct_at_address_of(unchecked_at(i), v);
    }
    constexpr void place_at(const std::size_t i, value_type&& v)
    {
        memory::construct_at_address_of(unchecked_at(i), std::move(v));
    }

    template <class... Args>
    constexpr void emplace_at(const std::size_t i, Args&&... args)
    {
        memory::construct_at_address_of(unchecked_at(i), std::forward<Args>(args)...);
    }

    // [WORKAROUND-1] - Needed by the non-trivially-copyable flavor of FixedVector
protected:
    constexpr void push_back_internal(const value_type& v)
    {
        place_at(end_index(), v);
        increment_size();
    }

    constexpr void push_back_internal(value_type&& v)
    {
        place_at(end_index(), std::move(v));
        increment_size();
    }
};

}  // namespace fixed_containers::fixed_vector_detail

namespace fixed_containers::fixed_vector_detail::specializations
{
template <typename T, std::size_t MAXIMUM_SIZE, customize::SequenceContainerChecking CheckingType>
class FixedVector : public fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    using Builder =
        fixed_vector_detail::FixedVectorBuilder<T, FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

    constexpr FixedVector() noexcept
      : Base()
    {
    }
    constexpr FixedVector(std::size_t count,
                          const T& value,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedVector(std::size_t count,
                                   const std_transition::source_location& loc =
                                       std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedVector(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
    constexpr FixedVector(std::initializer_list<T> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }

    constexpr FixedVector(const FixedVector& other)
        requires TriviallyCopyConstructible<T>
    = default;
    constexpr FixedVector(FixedVector&& other) noexcept
        requires TriviallyMoveConstructible<T>
    = default;
    constexpr FixedVector& operator=(const FixedVector& other)
        requires TriviallyCopyAssignable<T>
    = default;
    constexpr FixedVector& operator=(FixedVector&& other) noexcept
        requires TriviallyMoveAssignable<T>
    = default;

    constexpr FixedVector(const FixedVector& other)
      : FixedVector(other.begin(), other.end())
    {
    }
    constexpr FixedVector(FixedVector&& other) noexcept
      : FixedVector()
    {
        for (T& entry : other)
        {
            this->push_back_internal(std::move(entry));
        }

        // Clear the moved-out-of-vector. This is consistent with both std::vector
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr FixedVector& operator=(const FixedVector& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->assign(other.begin(), other.end());
        return *this;
    }
    constexpr FixedVector& operator=(FixedVector&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        for (T& entry : other)
        {
            this->push_back_internal(std::move(entry));
        }
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across FixedVectors. std::vector<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~FixedVector() noexcept { this->clear(); }
};

template <TriviallyCopyable T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType>
class FixedVector<T, MAXIMUM_SIZE, CheckingType>
  : public fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    using Builder =
        fixed_vector_detail::FixedVectorBuilder<T, FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

    constexpr FixedVector() noexcept
      : Base()
    {
    }
    constexpr FixedVector(std::size_t count,
                          const T& value,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedVector(std::size_t count,
                                   const std_transition::source_location& loc =
                                       std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedVector(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
    constexpr FixedVector(std::initializer_list<T> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
};

}  // namespace fixed_containers::fixed_vector_detail::specializations

namespace fixed_containers
{
/**
 * Fixed-capacity vector with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 */
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedVector
  : public fixed_vector_detail::specializations::FixedVector<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_vector_detail::specializations::FixedVector<T, MAXIMUM_SIZE, CheckingType>;

public:
    using Builder =
        fixed_vector_detail::FixedVectorBuilder<T, FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

    constexpr FixedVector() noexcept
      : Base()
    {
    }
    constexpr FixedVector(std::initializer_list<T> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
    constexpr FixedVector(std::size_t count,
                          const T& value,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedVector(std::size_t count,
                                   const std_transition::source_location& loc =
                                       std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedVector(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type is_full(
    const FixedVector<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= c.max_size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename U>
constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type erase(
    FixedVector<T, MAXIMUM_SIZE, CheckingType>& c, const U& value)
{
    const auto original_size = c.size();
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    return original_size - c.size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename Predicate>
constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type erase_if(
    FixedVector<T, MAXIMUM_SIZE, CheckingType>& c, Predicate predicate)
{
    const auto original_size = c.size();
    c.erase(std::remove_if(c.begin(), c.end(), predicate), c.end());
    return original_size - c.size();
}

/**
 * Construct a FixedVector with its capacity being deduced from the number of items being passed.
 */
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedVectorType = FixedVector<T, MAXIMUM_SIZE, CheckingType>>
[[nodiscard]] constexpr FixedVectorType make_fixed_vector(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), loc};
}

template <typename T, std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_vector(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>;
    using FixedVectorType = FixedVector<T, MAXIMUM_SIZE, CheckingType>;
    return make_fixed_vector<T, CheckingType, MAXIMUM_SIZE, FixedVectorType>(list, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedVector<T, MAXIMUM_SIZE, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
