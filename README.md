# What is this madness?

It's the start of a port to C++ of Perl 6's junctions.  Using this library, you can say thing like this:

    void foo(int x, int y, int z) {
        assert(all({x, y, z}) <= Limit);
        // ....
    }

Alternatively:

    void bar(int x, int y, int z) {
        if (any({x, y, z}) > Limit)
            // Badness ....
    }

You can even compare one junction with another:

    void baz(int x, int y, int z) {
        if (any({x, y, z}) > any({Limit1, Limit2, Limit3}))
            // Badness ....
    }

This isn't done with cutesy syntactic tricks: `all()` and `any()` (as well as `none()` and `one()`) return real objects, which you can store and reuse:

    void quux(int width, int height, int depth) {
        auto all_dimensions = all({width, height, depth});
        assert(all_dimensions >= 0 && all_dimensions <= Maximum);
        // ....
    }

You can also pass STL containers, such as `std::vector`s and `std::deque`s:

    void snodge(std::vector<int> &vec) {
        auto all_items = all(vec);
        assert(all_items >= 0 && all_items <= Limit);
        // ....
    }

If what you have is a pair of iterators, you can use that:

    template<typename Iter>
    void grollis(Iter begin, Iter end) {
        assert(all(begin, end) != 0);
        // ....
    }

As samples.cpp shows, you can apply a lambda to any junction and get a modified copy with the same category (none, one, any, all) but new values, potentially of a different type; for example, it demonstrates mapping a junction of strings to a new junction of string lengths.

# Memory management

If a junction helper function -- `none()`, `one()`, `any()` or `all()` -- receives an rvalue reference, it'll assume it's been passed a temporary object, and it'll copy all the elements into a `std::set`.  That's why the definition of `all_dimensions` above is safe: the list inside the braces produces a temporary `std::initializer_list<int>`, which disappears at the end of the statement, but `all()` copies the elements so that the resulting object is safe to use.  It does this by delegating to `all_copy()`.

Look back to the definition of `foo()` above.  The `std::initializer_list<int>` passed to all() is a temporary object, but so (as it turns out) is the object returned by `all()`.  This means the copy is unnecessary, because the `std::initializer_list<int>` lives as long as the junction needs it to.  Unfortunately, `all()` can't tell that it's constructing a temporary object.  When constructing a temporary, you can safely use `all_ref()`, `any_ref()` and so on: these functions elide the copy that would otherwise take place, and make the constructors run in constant time and space.

Conversely, by default, no copy is made if you pass in an lvalue reference, which normally corresponds to a named object.  In the definition of `snodge()` above, `all()` makes no copy of the data.  This is efficient, but causes a subtle dependency: if the value of `vec` changes, `all_items` will change correspondingly.  If you wish, you can force a copy to be made by calling `all_copy()`, `any_copy()` and so on.

If a copy is made, some operations are optimised, and go from linear time to constant time.  For example, if the number of elements in a vector is `N` and `Limit` is a simple scalar, the expression

    any(vec) > Limit

takes O(N) time if no copy is made, because the junction can't assume sortedness and it has to scan every element.  If a copy is made, only the last (highest) element need be inspected, and the expression runs in constant time.

Currently, constructing a junction from a pair of iterators always causes a copy to be made.  This needs to change.  We also need to recognise a `std::set` as being sorted and take advantage of its sortedness without needing to copy it.  Currently, we don't.

# Status

Brand new, alpha code, proof of concept, subject to change, not for use in production.  It doesn't even have a makefile yet.  To compile the test-bed with g++ on Linux:

    g++ --std=c++14 -Wall -O2 -lpthread testbed.cpp

samples.cpp is a good place to start looking, because, as with so many libraries, this one is easier to use than it is to read.  To compile samples.cpp with g++ on Linux:

    g++ --std=c++14 -Wall -O2 samples.cpp

# Future directions

It would be good to add further operators to junctions, as in

    if (any(items) & 0xFF)
        std::cout << "Not page-aligned\n";

Some optimisations are needed, as described above.

Just as we can already apply a lambda, it would be syntactically sweeter if we could apply a function or a method to each element of a junction.

If a lambda, function or method returned `bool`, we could return something that collapsed to `bool` on demand, so that you could say something like this:

    bool is_beyond_the_pale(int);
    // ....
    if (any(vec) (is_beyond_the_pale) .Test())
        return E_TOO_BAD;

The `.Test()` method is there because allowing a junction to convert implicitly to `bool` seems like a bad idea.  (Maybe this whole library is a bad idea.  Arguments on a postcard, please.)
