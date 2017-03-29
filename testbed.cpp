#include "JunctionAll.h"
#include "JunctionAny.h"
#include "JunctionOne.h"

#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

// To use threads with g++, add -lpthreads to the build command line:
#define USE_THREADS 1

namespace P6 {

// This testbed is multi-threaded, and so we need to synchronise writes to
// std::cout.  This little class lets callers do that with a minimum of fuss.
// A call to Outputter() creates a temporary object that has an operator << that
// delegates to std::cout, but which also acquires a mutex and holds it until
// the end of the evaulation of the expression that uses it -- which is exactly
// what you want if you do all your outputting in a single statement.  Callers
// that use more than one statement should create an Outputter object and use it
// for all their output, being sure to destroy it promptly.

class Outputter {
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock {mtx};

public:
    Outputter();
    ~Outputter();

    template<typename T>
    std::ostream &operator << (T const &val) {
        return std::cout << val;
    }
};

std::mutex Outputter::mtx;

// Out-of-line the locking and dropping of the mutex:
Outputter::Outputter()  { }
Outputter::~Outputter() { }

// As JunctionAny.h explains, a junction can be created in two states: one that
// copies its arguments into a sorted form and optimises certain operations, and
// one that omits the copy but forgoes certain optimisations.  This first set of
// tests verifies that junctions are created in the right form, given each
// possible kind of argument:

std::vector<int> make_vector();                 // Unimplemented
std::vector<int> const make_const_vector();     // Unimplemented

std::set<int> make_set();                       // Unimplemented
std::set<int> const make_const_set();           // Unimplemented

static void check(bool const ok, char const *const test_name) {
    if (not ok)
        Outputter() << "Test failed: orderedness: " << test_name << '\n';
}

void check_creation_types_none() {
    auto ilist {1, 2, 3};
    auto const cilist {4, 5, 6};

    check(    decltype(none(     {1, 2, 3}))     ::Ordered, "none({1, 2, 3})");
    check(not decltype(none(     ilist))         ::Ordered, "none(ilist)");
    check(not decltype(none(     cilist))        ::Ordered, "none(cilist)");
    check(not decltype(none_ref( {1, 2, 3}))     ::Ordered, "none_ref({1, 2, 3})");
    check(not decltype(none_ref( ilist))         ::Ordered, "none_ref(ilist)");
    check(not decltype(none_ref( cilist))        ::Ordered, "none_ref(cilist)");
    check(    decltype(none_copy({1, 2, 3}))     ::Ordered, "none_copy({1, 2, 3})");
    check(    decltype(none_copy(ilist))         ::Ordered, "none_copy(ilist)");

    std::vector<int> v {1, 2, 3};
    std::vector<int> const cv {v};

    check(not decltype(none(v))                  ::Ordered, "none(v)");
    check(not decltype(none(cv))                 ::Ordered, "none(cv)");
    check(    decltype(none(make_vector()))      ::Ordered, "none(make_vector())");
    check(    decltype(none(make_const_vector()))::Ordered, "none(make_const_vector())");
    check(not decltype(none_ref(v))              ::Ordered, "none_ref(v)");
    check(not decltype(none_ref(cv))             ::Ordered, "none_ref(cv)");
    check(    decltype(none_copy(v))             ::Ordered, "none_copy(v)");
    check(    decltype(none_copy(cv))            ::Ordered, "none_copy(cv)");

    check(    decltype(none(v.begin(), v.end())) ::Ordered, "none(v.begin(), v.end())");

    std::set<int> s {v.begin(), v.end()};
    std::set<int> const cs {s};
    check(not decltype(none(s))                  ::Ordered, "none(s)");
    check(not decltype(none(cs))                 ::Ordered, "none(cs)");
    check(    decltype(none(make_set()))         ::Ordered, "none(make_set())");
    check(    decltype(none(make_const_set()))   ::Ordered, "none(make_const_set())");
}

void check_creation_types_one() {
    auto ilist {1, 2, 3};
    auto const cilist {4, 5, 6};

    check(    decltype(one(     {1, 2, 3}))     ::Ordered, "one({1, 2, 3})");
    check(not decltype(one(     ilist))         ::Ordered, "one(ilist)");
    check(not decltype(one(     cilist))        ::Ordered, "one(cilist)");
    check(not decltype(one_ref( {1, 2, 3}))     ::Ordered, "one_ref({1, 2, 3})");
    check(not decltype(one_ref( ilist))         ::Ordered, "one_ref(ilist)");
    check(not decltype(one_ref( cilist))        ::Ordered, "one_ref(cilist)");
    check(    decltype(one_copy({1, 2, 3}))     ::Ordered, "one_copy({1, 2, 3})");
    check(    decltype(one_copy(ilist))         ::Ordered, "one_copy(ilist)");

    std::vector<int> v {1, 2, 3};
    std::vector<int> const cv {v};

    check(not decltype(one(v))                  ::Ordered, "one(v)");
    check(not decltype(one(cv))                 ::Ordered, "one(cv)");
    check(    decltype(one(make_vector()))      ::Ordered, "one(make_vector())");
    check(    decltype(one(make_const_vector()))::Ordered, "one(make_const_vector())");
    check(not decltype(one_ref(v))              ::Ordered, "one_ref(v)");
    check(not decltype(one_ref(cv))             ::Ordered, "one_ref(cv)");
    check(    decltype(one_copy(v))             ::Ordered, "one_copy(v)");
    check(    decltype(one_copy(cv))            ::Ordered, "one_copy(cv)");

    check(    decltype(one(v.begin(), v.end())) ::Ordered, "one(v.begin(), v.end())");

    std::set<int> s {v.begin(), v.end()};
    std::set<int> const cs {s};
    check(not decltype(one(s))                  ::Ordered, "one(s)");
    check(not decltype(one(cs))                 ::Ordered, "one(cs)");
    check(    decltype(one(make_set()))         ::Ordered, "one(make_set())");
    check(    decltype(one(make_const_set()))   ::Ordered, "one(make_const_set())");
}

void check_creation_types_any() {
    auto ilist {1, 2, 3};
    auto const cilist {4, 5, 6};

    check(    decltype(any(     {1, 2, 3}))     ::Ordered, "any({1, 2, 3})");
    check(not decltype(any(     ilist))         ::Ordered, "any(ilist)");
    check(not decltype(any(     cilist))        ::Ordered, "any(cilist)");
    check(not decltype(any_ref( {1, 2, 3}))     ::Ordered, "any_ref({1, 2, 3})");
    check(not decltype(any_ref( ilist))         ::Ordered, "any_ref(ilist)");
    check(not decltype(any_ref( cilist))        ::Ordered, "any_ref(cilist)");
    check(    decltype(any_copy({1, 2, 3}))     ::Ordered, "any_copy({1, 2, 3})");
    check(    decltype(any_copy(ilist))         ::Ordered, "any_copy(ilist)");

    std::vector<int> v {1, 2, 3};
    std::vector<int> const cv {v};

    check(not decltype(any(v))                  ::Ordered, "any(v)");
    check(not decltype(any(cv))                 ::Ordered, "any(cv)");
    check(    decltype(any(make_vector()))      ::Ordered, "any(make_vector())");
    check(    decltype(any(make_const_vector()))::Ordered, "any(make_const_vector())");
    check(not decltype(any_ref(v))              ::Ordered, "any_ref(v)");
    check(not decltype(any_ref(cv))             ::Ordered, "any_ref(cv)");
    check(    decltype(any_copy(v))             ::Ordered, "any_copy(v)");
    check(    decltype(any_copy(cv))            ::Ordered, "any_copy(cv)");

    check(    decltype(any(v.begin(), v.end())) ::Ordered, "any(v.begin(), v.end())");

    std::set<int> s {v.begin(), v.end()};
    std::set<int> const cs {s};
    check(not decltype(any(s))                  ::Ordered, "any(s)");
    check(not decltype(any(cs))                 ::Ordered, "any(cs)");
    check(    decltype(any(make_set()))         ::Ordered, "any(make_set())");
    check(    decltype(any(make_const_set()))   ::Ordered, "any(make_const_set())");
}

void check_creation_types_all() {
    auto ilist {1, 2, 3};
    auto const cilist {4, 5, 6};

    check(    decltype(all(     {1, 2, 3}))     ::Ordered, "all({1, 2, 3})");
    check(not decltype(all(     ilist))         ::Ordered, "all(ilist)");
    check(not decltype(all(     cilist))        ::Ordered, "all(cilist)");
    check(not decltype(all_ref( {1, 2, 3}))     ::Ordered, "all_ref({1, 2, 3})");
    check(not decltype(all_ref( ilist))         ::Ordered, "all_ref(ilist)");
    check(not decltype(all_ref( cilist))        ::Ordered, "all_ref(cilist)");
    check(    decltype(all_copy({1, 2, 3}))     ::Ordered, "all_copy({1, 2, 3})");
    check(    decltype(all_copy(ilist))         ::Ordered, "all_copy(ilist)");

    std::vector<int> v {1, 2, 3};
    std::vector<int> const cv {v};

    check(not decltype(all(v))                  ::Ordered, "all(v)");
    check(not decltype(all(cv))                 ::Ordered, "all(cv)");
    check(    decltype(all(make_vector()))      ::Ordered, "all(make_vector())");
    check(    decltype(all(make_const_vector()))::Ordered, "all(make_const_vector())");
    check(not decltype(all_ref(v))              ::Ordered, "all_ref(v)");
    check(not decltype(all_ref(cv))             ::Ordered, "all_ref(cv)");
    check(    decltype(all_copy(v))             ::Ordered, "all_copy(v)");
    check(    decltype(all_copy(cv))            ::Ordered, "all_copy(cv)");

    check(    decltype(all(v.begin(), v.end())) ::Ordered, "all(v.begin(), v.end())");

    std::set<int> s {v.begin(), v.end()};
    std::set<int> const cs {s};
    check(not decltype(all(s))                  ::Ordered, "all(s)");
    check(not decltype(all(cs))                 ::Ordered, "all(cs)");
    check(    decltype(all(make_set()))         ::Ordered, "all(make_set())");
    check(    decltype(all(make_const_set()))   ::Ordered, "all(make_const_set())");
}

void check_creation_types() {
    check_creation_types_none();
    check_creation_types_one();
    check_creation_types_any();
    check_creation_types_all();
}

// The six arithmetic (or pseudo-arithmetic) comparisons:

enum class Compare {First, Less = First, LessEq, Equal, NotEqual, GreaterEq, Greater, Last = Greater};

Compare &operator ++ (Compare &cmp) {
    return cmp = static_cast<Compare> (static_cast<int> (cmp) + 1);
}

// Compare elements with elements, junctions with junctions, or a mixture:

template<typename A, typename B>
static bool compare(A const &a, B const &b, Compare const comparison) {
    switch (comparison) {
        case Compare::Less:         return a <  b;
        case Compare::LessEq:       return a <= b;
        case Compare::Equal:        return a == b;
        case Compare::NotEqual:     return a != b;
        case Compare::GreaterEq:    return a >= b;
        case Compare::Greater:      return a >  b;
    }

    assert(false);
    return false;
}

// Conveniently carry four numbers (for comparing a junction to a constant) or
// six numbers (for comparing two junctions):

struct Numbers {
    static auto constexpr Unused = ~0u;
    unsigned a, b, c, d, e {Unused}, f {Unused};

    void SetABC(unsigned const bits) {
        a = (bits >>  0) & 3;
        b = (bits >>  2) & 3;
        c = (bits >>  4) & 3;
    }

    void SetDEF(unsigned const bits) {
        d = (bits >>  6) & 3;
        e = (bits >>  8) & 3;
        f = (bits >> 10) & 3;
    }

    bool HasDuplicatesInABC() const {
        return a == b or b == c or a == c;
    }

    bool HasDuplicatesInDEF() const {
        return d == e or e == f or d == f;
    }
};

std::ostream &operator << (std::ostream &os, Numbers const &nums) {
    os << nums.a << ", " << nums.b << ", " << nums.c << ", " << nums.d;
    if (nums.e != Numbers::Unused)
        os << ", " << nums.e << ", " << nums.f;

    return os;
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//  Comparing junctions to constants                                     //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

// How many numeric matches we expect when comparing a junction to a constant.
//
// TODO: eliminate this enum in favour of calling Junction::GetJunctionType().

enum class MatchCount {None, One, Any, All};

static bool check_match_count(unsigned const nr_matches, MatchCount const required) {
    return required == MatchCount::Any?
        nr_matches != 0:
        nr_matches == static_cast<unsigned> (required);
}

// Report a failure when comparing a junction to a number in either direction:

static void dump_failure(Numbers const &nums, char const *const test_name, Compare const comparison, bool const raw_match, bool const jct_match, bool const reversed) {
    Outputter outputter;
    outputter << "Test failed: " << test_name;
    if (reversed)
        outputter << " (reversed)";

    outputter << ": " << nums
              << ": cmp " << static_cast<unsigned> (comparison)
              << ": raw " << raw_match << ", jct " << jct_match << '\n';
}

// Compare any junction against a constant in both directions; report a test
// failure if the comparison yields the wrong result.  `Untouched' means `not
// bumped', where to bump is to apply a lambda that adds one to every element.

template<typename Junction>
static void compare_untouched_against_constant(Junction const &junction, Numbers const &nums, MatchCount match_count, char const *const test_name) {
    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {

        // Junction op constant:

        auto nr_matched = compare(nums.a, nums.d, comparison)
                        + compare(nums.b, nums.d, comparison)
                        + compare(nums.c, nums.d, comparison);

        auto raw_match = check_match_count(nr_matched, match_count);
        auto jct_match = compare(junction, nums.d, comparison);

        if (raw_match != jct_match)
            dump_failure(nums, test_name, comparison, raw_match, jct_match, false);

        // Constant op junction:

        nr_matched = compare(nums.d, nums.a, comparison)
                   + compare(nums.d, nums.b, comparison)
                   + compare(nums.d, nums.c, comparison);

        raw_match = check_match_count(nr_matched, match_count);
        jct_match = compare(nums.d, junction, comparison);

        if (raw_match != jct_match)
            dump_failure(nums, test_name, comparison, raw_match, jct_match, true);
    }
}

// Bump a junction by applying a lambda that makes a copy whose elements are all
// one higher than those of the original junction; then compare it with a
// constant and report a test failure if the result is wrong.

template<typename Junction>
static void compare_bumped_against_constant(Junction const &untouched_junction, Numbers const &nums, MatchCount match_count, char const *const test_name) {
    auto junction = untouched_junction([] (auto n) {return n + 1; });

    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {

        // Junction op constant:

        auto nr_matched = compare(nums.a + 1, nums.d, comparison)
                        + compare(nums.b + 1, nums.d, comparison)
                        + compare(nums.c + 1, nums.d, comparison);

        auto raw_match = check_match_count(nr_matched, match_count);
        auto jct_match = compare(junction, nums.d, comparison);

        if (raw_match != jct_match)
            dump_failure(nums, test_name, comparison, raw_match, jct_match, false);

        // Constant op junction:

        nr_matched = compare(nums.d, nums.a + 1, comparison)
                   + compare(nums.d, nums.b + 1, comparison)
                   + compare(nums.d, nums.c + 1, comparison);

        raw_match = check_match_count(nr_matched, match_count);
        jct_match = compare(nums.d, junction, comparison);

        if (raw_match != jct_match)
            dump_failure(nums, test_name, comparison, raw_match, jct_match, true);
    }
}

// Compare a junction with a constant in both directions, and with and without
// bumping it.

template<typename Junction>
static void compare_against_constant(Junction const &junction, Numbers const &nums, MatchCount match_count, char const *const test_name) {
    compare_untouched_against_constant(junction, nums, match_count, test_name);
    compare_bumped_against_constant(   junction, nums, match_count, test_name);
}

// Call a lambda on every combination of (a, b, c, d) in [0..3].  Skip
// combinations where (a, b, c) are not all distinct, because junctions
// deduplicate and our manual calculation of how many of (1, 1, 1) == 1
// yields 3, whereas one(1, 1, 1) == 1 correctly yields true.

template<typename Lambda>
static void compare_junction_with_constant(Lambda lambda) {
    for (unsigned bits = 0;  bits <= 0xFF;  ++bits) {
        Numbers nums;
        nums.SetABC(bits);

        // Junctions deduplicate, as sets do:
        if (nums.HasDuplicatesInABC())
            continue;

        nums.d = (bits >> 6) & 3;

        lambda(nums);
    }
}

// Display a junction type:

static std::ostream &operator << (std::ostream &os, JunctionType const type) {
    static char const *const names[] {"None", "One", "Any", "All"};
    constexpr auto NrNames = sizeof names / sizeof *names;

    auto const index = static_cast<unsigned> (type);
    return os << (index < NrNames? names[index]: "[invalid]");
}

// Display a test failure if a comparison of an empty or monadic junction
// against a constant returns the wrong value:

template<typename Junction>
static void check(bool const found, bool const wanted, Junction const &junction, char const *const test_name) {
    if (found == wanted)
        return;

    Outputter() << "Failed test: "
                << (junction.IsEmpty()? "Empty ": "Monadic ")
                << Junction::GetJunctionType() << "-junction "
                << test_name
                << '\n';
}

// Make a specified comparison between a junction and zero in both directions.
// Let the caller tell us which answer to expect, because it varies between
// junction types: (any({}) == 0) == false, because the junction doesn't have
// any elements that equal zero, whereas (all({}) == 0) == true, because every
// element does indeed equal zero, in the sense that there are no elements that
// *don't* equal zero.

template<typename Junction>
static void compare_empty(Junction const &junction, bool const wanted) {
    check(junction <  0u, wanted, junction, "less-than");
    check(junction <= 0u, wanted, junction, "less-equals");
    check(junction == 0u, wanted, junction, "equals");
    check(junction != 0u, wanted, junction, "not-equals");
    check(junction >= 0u, wanted, junction, "greater-equals");
    check(junction >  0u, wanted, junction, "greater-than");

    check(0u <  junction, wanted, junction, "less-than (reversed)");
    check(0u <= junction, wanted, junction, "less-equals (reversed)");
    check(0u == junction, wanted, junction, "equals (reversed)");
    check(0u != junction, wanted, junction, "not-equals (reversed)");
    check(0u >= junction, wanted, junction, "greater-equals (reversed)");
    check(0u >  junction, wanted, junction, "greater-than (reversed)");
}

// Create none-junctions in every possible way, and compare them with zero.

static void compare_none_empty() {
    std::initializer_list<unsigned> init_list { };
    compare_empty(none(init_list), true);
    compare_empty(none_copy(init_list), true);
    compare_empty(none_ref( init_list), true);

    std::vector<unsigned> vec;
    compare_empty(none(vec.begin(), vec.end()), true);
    compare_empty(none(vec), true);
    compare_empty(none_ref(vec), true);
    compare_empty(none_copy(vec), true);
    compare_empty(none(std::move(vec)), true);

    std::vector<unsigned> const cvec;
    compare_empty(none(cvec.begin(), cvec.end()), true);
    compare_empty(none(cvec), true);
    compare_empty(none_ref(cvec), true);
    compare_empty(none_copy(cvec), true);

    std::set<unsigned> set;
    compare_empty(none(set), true);
    compare_empty(none_ref(set), true);
    compare_empty(none_copy(set), true);
    compare_empty(none(set.begin(), set.end()), true);
    compare_empty(none(std::move(set)), true);

    std::set<unsigned> const cset;
    compare_empty(none(cset), true);
    compare_empty(none_ref(cset), true);
    compare_empty(none_copy(cset), true);
    compare_empty(none(cset.begin(), cset.end()), true);
}

// Accept a none-junction containing the single value 1 and compare it against
// a constant every possible way, eliciting both true and false results.

template<typename Junction>
static void compare_none_monadic(Junction const &junction) {
    check(junction <  0u, true,  junction, "less-than, true");
    check(junction <= 0u, true,  junction, "less-equals, true");
    check(junction == 0u, true,  junction, "equals, true");
    check(junction != 1u, true,  junction, "not-equals, true");
    check(junction >= 2u, true,  junction, "greater-equals, true");
    check(junction >  1u, true,  junction, "greater-than, true");

    check(junction <  2u, false, junction, "less-than, false");
    check(junction <= 1u, false, junction, "less-equals, false");
    check(junction == 1u, false, junction, "equals, false");
    check(junction != 0u, false, junction, "not-equals, false");
    check(junction >= 1u, false, junction, "greater-equals, false");
    check(junction >  0u, false, junction, "greater-than, false");

    check(1u <  junction, true,  junction, "less-than (reversed), true");
    check(2u <= junction, true,  junction, "less-equals (reversed), true");
    check(0u == junction, true,  junction, "equals (reversed), true");
    check(1u != junction, true,  junction, "not-equals (reversed), true");
    check(0u >= junction, true,  junction, "greater-equals (reversed), true");
    check(1u >  junction, true,  junction, "greater-than (reversed), true");

    check(0u <  junction, false, junction, "less-than (reversed), false");
    check(1u <= junction, false, junction, "less-equals (reversed), false");
    check(1u == junction, false, junction, "equals (reversed), false");
    check(0u != junction, false, junction, "not-equals (reversed), false");
    check(1u >= junction, false, junction, "greater-equals (reversed), false");
    check(2u >  junction, false, junction, "greater-than (reversed), false");
}

// Create a monadic none-junction every possible way, and compare it against
// constants.

static void compare_none_monadic() {
    std::initializer_list<unsigned> ilist {1u};
    compare_none_monadic(none(ilist));
    compare_none_monadic(none_ref(ilist));
    compare_none_monadic(none_copy(ilist));

    std::vector<unsigned> vec {1u};
    std::vector<unsigned> const cvec {vec};
    compare_none_monadic(none(vec.begin(), vec.end()));
    compare_none_monadic(none(vec));
    compare_none_monadic(none_ref(vec));
    compare_none_monadic(none_copy(vec));
    compare_none_monadic(none(std::move(vec)));
    compare_none_monadic(none(cvec.begin(), cvec.end()));
    compare_none_monadic(none(cvec));
    compare_none_monadic(none_ref(cvec));
    compare_none_monadic(none_copy(cvec));

    std::set<unsigned> set {1u};
    std::set<unsigned> cset {set};
    compare_none_monadic(none(set.begin(), set.end()));
    compare_none_monadic(none(set));
    compare_none_monadic(none_copy(set));
    compare_none_monadic(none_ref(set));
    compare_none_monadic(none(std::move(set)));
    compare_none_monadic(none(cset.begin(), cset.end()));
    compare_none_monadic(none(cset));
    compare_none_monadic(none_copy(cset));
    compare_none_monadic(none_ref(cset));
}

// Create a triadic none-junction every possible way, and compare it against
// constants.

static void compare_none_triadic() {
    compare_junction_with_constant([] (Numbers const &nums) {
        std::initializer_list<unsigned> ilist {nums.a, nums.b, nums.c};
        compare_against_constant(none(ilist), nums, MatchCount::None, "none (init list) against constant");
        compare_against_constant(none_copy(ilist), nums, MatchCount::None, "none_copy (init list) against constant");
        compare_against_constant(none_ref(ilist), nums, MatchCount::None, "none_ref (init list) against constant");
        compare_against_constant(none(std::move(ilist)), nums, MatchCount::None, "none (move init list) against constant");

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        std::vector<unsigned> const cvec {vec};
        compare_against_constant(none(vec), nums, MatchCount::None, "none (vector) against constant");
        compare_against_constant(none_copy(vec), nums, MatchCount::None, "none_copy (vector) against constant");
        compare_against_constant(none_ref(vec), nums, MatchCount::None, "none_ref (vector) against constant");
        compare_against_constant(none(vec.begin(), vec.end()), nums, MatchCount::None, "none (vector iterators) against constant");
        compare_against_constant(none(std::move(vec)), nums, MatchCount::None, "none (move vector) against constant");
        compare_against_constant(none(cvec), nums, MatchCount::None, "none (const vector) against constant");
        compare_against_constant(none_copy(cvec), nums, MatchCount::None, "none_copy (const vector) against constant");
        compare_against_constant(none_ref(cvec), nums, MatchCount::None, "none_ref (const vector) against constant");
        compare_against_constant(none(cvec.begin(), cvec.end()), nums, MatchCount::None, "none (const vector iterators) against constant");

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        std::set<unsigned> const cset {set};
        compare_against_constant(none(set), nums, MatchCount::None, "none (set) against constant");
        compare_against_constant(none_copy(set), nums, MatchCount::None, "none_copy (set) against constant");
        compare_against_constant(none_ref(set), nums, MatchCount::None, "none_ref (set) against constant");
        compare_against_constant(none(set.begin(), set.end()), nums, MatchCount::None, "none_ref (set iterators) against constant");
        compare_against_constant(none(std::move(set)), nums, MatchCount::None, "none (move set) against constant");
        compare_against_constant(none(cset), nums, MatchCount::None, "none (const set) against constant");
        compare_against_constant(none_copy(cset), nums, MatchCount::None, "none_copy (const set) against constant");
        compare_against_constant(none_ref(cset), nums, MatchCount::None, "none_ref (const set) against constant");
        compare_against_constant(none(cset.begin(), cset.end()), nums, MatchCount::None, "none (const set iterators) against constant");
    });
}

// Create empty, monadic and triadic none-junctions every possible way, and
// compare them with constants.

static void compare_none_with_constant() {
    compare_none_empty();
    compare_none_monadic();
    compare_none_triadic();
}

// Create an empty one-junction every possible way, and compare it against a
// constant.

static void compare_one_empty() {
    std::initializer_list<unsigned> init_list { };
    compare_empty(one(init_list), false);
    compare_empty(one_copy(init_list), false);
    compare_empty(one_ref( init_list), false);

    std::vector<unsigned> vec;
    compare_empty(one(vec.begin(), vec.end()), false);
    compare_empty(one(vec), false);
    compare_empty(one_ref(vec), false);
    compare_empty(one_copy(vec), false);
    compare_empty(one(std::move(vec)), false);

    std::vector<unsigned> const cvec;
    compare_empty(one(cvec.begin(), cvec.end()), false);
    compare_empty(one(cvec), false);
    compare_empty(one_ref(cvec), false);
    compare_empty(one_copy(cvec), false);

    std::set<unsigned> set;
    compare_empty(one(set.begin(), set.end()), false);
    compare_empty(one(set), false);
    compare_empty(one_ref(set), false);
    compare_empty(one_copy(set), false);
    compare_empty(one(std::move(set)), false);

    std::set<unsigned> const cset;
    compare_empty(one(cset), false);
    compare_empty(one_ref(cset), false);
    compare_empty(one_copy(cset), false);
    compare_empty(one(cset.begin(), cset.end()), false);
}

// Accept an uninverted junction -- anything except a none-junction.  Perform
// all six comparisons against constants, eliciting true, and six more eliciting
// false, and then do it all again with the operands reversed.  Report test
// failures if any comparison returns the wrong result.

template<typename Junction>
static void compare_uninverted_junction_monadic(Junction const &junction) {
    // `junction' contains a single element, which is 1
    check(junction <  2u, true,  junction, "less-than, true");
    check(junction <= 1u, true,  junction, "less-equals, true");
    check(junction == 1u, true,  junction, "equals, true");
    check(junction != 0u, true,  junction, "not-equals, true");
    check(junction >= 1u, true,  junction, "greater-equals, true");
    check(junction >  0u, true,  junction, "greater-than, true");

    check(junction <  1u, false, junction, "less-than, false");
    check(junction <= 0u, false, junction, "less-equals, false");
    check(junction == 0u, false, junction, "equals, false");
    check(junction != 1u, false, junction, "not-equals, false");
    check(junction >= 2u, false, junction, "greater-equals, false");
    check(junction >  1u, false, junction, "greater-than, false");

    check(0u <  junction, true,  junction, "less-than (reversed), true");
    check(1u <= junction, true,  junction, "less-equals (reversed), true");
    check(1u == junction, true,  junction, "equals (reversed), true");
    check(0u != junction, true,  junction, "not-equals (reversed), true");
    check(1u >= junction, true,  junction, "greater-equals (reversed), true");
    check(2u >  junction, true,  junction, "greater-than (reversed), true");

    check(1u <  junction, false, junction, "less-than (reversed), false");
    check(2u <= junction, false, junction, "less-equals (reversed), false");
    check(0u == junction, false, junction, "equals (reversed), false");
    check(1u != junction, false, junction, "not-equals (reversed), false");
    check(0u >= junction, false, junction, "greater-equals (reversed), false");
    check(1u >  junction, false, junction, "greater-than (reversed), false");
}

// Create a monadic one-junction every possible way, and compare it against
// constants.

static void compare_one_monadic() {
    std::initializer_list<unsigned> ilist {1u};
    compare_uninverted_junction_monadic(one(ilist));
    compare_uninverted_junction_monadic(one_ref(ilist));
    compare_uninverted_junction_monadic(one_copy(ilist));

    std::vector<unsigned> vec {1u};
    std::vector<unsigned> const cvec {vec};
    compare_uninverted_junction_monadic(one(vec.begin(), vec.end()));
    compare_uninverted_junction_monadic(one(vec));
    compare_uninverted_junction_monadic(one_ref(vec));
    compare_uninverted_junction_monadic(one_copy(vec));
    compare_uninverted_junction_monadic(one(std::move(vec)));
    compare_uninverted_junction_monadic(one(cvec.begin(), cvec.end()));
    compare_uninverted_junction_monadic(one(cvec));
    compare_uninverted_junction_monadic(one_ref(cvec));
    compare_uninverted_junction_monadic(one_copy(cvec));

    std::set<unsigned> set {1u};
    std::set<unsigned> const cset {set};
    compare_uninverted_junction_monadic(one(set.begin(), set.end()));
    compare_uninverted_junction_monadic(one(set));
    compare_uninverted_junction_monadic(one_copy(set));
    compare_uninverted_junction_monadic(one_ref(set));
    compare_uninverted_junction_monadic(one(std::move(set)));
    compare_uninverted_junction_monadic(one(cset.begin(), cset.end()));
    compare_uninverted_junction_monadic(one(cset));
    compare_uninverted_junction_monadic(one_copy(cset));
    compare_uninverted_junction_monadic(one_ref(cset));
}

// Create a triadic one-junction every possible way, and compare it against
// constants.

static void compare_one_triadic() {
    compare_junction_with_constant([] (Numbers const &nums) {
        std::initializer_list<unsigned> ilist {nums.a, nums.b, nums.c};
        compare_against_constant(one(ilist), nums, MatchCount::One, "one (init list) against constant");
        compare_against_constant(one_copy(ilist), nums, MatchCount::One, "one_copy (init list) against constant");
        compare_against_constant(one_ref(ilist), nums, MatchCount::One, "one_ref (init list) against constant");
        compare_against_constant(one(std::move(ilist)), nums, MatchCount::One, "one (move init list) against constant");

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        std::vector<unsigned> const cvec {vec};
        compare_against_constant(one(vec), nums, MatchCount::One, "one (vector) against constant");
        compare_against_constant(one_copy(vec), nums, MatchCount::One, "one_copy (vector) against constant");
        compare_against_constant(one_ref(vec), nums, MatchCount::One, "one_ref (vector) against constant");
        compare_against_constant(one(vec.begin(), vec.end()), nums, MatchCount::One, "one (vector iterators) against constant");
        compare_against_constant(one(std::move(vec)), nums, MatchCount::One, "one (move vector) against constant");
        compare_against_constant(one(cvec), nums, MatchCount::One, "one (const vector) against constant");
        compare_against_constant(one_copy(cvec), nums, MatchCount::One, "one_copy (const vector) against constant");
        compare_against_constant(one_ref(cvec), nums, MatchCount::One, "one_ref (const vector) against constant");
        compare_against_constant(one(cvec.begin(), cvec.end()), nums, MatchCount::One, "one (const vector iterators) against constant");

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        std::set<unsigned> const cset {set};
        compare_against_constant(one(set), nums, MatchCount::One, "one (set) against constant");
        compare_against_constant(one_copy(set), nums, MatchCount::One, "one_copy (set) against constant");
        compare_against_constant(one_ref(set), nums, MatchCount::One, "one_ref (set) against constant");
        compare_against_constant(one(set.begin(), set.end()), nums, MatchCount::One, "one_ref (set iterators) against constant");
        compare_against_constant(one(std::move(set)), nums, MatchCount::One, "one (move set) against constant");
        compare_against_constant(one(cset), nums, MatchCount::One, "one (const set) against constant");
        compare_against_constant(one_copy(cset), nums, MatchCount::One, "one_copy (const set) against constant");
        compare_against_constant(one_ref(cset), nums, MatchCount::One, "one_ref (const set) against constant");
        compare_against_constant(one(cset.begin(), cset.end()), nums, MatchCount::One, "one (const set iterators) against constant");
    });
}

// Create empty, monadic and triadic one-junctions every possible way, and
// compare them with constants.

static void compare_one_with_constant() {
    compare_one_empty();
    compare_one_monadic();
    compare_one_triadic();
}

// Create an empty any-junction every possible way, and compare it against a
// constant.

static void compare_any_empty() {
    std::initializer_list<unsigned> init_list { };
    compare_empty(any(init_list), false);
    compare_empty(any_copy(init_list), false);
    compare_empty(any_ref( init_list), false);

    std::vector<unsigned> vec;
    compare_empty(any(vec.begin(), vec.end()), false);
    compare_empty(any(vec), false);
    compare_empty(any_ref(vec), false);
    compare_empty(any_copy(vec), false);
    compare_empty(any(std::move(vec)), false);

    std::vector<unsigned> const cvec;
    compare_empty(any(cvec.begin(), cvec.end()), false);
    compare_empty(any(cvec), false);
    compare_empty(any_ref(cvec), false);
    compare_empty(any_copy(cvec), false);

    std::set<unsigned> set;
    compare_empty(any(set.begin(), set.end()), false);
    compare_empty(any(set), false);
    compare_empty(any_ref(set), false);
    compare_empty(any_copy(set), false);
    compare_empty(any(std::move(set)), false);

    std::set<unsigned> const cset;
    compare_empty(any(cset), false);
    compare_empty(any_ref(cset), false);
    compare_empty(any_copy(cset), false);
    compare_empty(any(cset.begin(), cset.end()), false);
}

// Create a monadic any-junction every possible way, and compare it against
// constants.

static void compare_any_monadic() {
    std::initializer_list<unsigned> ilist {1u};
    compare_uninverted_junction_monadic(any(ilist));
    compare_uninverted_junction_monadic(any_ref(ilist));
    compare_uninverted_junction_monadic(any_copy(ilist));

    std::vector<unsigned> vec {1u};
    std::vector<unsigned> const cvec {vec};
    compare_uninverted_junction_monadic(any(vec.begin(), vec.end()));
    compare_uninverted_junction_monadic(any(vec));
    compare_uninverted_junction_monadic(any_ref(vec));
    compare_uninverted_junction_monadic(any_copy(vec));
    compare_uninverted_junction_monadic(any(std::move(vec)));
    compare_uninverted_junction_monadic(any(cvec.begin(), cvec.end()));
    compare_uninverted_junction_monadic(any(cvec));
    compare_uninverted_junction_monadic(any_ref(cvec));
    compare_uninverted_junction_monadic(any_copy(cvec));

    std::set<unsigned> set {1u};
    std::set<unsigned> const cset {set};
    compare_uninverted_junction_monadic(any(set.begin(), set.end()));
    compare_uninverted_junction_monadic(any(set));
    compare_uninverted_junction_monadic(any_copy(set));
    compare_uninverted_junction_monadic(any_ref(set));
    compare_uninverted_junction_monadic(any(std::move(set)));
    compare_uninverted_junction_monadic(any(cset.begin(), cset.end()));
    compare_uninverted_junction_monadic(any(cset));
    compare_uninverted_junction_monadic(any_copy(cset));
    compare_uninverted_junction_monadic(any_ref(cset));
}

// Create a triadic any-junction every possible way, and compare it against
// constants.

static void compare_any_triadic() {
    compare_junction_with_constant([] (Numbers const &nums) {
        std::initializer_list<unsigned> ilist {nums.a, nums.b, nums.c};
        compare_against_constant(any(ilist), nums, MatchCount::Any, "any (init list) against constant");
        compare_against_constant(any_copy(ilist), nums, MatchCount::Any, "any_copy (init list) against constant");
        compare_against_constant(any_ref(ilist), nums, MatchCount::Any, "any_ref (init list) against constant");
        compare_against_constant(any(std::move(ilist)), nums, MatchCount::Any, "any (move init list) against constant");

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        std::vector<unsigned> const cvec {vec};
        compare_against_constant(any(vec), nums, MatchCount::Any, "any (vector) against constant");
        compare_against_constant(any_copy(vec), nums, MatchCount::Any, "any_copy (vector) against constant");
        compare_against_constant(any_ref(vec), nums, MatchCount::Any, "any_ref (vector) against constant");
        compare_against_constant(any(vec.begin(), vec.end()), nums, MatchCount::Any, "any (vector iterators) against constant");
        compare_against_constant(any(std::move(vec)), nums, MatchCount::Any, "any (move vector) against constant");
        compare_against_constant(any(cvec), nums, MatchCount::Any, "any (const vector) against constant");
        compare_against_constant(any_copy(cvec), nums, MatchCount::Any, "any_copy (const vector) against constant");
        compare_against_constant(any_ref(cvec), nums, MatchCount::Any, "any_ref (const vector) against constant");
        compare_against_constant(any(cvec.begin(), cvec.end()), nums, MatchCount::Any, "any (const vector iterators) against constant");

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        std::set<unsigned> const cset {set};
        compare_against_constant(any(set), nums, MatchCount::Any, "any (set) against constant");
        compare_against_constant(any_copy(set), nums, MatchCount::Any, "any_copy (set) against constant");
        compare_against_constant(any_ref(set), nums, MatchCount::Any, "any_ref (set) against constant");
        compare_against_constant(any(set.begin(), set.end()), nums, MatchCount::Any, "any_ref (set iterators) against constant");
        compare_against_constant(any(std::move(set)), nums, MatchCount::Any, "any (move set) against constant");
        compare_against_constant(any(cset), nums, MatchCount::Any, "any (const set) against constant");
        compare_against_constant(any_copy(cset), nums, MatchCount::Any, "any_copy (const set) against constant");
        compare_against_constant(any_ref(cset), nums, MatchCount::Any, "any_ref (const set) against constant");
        compare_against_constant(any(cset.begin(), cset.end()), nums, MatchCount::Any, "any (const set iterators) against constant");
    });
}

// Create empty, monadic and triadic any-junctions every possible way, and
// compare them with constants.

static void compare_any_with_constant() {
    compare_any_empty();
    compare_any_monadic();
    compare_any_triadic();
}

// Create an empty all-junction every possible way, and compare it against a
// constant.

static void compare_all_empty() {
    std::initializer_list<unsigned> init_list { };
    compare_empty(all(init_list), true);
    compare_empty(all_copy(init_list), true);
    compare_empty(all_ref( init_list), true);

    std::vector<unsigned> vec;
    compare_empty(all(vec.begin(), vec.end()), true);
    compare_empty(all(vec), true);
    compare_empty(all_ref(vec), true);
    compare_empty(all_copy(vec), true);
    compare_empty(all(std::move(vec)), true);

    std::vector<unsigned> const cvec;
    compare_empty(all(cvec.begin(), cvec.end()), true);
    compare_empty(all(cvec), true);
    compare_empty(all_ref(cvec), true);
    compare_empty(all_copy(cvec), true);

    std::set<unsigned> set;
    compare_empty(all(set.begin(), set.end()), true);
    compare_empty(all(set), true);
    compare_empty(all_ref(set), true);
    compare_empty(all_copy(set), true);
    compare_empty(all(std::move(set)), true);

    std::set<unsigned> const cset;
    compare_empty(all(cset), true);
    compare_empty(all_ref(cset), true);
    compare_empty(all_copy(cset), true);
    compare_empty(all(cset.begin(), cset.end()), true);
}

// Create a monadic all-junction every possible way, and compare it against
// constants.

static void compare_all_monadic() {
    std::initializer_list<unsigned> ilist {1u};
    compare_uninverted_junction_monadic(all(ilist));
    compare_uninverted_junction_monadic(all_ref(ilist));
    compare_uninverted_junction_monadic(all_copy(ilist));

    std::vector<unsigned> vec {1u};
    std::vector<unsigned> const cvec {vec};
    compare_uninverted_junction_monadic(all(vec.begin(), vec.end()));
    compare_uninverted_junction_monadic(all(vec));
    compare_uninverted_junction_monadic(all_ref(vec));
    compare_uninverted_junction_monadic(all_copy(vec));
    compare_uninverted_junction_monadic(all(std::move(vec)));
    compare_uninverted_junction_monadic(all(cvec.begin(), cvec.end()));
    compare_uninverted_junction_monadic(all(cvec));
    compare_uninverted_junction_monadic(all_ref(cvec));
    compare_uninverted_junction_monadic(all_copy(cvec));

    std::set<unsigned> set {1u};
    std::set<unsigned> const cset {set};
    compare_uninverted_junction_monadic(all(set.begin(), set.end()));
    compare_uninverted_junction_monadic(all(set));
    compare_uninverted_junction_monadic(all_copy(set));
    compare_uninverted_junction_monadic(all_ref(set));
    compare_uninverted_junction_monadic(all(std::move(set)));
    compare_uninverted_junction_monadic(all(cset.begin(), cset.end()));
    compare_uninverted_junction_monadic(all(cset));
    compare_uninverted_junction_monadic(all_copy(cset));
    compare_uninverted_junction_monadic(all_ref(cset));
}

// Create a triadic any-junction every possible way, and compare it against
// constants.

static void compare_all_triadic() {
    compare_junction_with_constant([] (Numbers const &nums) {
        std::initializer_list<unsigned> ilist {nums.a, nums.b, nums.c};
        compare_against_constant(all(ilist), nums, MatchCount::All, "all (init list) against constant");
        compare_against_constant(all_copy(ilist), nums, MatchCount::All, "all_copy (init list) against constant");
        compare_against_constant(all_ref(ilist), nums, MatchCount::All, "all_ref (init list) against constant");
        compare_against_constant(all(std::move(ilist)), nums, MatchCount::All, "all (move init list) against constant");

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        std::vector<unsigned> const cvec {vec};
        compare_against_constant(all(vec), nums, MatchCount::All, "all (vector) against constant");
        compare_against_constant(all_copy(vec), nums, MatchCount::All, "all_copy (vector) against constant");
        compare_against_constant(all_ref(vec), nums, MatchCount::All, "all_ref (vector) against constant");
        compare_against_constant(all(vec.begin(), vec.end()), nums, MatchCount::All, "all (vector iterators) against constant");
        compare_against_constant(all(std::move(vec)), nums, MatchCount::All, "all (move vector) against constant");
        compare_against_constant(all(cvec), nums, MatchCount::All, "all (const vector) against constant");
        compare_against_constant(all_copy(cvec), nums, MatchCount::All, "all_copy (const vector) against constant");
        compare_against_constant(all_ref(cvec), nums, MatchCount::All, "all_ref (const vector) against constant");
        compare_against_constant(all(cvec.begin(), cvec.end()), nums, MatchCount::All, "all (const vector iterators) against constant");

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        std::set<unsigned> const cset {set};
        compare_against_constant(all(set), nums, MatchCount::All, "all (set) against constant");
        compare_against_constant(all_copy(set), nums, MatchCount::All, "all_copy (set) against constant");
        compare_against_constant(all_ref(set), nums, MatchCount::All, "all_ref (set) against constant");
        compare_against_constant(all(set.begin(), set.end()), nums, MatchCount::All, "all_ref (set iterators) against constant");
        compare_against_constant(all(std::move(set)), nums, MatchCount::All, "all (move set) against constant");
        compare_against_constant(all(cset), nums, MatchCount::All, "all (const set) against constant");
        compare_against_constant(all_copy(cset), nums, MatchCount::All, "all_copy (const set) against constant");
        compare_against_constant(all_ref(cset), nums, MatchCount::All, "all_ref (const set) against constant");
        compare_against_constant(all(cset.begin(), cset.end()), nums, MatchCount::All, "all (const set iterators) against constant");
    });
}

// Create empty, monadic and triadic all-junctions every possible way, and
// compare them with constants.

static void compare_all_with_constant() {
    compare_all_empty();
    compare_all_monadic();
    compare_all_triadic();
}

static void compare_junctions_with_constants() {
    compare_none_with_constant();
    compare_one_with_constant();
    compare_any_with_constant();
    compare_all_with_constant();
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//  Comparing junctions to other junctions                               //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

// Compare a to (d, e, f) and return the number of matches:

static unsigned compare_a_to_def(unsigned const a, unsigned const d, unsigned const e, unsigned const f, Compare const comparison) {
    return compare(a, d, comparison)
         + compare(a, e, comparison)
         + compare(a, f, comparison);
}

// Compare two junctions.  Report a test error if the comparison yields the
// wrong result:

template<typename J, typename K>
static void check(J const &j, K const &k, bool const expected, Numbers const &nums, Compare const comparison) {
    auto const found = compare(j, k, comparison);
    if (found != expected)
        Outputter() << "Test failed: "
                    << J::GetJunctionType() << " against " << K::GetJunctionType()
                    << " on numbers " << nums
                    << ", comparison " << static_cast<int> (comparison)
                    << ", expected " << expected << '\n';
}

//
// Compare none-junctions to other junctions.
//
// In the code that follows, matches_x_to_y() functions perform manual
// comparisons, and match_x_to_y() functions verify junction comparisons against
// those manual comparisons.  Code is presented in the usual order -- none, one,
// any, all -- with the second junction varying fastest.

///

// None to none:
//
// (a, b, c) and (d, e, f) represent two None-junctions, j and k.  Should a
// specified comparison return true?
//
// If none(a, b, c) == none(d, e, f) it means that each of (a, b, c) matches at
// least of (d, e, f).  Therefore:
// none(a, b, c) == none(d, e, f) <=> all(a, b, c) == any(d, e, f)
//
// The latter is a more intuitive condition to reason about, so we'll code up
// that comparison instead.
//
// The same relationship applies to the other five comparison operators.

static bool matches_all_to_any(Numbers const &, Compare);

static bool matches_none_to_none(Numbers const &nums, Compare const comparison) {
    return matches_all_to_any(nums, comparison);
}

template<typename NoneJct>
static void check_none_to_none(NoneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = none({nums.d, nums.e, nums.f});
    auto const expected = matches_none_to_none(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// None to one:

bool matches_none_to_one(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) != 1
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) != 1
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) != 1;
}

template<typename NoneJct>
static void check_none_to_one(NoneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = one({nums.d, nums.e, nums.f});
    auto const expected = matches_none_to_one(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// None to any:

bool matches_none_to_any(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 0
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 0
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 0;
}

template<typename NoneJct>
static void check_none_to_any(NoneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = any({nums.d, nums.e, nums.f});
    auto const expected = matches_none_to_any(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// None to all:

bool matches_none_to_all(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) != 3
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) != 3
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) != 3;
}

template<typename NoneJct>
static void check_none_to_all(NoneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = all({nums.d, nums.e, nums.f});
    auto const expected = matches_none_to_all(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Perform every arithmetic comparison between None and every junction type...

template<typename NoneJct>
static void check_none_to_everything(NoneJct const &j, Numbers const &nums) {
    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {
        check_none_to_none(j, nums, comparison);
        check_none_to_any( j, nums, comparison);
        check_none_to_all( j, nums, comparison);

        if (not nums.HasDuplicatesInDEF())
            check_none_to_one(j, nums, comparison);
    }
}

// ... where None-junctions are constructed both by reference and by copy.
// We've proved above that none() calls either none_copy() or none_ref(), as
// appropriate, as well as the handling of const input, and so we only need to
// test those lower-level functions -- we needn't test the none() overloads all
// over again.

static void check_none_to_every_junction_type() {
    for (auto bits = 0u;  bits <= 0xFFFu;  ++bits) {
        Numbers nums;
        nums.SetABC(bits);
        nums.SetDEF(bits);

        check_none_to_everything(none_copy({nums.a, nums.b, nums.c}), nums);
        check_none_to_everything(none_ref({nums.a, nums.b, nums.c}), nums);

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        check_none_to_everything(none(vec.begin(), vec.end()), nums);
        check_none_to_everything(none_copy(vec), nums);
        check_none_to_everything(none_ref(vec), nums);

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        check_none_to_everything(none_copy(set), nums);
        check_none_to_everything(none_ref(set), nums);
    }
}

// One to none:

static bool matches_one_to_none(Numbers const &nums, Compare const comparison) {
    return !compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison)
         + !compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison)
         + !compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison)
         == 1;
}

template<typename OneJct>
static void check_one_to_none(OneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = none({nums.d, nums.e, nums.f});
    auto const expected = matches_one_to_none(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// One to one:

static bool matches_one_to_one(Numbers const &nums, Compare const comparison) {
    return (compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 1)
         + (compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 1)
         + (compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 1)
         == 1;
}

template<typename OneJct>
static void check_one_to_one(OneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = one({nums.d, nums.e, nums.f});
    auto const expected = matches_one_to_one(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// One to any:

static bool matches_one_to_any(Numbers const &nums, Compare const comparison) {
    return !!compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison)
         + !!compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison)
         + !!compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison)
         == 1;
}

template<typename OneJct>
static void check_one_to_any(OneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = any({nums.d, nums.e, nums.f});
    auto const expected = matches_one_to_any(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// One to all:

static bool matches_one_to_all(Numbers const &nums, Compare const comparison) {
    return (compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 3)
         + (compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 3)
         + (compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 3)
         == 1;
}

template<typename OneJct>
static void check_one_to_all(OneJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = all({nums.d, nums.e, nums.f});
    auto const expected = matches_one_to_all(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Perform every arithmetic comparison between One and every junction type...

template<typename OneJct>
static void check_one_to_everything(OneJct const &j, Numbers const &nums) {
    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {
        check_one_to_none(j, nums, comparison);
        check_one_to_any( j, nums, comparison);
        check_one_to_all( j, nums, comparison);

        if (not nums.HasDuplicatesInDEF())
            check_one_to_one(j, nums, comparison);
    }
}

// ... where One-junctions are constructed both by reference and by copy.
// We've proved above that one() calls either one_copy() or one_ref(), as
// appropriate, as well as the handling of const input, and so we only need to
// test those lower-level functions -- we needn't test the one() overloads all
// over again.

static void check_one_to_every_junction_type() {
    for (auto bits = 0u;  bits <= 0xFFFu;  ++bits) {
        Numbers nums;
        nums.SetABC(bits);
        if (nums.HasDuplicatesInABC())
            continue;

        nums.SetDEF(bits);

        check_one_to_everything(one_copy({nums.a, nums.b, nums.c}), nums);
        check_one_to_everything(one_ref({nums.a, nums.b, nums.c}), nums);

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        check_one_to_everything(one(vec.begin(), vec.end()), nums);
        check_one_to_everything(one_copy(vec), nums);
        check_one_to_everything(one_ref(vec), nums);

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        check_one_to_everything(one_copy(set), nums);
        check_one_to_everything(one_ref(set), nums);
    }
}

// Any to none:

static bool matches_any_to_none(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 0
        or compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 0
        or compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 0;
}

template<typename AnyJct>
static void check_any_to_none(AnyJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = none({nums.d, nums.e, nums.f});
    auto const expected = matches_any_to_none(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Any to one:

static bool matches_any_to_one(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 1
        or compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 1
        or compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 1;
}

template<typename AnyJct>
static void check_any_to_one(AnyJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = one({nums.d, nums.e, nums.f});
    auto const expected = matches_any_to_one(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Any to any:

static bool matches_any_to_any(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) != 0
        or compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) != 0
        or compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) != 0;
}

template<typename AnyJct>
static void check_any_to_any(AnyJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = any({nums.d, nums.e, nums.f});
    auto const expected = matches_any_to_any(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Any to all:

static bool matches_any_to_all(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 3
        or compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 3
        or compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 3;
}

template<typename AnyJct>
static void check_any_to_all(AnyJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = all({nums.d, nums.e, nums.f});
    auto const expected = matches_any_to_all(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Perform every arithmetic comparison between Any and every junction type...

template<typename AnyJct>
static void check_any_to_everything(AnyJct const &j, Numbers const &nums) {
    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {
        check_any_to_none(j, nums, comparison);
        check_any_to_any( j, nums, comparison);
        check_any_to_all( j, nums, comparison);

        if (not nums.HasDuplicatesInDEF())
            check_any_to_one(j, nums, comparison);
    }
}

// ... where Any-junctions are constructed both by reference and by copy.
// We've proved above that any() calls either any_copy() or any_ref(), as
// appropriate, as well as the handling of const input, and so we only need to
// test those lower-level functions -- we needn't test the any() overloads all
// over again.

static void check_any_to_every_junction_type() {
    for (auto bits = 0u;  bits <= 0xFFFu;  ++bits) {
        Numbers nums;
        nums.SetABC(bits);
        nums.SetDEF(bits);

        check_any_to_everything(any_copy({nums.a, nums.b, nums.c}), nums);
        check_any_to_everything(any_ref({nums.a, nums.b, nums.c}), nums);

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        check_any_to_everything(any(vec.begin(), vec.end()), nums);
        check_any_to_everything(any_copy(vec), nums);
        check_any_to_everything(any_ref(vec), nums);

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        check_any_to_everything(any_copy(set), nums);
        check_any_to_everything(any_ref(set), nums);
    }
}

// All to none:

static bool matches_all_to_none(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 0
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 0
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 0;
}

template<typename AllJct>
static void check_all_to_none(AllJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = none({nums.d, nums.e, nums.f});
    auto const expected = matches_all_to_none(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// All to one:

static bool matches_all_to_one(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 1
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 1
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 1;
}

template<typename AllJct>
static void check_all_to_one(AllJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = one({nums.d, nums.e, nums.f});
    auto const expected = matches_all_to_one(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// All to any:

static bool matches_all_to_any(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) != 0
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) != 0
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) != 0;
}

template<typename AllJct>
static void check_all_to_any(AllJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = any({nums.d, nums.e, nums.f});
    auto const expected = matches_all_to_any(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// All to all:

static bool matches_all_to_all(Numbers const &nums, Compare const comparison) {
    return compare_a_to_def(nums.a, nums.d, nums.e, nums.f, comparison) == 3
       and compare_a_to_def(nums.b, nums.d, nums.e, nums.f, comparison) == 3
       and compare_a_to_def(nums.c, nums.d, nums.e, nums.f, comparison) == 3;
}

template<typename AllJct>
static void check_all_to_all(AllJct const &j, Numbers const &nums, Compare const comparison) {
    auto const k        = all({nums.d, nums.e, nums.f});
    auto const expected = matches_all_to_all(nums, comparison);
    check(j, k, expected, nums, comparison);
}

// Perform every arithmetic comparison between All and every junction type...

template<typename AllJct>
static void check_all_to_everything(AllJct const &j, Numbers const &nums) {
    for (auto comparison = Compare::First;  comparison <= Compare::Last;  ++comparison) {
        check_all_to_none(j, nums, comparison);
        check_all_to_any( j, nums, comparison);
        check_all_to_all( j, nums, comparison);

        if (not nums.HasDuplicatesInDEF())
            check_all_to_one(j, nums, comparison);
    }
}

// ... where All-junctions are constructed both by reference and by copy.
// We've proved above that all() calls either all_copy() or all_ref(), as
// appropriate, as well as the handling of const input, and so we only need to
// test those lower-level functions -- we needn't test the all() overloads all
// over again.

static void check_all_to_every_junction_type() {
    for (auto bits = 0u;  bits <= 0xFFFu;  ++bits) {
        Numbers nums;
        nums.SetABC(bits);
        nums.SetDEF(bits);

        check_all_to_everything(all_copy({nums.a, nums.b, nums.c}), nums);
        check_all_to_everything(all_ref({nums.a, nums.b, nums.c}), nums);

        std::vector<unsigned> vec {nums.a, nums.b, nums.c};
        check_all_to_everything(all(vec.begin(), vec.end()), nums);
        check_all_to_everything(all_copy(vec), nums);
        check_all_to_everything(all_ref(vec), nums);

        std::set<unsigned> set {nums.a, nums.b, nums.c};
        check_all_to_everything(all_copy(set), nums);
        check_all_to_everything(all_ref(set), nums);
    }
}

#if USE_THREADS

static void compare_junctions_with_junctions() {
    std::thread none_thread(check_none_to_every_junction_type);
    std::thread  one_thread(check_one_to_every_junction_type);
    std::thread  any_thread(check_any_to_every_junction_type);
    std::thread  all_thread(check_all_to_every_junction_type);
                                                                      
    for (auto pt: {&none_thread, &one_thread, &any_thread, &all_thread})
        pt->join();
}

#else

static void compare_junctions_with_junctions() {
    check_none_to_every_junction_type();
    check_one_to_every_junction_type();
    check_any_to_every_junction_type();
    check_all_to_every_junction_type();
}

#endif

}   // Escape from namespace P6

int main() {
    P6::check_creation_types();
    P6::compare_junctions_with_constants();
    P6::compare_junctions_with_junctions();
    return 0;
}

