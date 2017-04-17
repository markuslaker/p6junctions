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

#if !defined P6Junction_h
#define      P6Junction_h

// Defines a base class for all junctions.

#include <initializer_list>
#include <set>

// Provides a way to recognise a Junction without needing to think about
// templates:

namespace P6 {

template<typename T>
class Junction;

namespace Details {

class UntemplatedJunctionTag {
    UntemplatedJunctionTag() { }

    template<typename T>
    friend class ::P6::Junction;
};

} // Out of namespace Details

// Here's the base class itself.  "Store" will be JunctionPiggyBackStore if
// we're constructed via xxx_ref; for xxx_copy(), it's JunctionSortedStore,
// which copies all the elements in O(N log N) time and linear space.

template<typename Store>
class Junction: public Store, public Details::UntemplatedJunctionTag {
public:
    using Element = typename Store::Element;

    static bool constexpr IsOrdered() {
        return Store::Ordered;
    }

protected:
    template<typename Elem>
    Junction(std::initializer_list<Elem> const &ilist): Store(ilist)   { }

    template<typename Container>
    Junction(Container const &container): Store(container)   { }

    template<typename Elem>
    Junction(std::set<Elem> &&container): Store(std::move(container))   { }

    template<typename Iterator>
    Junction(Iterator const begin, Iterator const end): Store(begin, end)   { }

    Junction() = delete;

    ~Junction() { }     // Delete only via subclass, so that we don't need a virtual dtor

    template<typename Subclass, typename Lambda>
    Subclass Map(Lambda const &lambda) const {
        using ResultElement = decltype(lambda(*Store::Elements().begin()));
        std::set<ResultElement> new_elements;
        for (Element const &elem: Store::Elements())
            new_elements.insert(lambda(elem));

        Subclass result(std::move(new_elements));
        return result;
    }
};

template<typename Store, bool MustInvert>
class AnyOrNone;

template<typename Store>
using Any = AnyOrNone<Store, false>;

template<typename Store>
using None = AnyOrNone<Store, true>;

template<typename Store>
class All;

template<typename Store>
class One;

// Every junction can report its own type:
enum class JunctionType {None, One, Any, All};

}

#endif
