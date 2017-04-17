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

#if !defined P6JunctionPiggyBackStore_h
#define      P6JunctionPiggyBackStore_h

// Stores a Junction's elements by reference to a container passed in by the
// caller, without needing to copy them into storage of its own.

namespace P6 { namespace Details {

template<typename Container>
class JunctionPiggyBackStore {
public:
    using Element             = typename Container::value_type;
    static bool const Ordered = false;

private:
    Container const &container;

    // std::initializer_list<T> lacks an empty() method, but, for other
    // containers, calling empty() is faster than (size() == 0).  Optimally
    // determine whether a container is empty:

    template<typename Element>
    static bool IsEmpty(std::initializer_list<Element> const &container) {
        return container.size() == 0;
    }

    template<typename Cont>
    static bool IsEmpty(Cont const &container) {
        return container.empty();
    }

protected:
    JunctionPiggyBackStore(Container const &container)
        : container(container)   { }

    Element const GetAnyElement() const {
        return *container.front();
    }

public:
    Container const &Elements() const {
        return container;
    }

    bool IsEmpty() const {
        return IsEmpty(container);
    }
};

} }

#endif

