/*
Copyright (c) 2017, Mark Stephen Laker

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#if !defined P6JunctionAll_h
#define      P6JunctionAll_h

// An All-junction collapses to true if a Boolean test returns true for all of
// its members.

#include "Junction.h"
#include "JunctionPiggyBackStore.h"
#include "JunctionReverseComparisons.h"
#include "JunctionSortedStore.h"

namespace P6 {

template<typename Store>
class All: public Junction<Store> {
    using Jct     = Junction<Store>;
    using Element = typename Jct::Element;

public:
    // True if the junction copied the elements into a new std::set on
    // construction, enabling some optimisations:
    static bool constexpr Ordered = Store::Ordered;

    // It's syntactically easier to use the helper functions at the end of this
    // header file than to call constructors directly:
    explicit All(std::initializer_list<Element> const &ilist):  Jct(ilist)   { }

    template<typename Container>
    explicit All(Container const &container):                   Jct(container)   { }

    template<typename Elt>
    explicit All(std::set<Elt> &&container):                    Jct(std::move(container))   { }

    template<typename Iterator>
    All(Iterator const begin, Iterator const end):              Jct(begin, end)   { }

    template<typename OtherStore>
    explicit All(All<OtherStore> const &other):                 Jct(other.Elements())   { }

    // Applying a lambda returns a modified copy of the junction:
    template<typename Lambda>
    auto operator () (Lambda const &lambda) const {
        using ResultElement = decltype(lambda(Jct::GetAnyElement()));
        using Result        = All<Details::JunctionSortedStore<ResultElement>>;
        return Jct::template Map<Result> (lambda);
    }

    // Every junction can statically return its own type:
    static JunctionType constexpr GetJunctionType() {
        return JunctionType::All;
    }

    // Comparison operators follow.
    //
    // Because std::set stores elements in ascending order, many of these
    // comparison operators need look at only the first or last element when
    // the backing store is sorted.
    //
    // For example, in (all({1, 2, 3}) > n), we need look only at the lowest
    // element, which is the first in the set; the others are guaranteed to be 
    // larger than n if the first is.
    //
    // None-junctions on the RHS are a special case, and need handling
    // separately.  If we see (all({2, 3, 4}) > none({x, y, z})), the highest
    // element, the 4, is more likely to return false than the lowest element,
    // the 2, and, in fact, if 4 passes then 2 is guaranteed to pass.
    // Therefore, order is reversed for none-junctions.
    //
    // One-junctions are a further special case.  We can't predict which of our
    // elements will match a one-junction, and so we must try them all and see
    // whether they all match.

private:
    template<typename Lambda>
    bool CheckAllElements(Lambda const &lambda) const {
        for (auto const &elem: Jct::Elements())
            if (not(lambda(elem)))
                return false;

        return true;
    }

public:
    // <

    template<typename NoneStore>
    typename Details::EnableIf2<Ordered, bool, NoneStore>::type operator < (None<NoneStore> const &rhs) const {
        return Jct::IsEmpty() or Jct::FirstElement() < rhs;
    }

    template<typename OneStore>
    bool operator < (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return Jct::IsEmpty() or Jct::LastElement() < rhs;
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;});
    }

    // <=

    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator <= (None<NoneStore> const &rhs) const {
        return Jct::IsEmpty() or Jct::FirstElement() <= rhs;
    }

    template<typename OneStore>
    bool operator <= (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem <= rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator <= (ElementOrJunction const &rhs) const {
        return Jct::IsEmpty() or Jct::LastElement() <= rhs;
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator <= (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem <= rhs;});
    }

    // ==, !=

    // There's no short-cut when we check for equality or inequality:
    template<typename ElementOrJunction>
    bool operator == (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem == rhs;});
    }

    // operator != can't be a straight negation of operator ==, because
    // (all(1, 2) == 2) and (all(1, 2) != 2) are both false.
    template<typename ElementOrJunction>
    bool operator != (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem != rhs;});
    }

    // >=
    //
    // This can't be a straight negation of operator <, because
    // (all(1, 2) < 2) and (all(1, 2) >= 2) are both false.

    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator >= (None<NoneStore> const &rhs) const {
        return Jct::IsEmpty() or Jct::LastElement() >= rhs;
    }

    template<typename OneStore>
    bool operator >= (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem >= rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator >= (ElementOrJunction const &rhs) const {
        return Store::IsEmpty() or Jct::FirstElement() >= rhs;
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator >= (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem >= rhs;});
    }

    // >
    //
    // This can't be a straight negation of operator <=, because
    // (all(1, 2, 3) <= 2) and (all(1, 2, 3) > 2) are both false.
    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator > (None<NoneStore> const &rhs) const {
        return Jct::IsEmpty() or Jct::LastElement() > rhs;
    }

    template<typename OneStore>
    bool operator > (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return Jct::IsEmpty() or Jct::FirstElement() > rhs;
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;});
    }
};

// Helper functions to create All-junctions -- see the comments about memory-
// management in JunctionAny.h:

//
// Initialiser lists:
//

// Force the absence of a copy:

template<typename Element>
auto all_ref(std::initializer_list<Element> const &ilist) {
    using Store = Details::JunctionPiggyBackStore<std::initializer_list<Element>>;
    return All<Store> (ilist);
}

// Force a copy:

template<typename Element>
auto all_copy(std::initializer_list<Element> ilist) {
    using Store = Details::JunctionSortedStore<Element>;
    return All<Store> (ilist);
}

// Temporaries get copied by default:

template<typename Element>
auto all(std::initializer_list<Element> &&ilist) {
    return all_copy(ilist);
}

// Named variables get piggybacked by default:

template<typename Element>
auto all(std::initializer_list<Element> &ilist) {
    return all_ref(ilist);
}

//
// (Other) STL containers:
//

// Force the absence of a copy:

template<typename Container>
auto all_ref(Container const &container) {
    using Store = Details::JunctionPiggyBackStore<Container>;
    return All<Store> (container);
}

// Force a copy:

template<typename Container>
auto all_copy(Container const &container) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*container.begin())>::type>::type;
    using Store = Details::JunctionSortedStore<Element>;
    return All<Store> (container.begin(), container.end());
}

// By default, copy a temporary container passed by rvalue reference:

template<typename Container>
auto all(Container &&container) {
    return all_copy(container);
}

// Named containers passed by lvalue references don't get copied by default:

template<typename Container>
auto all(Container &container) {
    return all_ref(container);
}

template<typename Container>
auto all(Container const &container) {
    return all_ref(container);
}

//
// Iterator pairs:
//

// For now, a pair of iterators causes a copy, although that'll change in time:

template<typename Iterator>
auto all(Iterator const begin, Iterator const end) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type;
    return All<Details::JunctionSortedStore<Element>> (begin, end);
}

//
// Sets:
//

// A temporary std::set is a special case, because it's already sorted.  We can
// therefore move its contents into a JunctionSortedStore and get the benefit of
// the sorting that's already been done.

template<typename Element>
auto all(std::set<Element> &&elements) {
    return All<Details::JunctionSortedStore<Element>> (std::move(elements));
}

// TODO: assume a named std::set is sorted, too, and piggyback on it, rather
// than copying it and assuming it's unsorted, which is doubly bad.

}

#endif

