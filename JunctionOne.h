#if !defined P6JunctionOne_h
#define      P6JunctionOne_h

// A One-junction collapses to true if a Boolean test returns true for exactly
// one of its members.

#include "Junction.h"
#include "JunctionPiggyBackStore.h"
#include "JunctionReverseComparisons.h"
#include "JunctionSortedStore.h"

namespace P6 {

template<typename Store>
class One: public Junction<Store> {
    using Jct     = Junction<Store>;
    using Element = typename Jct::Element;

public:
    // True if the junction copied the elements into a new std::set on
    // construction, enabling some optimisations:
    static bool constexpr Ordered = Store::Ordered;

    // It's syntactically easier to use the helper functions at the end of this
    // header file than to call constructors directly:
    explicit One(std::initializer_list<Element> const &ilist):  Jct(ilist)   { }

    template<typename Container>
    explicit One(Container const &container):                   Jct(container)   { }

    template<typename Elt>
    explicit One(std::set<Elt> &&container):                    Jct(std::move(container))   { }

    template<typename Iterator>
    One(Iterator const begin, Iterator const end):              Jct(begin, end)   { }

    template<typename OtherStore>
    explicit One(All<OtherStore> const &other):                 Jct(other.Elements())   { }

    // Applying a lambda returns a modified copy of the junction:
    template<typename Lambda>
    auto operator () (Lambda const &lambda) const {
        using ResultElement = decltype(lambda(Jct::GetAnyElement()));
        using Result        = One<Details::JunctionSortedStore<ResultElement>>;
        return Jct::template Map<Result> (lambda);
    }

    // Every junction can statically return its own type:
    static JunctionType constexpr GetJunctionType() {
        return JunctionType::One;
    }

    // Because std::set stores elements in ascending order, many of these
    // comparison operators need look at only the first or last element when
    // the backing store is sorted.
    //
    // elements.  For example, in (one({1, 2, 3}) > n), we need check only that
    // (3 > n and not(2 > n)); we needn't look at the smallest element.
    //
    // None-junctions on the RHS are a special case, and need handling
    // separately.  If we see (one({2, 3, 4}) > none({x, y, z})), the highest
    // element, the 4, is more likely to return false than the lowest element,
    // the 2, and, in fact, if 4 passes then 2 is guaranteed to pass.
    // Therefore, order is reversed for none-junctions: we check that
    // (2 > N and not(3 > N)), where N = none({x, y, z}, and that's all we need
    // to do.
    //
    // Other one-junctions, as in (one({p, q, r}) > one({x, y, z})), are a
    // further special case.  We can't predict which of our elements will match
    // a one-junction, and so we must try them all and see how many match.

private:
    template<typename Lambda>
    bool CheckAllElements(Lambda const &lambda) const {
        unsigned matches {0};
        for (auto const &elem: Jct::Elements())
            if (lambda(elem))
                if (++matches > 1)
                    return false;

        return matches == 1;
    }

public:
    // <

    template<typename NoneStore>
    typename Details::EnableIf2<Ordered, bool, NoneStore>::type operator < (None<NoneStore> const &rhs) const {
        return not Jct::IsEmpty()       and
               Jct::LastElement() < rhs and
               not(Jct::HasSecondElement() and Jct::PenultimateElement() < rhs); 
    }

    template<typename OneStore>
    bool operator < (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return not Jct::IsEmpty()           and
               Jct::FirstElement() < rhs   and
               not(Jct::HasSecondElement() and Jct::SecondElement() < rhs);
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Ordered, bool, ElementOrJunction>::type operator < (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem < rhs;});
    }

    // <=

    template<typename NoneStore>
    typename Details::EnableIf2<Store::Ordered, bool, NoneStore>::type operator <= (None<NoneStore> const &rhs) const {
        return not Jct::IsEmpty()       and
               Jct::LastElement() <= rhs and
               not(Jct::HasSecondElement() and Jct::PenultimateElement() <= rhs); 
    }

    template<typename OneStore>
    bool operator <= (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem <= rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator <= (ElementOrJunction const &rhs) const {
        return not Jct::IsEmpty()           and
               Jct::FirstElement() <= rhs   and
               not(Jct::HasSecondElement() and Jct::SecondElement() <= rhs);
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
        return not Jct::IsEmpty()       and
        Jct::FirstElement() >= rhs       and
        not (Jct::HasSecondElement() and Jct::SecondElement() >= rhs);
    }

    template<typename OneStore>
    bool operator >= (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem >= rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator >= (ElementOrJunction const &rhs) const {
        return not Jct::IsEmpty()           and
               Jct::LastElement() >= rhs    and
               not(Jct::HasSecondElement() and Jct::PenultimateElement() >= rhs); 
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
        return not Jct::IsEmpty()       and
        Jct::FirstElement() > rhs       and
        not (Jct::HasSecondElement() and Jct::SecondElement() > rhs);
    }

    template<typename OneStore>
    bool operator > (One<OneStore> const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;});
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return not Jct::IsEmpty()           and
               Jct::LastElement() > rhs     and
               not(Jct::HasSecondElement() and Jct::PenultimateElement() > rhs); 
    }

    template<typename ElementOrJunction>
    typename Details::EnableIf2<not Store::Ordered, bool, ElementOrJunction>::type operator > (ElementOrJunction const &rhs) const {
        return CheckAllElements([&rhs] (Element const &elem) {return elem > rhs;});
    }
};

// Helper functions to create One-junctions -- see the comments about memory-
// management in JunctionAny.h:

//
// Initialiser lists:
//

// Force the absence of a copy:

template<typename Element>
auto one_ref(std::initializer_list<Element> const &ilist) {
    using Store = Details::JunctionPiggyBackStore<std::initializer_list<Element>>;
    return One<Store> (ilist);
}

// Force a copy:

template<typename Element>
auto one_copy(std::initializer_list<Element> ilist) {
    using Store = Details::JunctionSortedStore<Element>;
    return One<Store> (ilist);
}

// Temporaries get copied by default:

template<typename Element>
auto one(std::initializer_list<Element> &&ilist) {
    return one_copy(ilist);
}

// Named variables get piggybacked by default:

template<typename Element>
auto one(std::initializer_list<Element> &ilist) {
    return one_ref(ilist);
}

template<typename Element>
auto one(std::initializer_list<Element> const &ilist) {
    return one_ref(ilist);
}

//
// (Other) STL containers:
//

// Force the absence of a copy:

template<typename Container>
auto one_ref(Container const &container) {
    using Store = Details::JunctionPiggyBackStore<Container>;
    return One<Store> (container);
}

// Force a copy:

template<typename Container>
auto one_copy(Container const &container) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*container.begin())>::type>::type;
    using Store = Details::JunctionSortedStore<Element>;
    return One<Store> (container.begin(), container.end());
}

// By default, copy a temporary container passed by rvalue reference:

template<typename Container>
auto one(Container &&container) {
    return one_copy(container);
}

// Named containers passed by lvalue references don't get copied by default:

template<typename Container>
auto one(Container &container) {
    return one_ref(container);
}

template<typename Container>
auto one(Container const &container) {
    return one_ref(container);
}

//
// Iterator pairs:
//

// For now, a pair of iterators causes a copy, although that'll change in time:

template<typename Iterator>
auto one(Iterator const begin, Iterator const end) {
    using Element = typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type;
    return One<Details::JunctionSortedStore<Element>> (begin, end);
}

//
// Sets:
//

// A temporary std::set is a special case, because it's already sorted.  We can
// therefore move its contents into a JunctionSortedStore and get the benefit of
// the sorting that's already been done.

template<typename Element>
auto one(std::set<Element> &&elements) {
    return One<Details::JunctionSortedStore<Element>> (std::move(elements));
}

// TODO: assume a named std::set is sorted, too, and piggyback on it, rather
// than copying it and assuming it's unsorted, which is doubly bad.
}

#endif

