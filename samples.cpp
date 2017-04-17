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

#include "JunctionAll.h"
#include "JunctionAny.h"
#include "JunctionOne.h"

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

// This source file shows some simple ways of using junctions.

using namespace P6;

void check_digits() {
    // Junctions are a convenient way to test several variables at once:
    int const a {1}, b {3}, c {7}, d {8};
    assert(all({a, b, c, d}) < 10);
    assert(one({2, 5, 98, 4}) < b);
    assert(b > any({a, c, d}));
    assert(not(all({a, b, c, d}) > 2));

    // Junctions can work with complete STL containers:
    std::vector<int> digits {1, 4, 2, 8, 5, 7};
    assert(all(digits) >= 1);
    assert(any(digits) >  5);
    assert(one(digits) == 4);
    assert(none(digits) == 3);

    assert(not(all(digits) > 3));
    assert(not(any(digits) > 8));
    assert(none(digits) > 8);        // Exactly equivalent to "not(any)" above
    assert(not(one(digits) > 3));    // It's not one digit: it's four digits
    assert(not(one(digits) == 3));   // It's not one digit: it's no digits

    // "not none()" is better spelt as "any()".

    // We can work with a pair of iterators, rather than a complete container:
    auto const all_inner_digits = all(digits.begin() + 1, digits.end() - 1);
    assert(all_inner_digits > 1);

    // Applying a lambda to a junction creates a modified copy:
    auto const all_inner_digits_decremented = all_inner_digits([] (auto n) {return n-1;});
    assert(all_inner_digits_decremented >= 1);
    assert(not(all_inner_digits_decremented >= 2));
}

void check_strings() {
    // We need to convert elements to std::string to get something better than
    // C++'s default string comparison semantic, in which ("Jill" > "Catherine")
    // returns something different from what a naive user would expect.

    auto all_names = all<std::string> ({"Fred", "Jim", "Sheila"});
    assert(all_names > "Catherine");
    assert(all_names != "Clarence");

    // Use a lambda to get an All-junction of string lengths:
    auto const all_lengths = all_names([] (auto const &str) {return str.size();});
    assert(all_lengths > 2u);
    assert(not(all_lengths > 3u));
}

int main() {
    check_digits();
    check_strings();
    return 0;
}

