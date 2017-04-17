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

#if !defined P6JunctionSortedStore_h
#define      P6JunctionSortedStore_h

// Stores a Junction's elements in a std::set, which is guaranteed to hold them
// in ascending order, enabling optimisations for some comparisons.

#include <cassert>
#include <set>

namespace P6 { namespace Details {

template<typename T>
class JunctionSortedStore {
public:
    using Element             = T;
    static bool const Ordered = true;

private:
    std::set<Element> elements;
    bool moved = false;

public:
    JunctionSortedStore(std::initializer_list<Element> const ilist)
        : elements(ilist)   { }

    template<typename Iterator>
    JunctionSortedStore(Iterator const begin, Iterator const end)
        : elements(begin, end)   { }

    JunctionSortedStore(std::set<Element> &&elements)
        : elements(elements),
          moved(true)   { }

    std::set<Element> const &Elements() const {
        return elements;
    }

    bool IsEmpty() const {
        return elements.empty();
    }

    auto GetSize() const {
        return elements.size();
    }

    bool HasSecondElement() const {
        return GetSize() >= 2;
    }

    // For testing:
    bool CalledMoveConstructor() const {
        return moved;
    }

protected:
    Element const &FirstElement() const {
        assert(not IsEmpty());
        return *elements.begin();
    }

    Element const &SecondElement() const {
        assert(HasSecondElement());
        auto it = elements.begin();
        ++it;
        return *it;
    }

    Element const &PenultimateElement() const {
        assert(HasSecondElement());
        auto it = elements.rbegin();
        ++it;
        return *it;
    }

    Element const &LastElement() const {
        assert(not IsEmpty());
        return *elements.rbegin();
    }

    Element const &GetAnyElement() const {
        return FirstElement();
    }
};

} }

#endif

