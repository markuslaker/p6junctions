#if !defined P6JunctionReverseComparisons_h
#define      P6JunctionReverseComparisons_h

// Defines reverse comparisons for junctions.  For example, if a junction
// defines jct == elem then this header file supplies the equivalent comparison
// elem == jct.

#include <type_traits>

namespace P6 { namespace Details {

class UntemplatedJunctionTag;

// SFINAE requires that one of the function's template arguments be passed to
// std::enable_if.  For cases where a method returns a fixed type that doesn't
// depend on any template parameter, we provide a third argument, just for
// SFINAE's benefit, and then completely ignore it.

template<bool B, typename T, typename IgnoredButProvidedForSfinae>
struct EnableIf2 {
    using type = T;
};

template<typename T, typename IgnoredButProvidedForSfinae>
struct EnableIf2<false, T, IgnoredButProvidedForSfinae> { };

// gcc-4.9.2, which ships with Debian Jessie, doesn't support C++14's variable
// templates.  So the best we can do is the following.

// Set `value' to compile-time true if the specified type is derived from
// Junction, but is not Junction itself:

template<typename T>
struct IsJunction {
    static bool const value = std::is_base_of<UntemplatedJunctionTag, T>::value and not std::is_base_of<T, UntemplatedJunctionTag>::value;
};

template<typename T, typename U>
struct IsOnlyFirstAJunction {
    static bool const value = IsJunction<T>::value and not IsJunction<U>::value;
};

template<typename T, typename U>
struct IsOnlySecondAJunction {
    static bool const value = IsJunction<U>::value and not IsJunction<T>::value;
};

// Define a type only if the second argument is a Junction and the first isn't;
// leave it undefined otherwise.  Used in SFINAE.

template<typename Arg1, typename Arg2, typename Result>
struct WhereOnlySecondIsAJunction: std::enable_if<IsOnlySecondAJunction<Arg1, Arg2>::value, Result> { };

}   // out of namespace Details

// These templates assume conventional relationships between the usual
// comparison operators: for example, that ((a < b) <=> (b >= a)).  That doesn't
// apply to junctions, which is one reason we don't use them for junction-to-
// junction comparisons.

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator < (V const &v, J const &j) {
    return j > v;
}

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator <= (V const &v, J const &j) {
    return j >= v;
}

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator == (V const &v, J const &j) {
    return j == v;
}

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator >= (V const &v, J const &j) {
    return j <= v;
}

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator > (V const &v, J const &j) {
    return j < v;
}

template<typename V, typename J>
typename Details::WhereOnlySecondIsAJunction<V, J, bool>::type operator != (V const &v, J const &j) {
    return j != v;
}

}

#endif

