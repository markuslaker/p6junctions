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

