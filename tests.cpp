#include "match_syntax.hpp"
#include "minitest.hpp"

using namespace std::literals;

int main() {
    SECTION("Numbers") {
        REQUIRE("one hundred forty four"s == match(144) {
            equal(7)   = "seven",
            equal(14)  = "fourteen",
            equal(100) = "one hundred",
            equal(144) = "one hundred forty four",
            equal(228) = "two hundred twenty two"
        });

        REQUIRE("greater 0.5"s == match(0.55) {
            greater(0.9) = "greater 0.9",
            greater(0.8) = "greater 0.8",
            greater(0.6) = "greater 0.6",
            greater(0.5) = "greater 0.5",
            greater(0.3) = "greater 0.3"
        });

        REQUIRE("less 0.4" == match(0.333) {
            less(0.3)   = "less 0.3",
            less(0.333) = "less 0.333",
            less(0.4)   = "less 0.4"
        });

        REQUIRE("less_eq 10" == match(10) {
            less(5)       = "less 5",
            less_eq(10.0) = "less_eq 10"
        });

        REQUIRE("greater_eq 15" == match(15.0) {
            greater_eq(16.9) = "greater_eq 16.9",
            greater_eq(15)   = "greater_eq 15"
        });

        REQUIRE("no_opt" == match(10) {
            equal(5)       = "equal 5",
            greater_eq(20) = "greater_eq 20",
            no_opt         = "no_opt"
        });

        REQUIRE("in_range 22.55 100" == match(22.55) {
            in_range(4, 6)        = "in_range 4 6",
            in_range(20.5, 22.55) = "in_range 20.5 22.55",
            in_range(22.55, 100)  = "in_range 22.55 100"
        });
    }

    SECTION("Strings") {
        REQUIRE(match("str") {
            equal("kek")      = false,
            equal(u"16 byte") = false,
            equal("str"s)     = true,
            equal(U"str2")    = false
        });

        REQUIRE(match("str") {
            equal("str")      = true,
            equal(u"16 byte") = false,
            equal("str4"s)    = false,
            equal(U"str2")    = false
        });


        std::string str = "str";
        REQUIRE(match(str) {
            equal("str5")     = false,
            equal(u"16 byte") = false,
            equal("str4"s)    = false,
            equal("str")      = true
        });

        std::string_view str2 = "hit";
        auto str3 = "hit";
        REQUIRE(match(str2) {
            equal("str5")     = false,
            equal(u"16 byte") = false,
            equal("str4"s)    = false,
            equal(U"str")     = false,
            equal(str3)       = true
        });
    }

    SECTION("With lambda") {
        std::string test;

        match(150) {
                in_range(0, 100)   = doo { test = "0, 100"; },
                in_range(100, 200) = doo { test = "hit!"; }
        };

        REQUIRE(test == "hit!");
    }

    SECTION("Inner match") {
        // Do not use this, inner matchs isn't lazy
        auto answ = match(228) {
                equal(4) = match(20) {
                        equal(21) = "false",
                        equal(52) = "false",
                        no_opt    = "false"
                },
                equal(228) = match(20) {
                        equal(22)   = "false",
                        greater(15) = "true",
                        no_opt      = "false"
                }
        };

        REQUIRE("true"s == answ);


        // Lazy inner matchs via lambdas
        // doo = []()
        static std::string lazy_test;

        auto answ2 = match(228) {
                equal(4) = doo {
                    lazy_test += "first";
                    return match(20) {
                            equal(21) = "false",
                            equal(52) = "false",
                            no_opt    = "false"
                    };},
                equal(228) = doo {
                    lazy_test += "second";
                    return match(20) {
                            equal(22)   = "false",
                            greater(15) = "true",
                            no_opt      = "false"
                    };}
        };
        REQUIRE("true"s == answ2);
        REQUIRE("second"s == lazy_test);


        // Same as previous, use in_match macro
        auto answ3 = match(228) {
                equal(4) = lazy(match(20) {
                    equal(21) = "false",
                    equal(52) = "false",
                    no_opt    = "false"
                }),
                equal(228) = in_match(20, {
                    equal(22)   = "false",
                    greater(15) = "true",
                    no_opt      = "false"
                })
        };
        REQUIRE("true"s == answ3);
    }

    SECTION("Lazy match") {
        static std::string lazy_check;
        auto expensive_operation = []() {
            lazy_check += "EXPENSIVE";
            return true;
        };

        auto very_expensive_operation = []() {
            lazy_check += "VERY EXPENSIVE";
            return false;
        };

        auto answ = match("exp") {
                equal("very exp") = lazy(very_expensive_operation()),
                equal("exp")      = lazy(expensive_operation())
        };
        REQUIRE("EXPENSIVE"s == lazy_check);
        REQUIRE(answ);

        lazy_check = "";

        // Not lazy
        auto answ2 = match("exp") {
                equal("very exp") = very_expensive_operation(),
                equal("exp")      = expensive_operation()
        };
        REQUIRE("VERY EXPENSIVEEXPENSIVE"s == lazy_check);
        REQUIRE(answ);
    }
}