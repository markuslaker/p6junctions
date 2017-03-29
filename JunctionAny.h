#if !defined P6JunctionAny_h
#define      P6JunctionAny_h

// An Any-junction collapses to true if a Boolean test returns true for any of
// its members.

#include "Junction.h"
#include "JunctionPiggyBackStore.h"
#include "JunctionReverseComparisons.h"
#include "JunctionSortedStore.h"

namespace P6 {

// A None-junction is implemented as an inverted Any-junction, on the basis
// that, for example,
// (none({x, y, x}) == 3) <=> not(any({1, 2, 3}) == 3)

template<typename Store, bool MustInvert>
class AnyOrNone: public Junction<Store> {
    using Jct     = Junction<Store>;
    using Element = typename Jct::Element;

    static bool Invert(bool const b) {
        return b ^ MustInvert;
    }

public:
    // True if the junction copied the elements into a new std::set on
    // construction, enabling some optimisations:
    static bool const Ordered = Store::Ordered;

    // It's syntactically easier to use the helper functions at the end of this
    // header file than to call constructors directly:
    explicit AnyOrNone(std::initializer_list<Element> const &ilist):  Jct(ilist)   { }

    template<typename Container>
    explicit AnyOrNone(Container const &container):                   Jct(container)   { }

    template<typename Elt>
    explicit AnyOrNone(std::set<Elt> &&container):                    Jct(std::move(container))   { }

    template<typename Iterator>
    AnyOrNone(Iterator const begin, Iterator const end):              Jct(begin, end)   { }

    template<typename OtherStore>
    explicit AnyOrNone(Any<OtherStore> const &other):                 Jct(other.Elements())   { }

    // Applying a lambda returns a modified copy of the junction:
    template<typename Lambda>
    auto operator () (Lambda const &lambda) const {
        using ResultElement = decltype(lambda(Jct::GetAnyElement()));
        using Result        = AnyOrNone<Details::JunctionSortedStore<ResultElement>, MustInvert>;
        return Jct::template Map<Result> (lambda);
    }

    // Every junction can statically return its own type:
    static JunctionType constexpr GetJunctionType() {
        return MustInvert? JunctionType::None: JunctionType::All;
    }

    // Because std::set stores elements in ascending order, many of these
    // comparison operators need look at only the first or last element when
    // the backing store is sorted.
    //
    // For example, in (any({1, 2, 3}) > n), we need look only at the highest
    // element, which is the last in the set; one match is enough for us to
    // collapse to true.
    //
    // None-junctions on the RHS are a special case, and need handling
    // separately.  If we see (any({2, 3, 4}) > none({x, y, z})), the lowest
    // element, the 2, is more likely to return true than the highest element,
    // the 4, and, again, that one match is enough for a collapse to true.
    // Therefore, order is reversed for none-junctions.
    //
    // One-junctions are a further special case.  We can't predict which of our
    // elements will match a one-junction, and so we must try them all in turn
    // until one matches.

private:
    template<typename Lambda>
    bool CheckAllElements(Lambda const &lambda) const {
        for (auto const &elem: Jct::Elements())
            if (lambda(elem))
                return true;

        return false;
    }

public:
    // <

    template<typename NoneStore>
    typename Details::EnableIf2<Ordered, bool, NoneStore>::type operator < (None<NoneStore> const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::LastElement() < rhs);
    }

    template<typename OneStore>
    bool operator < (One<OneStore> const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;}));
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::FirstElement() < rhs);
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;}));
    }

    // <=

    template<typename NoneStore>
    typename Details::EnableIf2<Ordered, bool, NoneStore>::type operator <= (None<NoneStore> const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::LastElement() <= rhs);
    }

    template<typename OneStore>
    bool operator <= (One<OneStore> const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem <= rhs;}));
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Ordered, bool, ElementOrJunction>::type operator <= (ElementOrJunction const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::FirstElement() <= rhs);
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Ordered, bool, ElementOrJunction>::type operator <= (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem <= rhs;}));
    }

    // ==, !=

    // TODO: if both junctions are sorted then we should be able to compare for
    // equality in O(N+M) time, where N and M are the sizes of the junctions.
    // If only one is sorted, we should be able to get it down to O(N lg M),
    // where M is the size of the sorted junction.

    // There's no short-cut when we check for equality or inequality:
    template<typename ElementOrJunction>
    bool operator == (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem == rhs;}));
    }

    // operator != can't be a straight negation of operator ==, because
    // (any(1, 2) == 2) and (any(1, 2) != 2) are both true.
    template<typename ElementOrJunction>
    bool operator != (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem != rhs;}));
    }

    // >=
    //
    // This can't be a straight negation of operator <, because
    // (any(1, 2) < 2) and (any(1, 2) >= 2) are both true.

    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator >= (None<NoneStore> const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::FirstElement() >= rhs);
    }

    template<typename OneStore>
    bool operator >= (One<OneStore> const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem >= rhs;}));
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator >= (ElementOrJunction const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::LastElement() >= rhs);
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator >= (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem >= rhs;}));
    }

    // >
    //
    // This can't be a straight negation of operator <=, because
    // (any(1, 2, 3) <= 2) and (any(1, 2, 3) > 2) are both true.
    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator > (None<NoneStore> const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::FirstElement() > rhs);
    }

    template<typename OneStore>
    bool operator > (One<OneStore> const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;}));
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return Invert(not Jct::IsEmpty() and Jct::LastElement() > rhs);
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return Invert(CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;}));
    }
};

// Helper functions to create Any-junctions:
//
// Memory management, as usual, takes some care.  Imagine these four cases:
//
// int i = ...;
// bool b1 = i == any({2, 3, 5, 7, 11});
//
// int j = ...;
// auto any_low_prime = any({2, 3, 5, 7, 11});
// bool b2 = i == any_low_prime or j == any_low_prime;
//
// std::vector<int> low_fibonacci {1, 1, 2, 3, 5, 8};
// bool b3 = i == any(fib_vec);
//
// auto any_low_fib = any(low_fibonacci);
// bool b4 = i == any_low_fib or j == any_low_fib;
//
// With b1, the brace-expression (a std::initializer_list<int>) is a temporary
// object, but it lives until the end of the expression that initialises b1.
// The object created by any() is also a temporary object, which is destroyed at
// the end of the same expression.  So there's no logical need for any() to copy
// the brace-expression.  With b2, in contrast, the entire std::initializer_list
// does need copying, because the object created by any() must survive after the
// brace-expression has been destroyed.
//
// Unfortunately, any() can't tell whether the object it creates will be a
// temporary or not.  It therefore doesn't know whether that object will outlive
// the objects passed as arguments to any(), and whether those objects need
// copying.  It errs on the side of safety, and always copies its arguments if
// it receives an rvalue reference, which usually signifies that the argument is
// a temporary.
//
// For b3 and b4, conversely, the named object low_fibonacci outlives both the
// temporary object created by any() for the definition of b3, and also the
// named object any_low_fib.  Therefore, any() needn't copy its input when it's
// passed a named object via an lvalue reference, and, for performance, it
// doesn't do so.
//
// The disadvantage of not copying low_fibonacci during the construction of
// any_low_fib is that, if the contents of low_fibonacci change, the behaviour
// of any_low_fib will change as well:
//
// auto any_low_fib = any(low_fibonacci);
// assert(not(13 == any_low_fib));
// low_fibonacci.push_back(13);
// assert(13 == any_low_fib);
//
// By default, any() copies arguments received by rvalue references (which are
// usually temporaries) and doesn't copy others.  To force named arguments to be
// copied when they wouldn't normally be, call any_copy() instead of copy:
//
// auto any_low_fib = any_copy(low_fibonacci);
// assert(not(13 == any_low_fib));
// low_fibonacci.push_back(13);                 // Doesn't change any_low_fib.
// assert(not(13 == any_low_fib));              // Unchanged.
//
// To prevent temporary arguments from being copied when they would otherwise
// be, call any_ref():
//
// bool b5 = i == any_ref({2, 3, 5, 7});        // Safe: the object constructed by any_ref is a temporary
//
// auto any_tiny_prime = any_ref({2, 3, 5});    // ERROR: don't do this: any_tiny_prime outlives the {2, 3, 5} temporary object to which it refers
// bool b6 = i == any_tiny_prime;               // UNDEFINED BEHAVIOUR caused by dangling reference
//
// If in doubt, avoid problems by not calling any_ref().
//
// Passing a pair of iterators currently causes a copy with no way to override
// it, but this behaviour will change.
//
// Memory management for one-, one- and all-junctions works in the same way as
// for any-junctions.

//
// Initialiser lists:
//

// Force the absence of a copy:

template<typename Element>
auto any_ref(std::initializer_list<Element> const &ilist) {
    using Store = Details::JunctionPiggyBackStore<std::initializer_list<Element>>;
    return AnyOrNone<Store, false> (ilist);
}

template<typename Element>
auto none_ref(std::initializer_list<Element> const &ilist) {
    using Store = Details::JunctionPiggyBackStore<std::initializer_list<Element>>;
    return AnyOrNone<Store, true> (ilist);
}

// Force a copy:

template<typename Element>
auto any_copy(std::initializer_list<Element> ilist) {
    using Store = Details::JunctionSortedStore<Element>;
    return AnyOrNone<Store, false> (ilist);
}

template<typename Element>
auto none_copy(std::initializer_list<Element> const ilist) {
    using Store = Details::JunctionSortedStore<Element>;
    return AnyOrNone<Store, true> (ilist);
}

// Temporaries get copied by default:

template<typename Element>
auto any(std::initializer_list<Element> &&ilist) {
    return any_copy(ilist);
}

template<typename Element>
auto none(std::initializer_list<Element> &&ilist) {
    return none_copy(ilist);
}

// Named variables get piggybacked by default:

template<typename Element>
auto any(std::initializer_list<Element> &ilist) {
    return any_ref(ilist);
}

template<typename Element>
auto none(std::initializer_list<Element> &ilist) {
    return none_ref(ilist);
}

//
// (Other) STL containers:
//

// Force the absence of a copy:

template<typename Container>
auto any_ref(Container const &container) {
    using Store = Details::JunctionPiggyBackStore<Container>;
    return AnyOrNone<Store, false> (container);
}

template<typename Container>
auto none_ref(Container const &container) {
    using Store = Details::JunctionPiggyBackStore<Container>;
    return AnyOrNone<Store, true> (container);
}

// Force a copy:

template<typename Container>
auto any_copy(Container const &container) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*container.begin())>::type>::type;
    using Store = Details::JunctionSortedStore<Element>;
    return AnyOrNone<Store, false> (container.begin(), container.end());
}

template<typename Container>
auto none_copy(Container const &container) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*container.begin())>::type>::type;
    using Store = Details::JunctionSortedStore<Element>;
    return AnyOrNone<Store, true> (container.begin(), container.end());
}

// By default, copy a temporary container passed by rvalue reference:

template<typename Container>
auto any(Container &&container) {
    return any_copy(container);
}

template<typename Container>
auto none(Container &&container) {
    return none_copy(container);
}

// Named containers passed by lvalue references don't get copied by default:

template<typename Container>
auto any(Container &container) {
    return any_ref(container);
}

template<typename Container>
auto none(Container &container) {
    return none_ref(container);
}

template<typename Container>
auto any(Container const &container) {
    return any_ref(container);
}

template<typename Container>
auto none(Container const &container) {
    return none_ref(container);
}

//
// Iterator pairs:
//

// For now, a pair of iterators causes a copy, although that'll change in time:

template<typename Iterator>
auto any(Iterator const begin, Iterator const end) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type;
    return AnyOrNone<Details::JunctionSortedStore<Element>, false> (begin, end);
}

template<typename Iterator>
auto none(Iterator const begin, Iterator const end) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type;
    return AnyOrNone<Details::JunctionSortedStore<Element>, true> (begin, end);
}

//
// Sets:
//

// A temporary std::set is a special case, because it's already sorted.  We can
// therefore move its contents into a JunctionSortedStore and get the benefit of
// the sorting that's already been done.

template<typename Element>
auto any(std::set<Element> &&elements) {
    return AnyOrNone<Details::JunctionSortedStore<Element>, false> (std::move(elements));
}

template<typename Element>
auto none(std::set<Element> &&elements) {
    return AnyOrNone<Details::JunctionSortedStore<Element>, true> (std::move(elements));
}

// TODO: assume a named std::set is sorted, too, and piggyback on it, rather
// than copying it and assuming it's unsorted, which is doubly bad.

}

#endif

