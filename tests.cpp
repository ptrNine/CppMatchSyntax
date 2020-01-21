#include "match_syntax.hpp"
#include "minitest.hpp"

using namespace std::literals;

int main() {
    SECTION("Numbers") {
        REQUIRE("one hundred forty four"s == match(144) {
            eql(7)   = "seven",
            eql(14)  = "fourteen",
            eql(100) = "one hundred",
            eql(144) = "one hundred forty four",
            eql(228) = "two hundred twenty two"
        });

        REQUIRE("greater 0.5"s == match(0.55) {
            grt(0.9) = "greater 0.9",
            grt(0.8) = "greater 0.8",
            grt(0.6) = "greater 0.6",
            grt(0.5) = "greater 0.5",
            grt(0.3) = "greater 0.3"
        });

        REQUIRE("less 0.4" == match(0.333) {
            les(0.3)   = "less 0.3",
            les(0.333) = "less 0.333",
            les(0.4)   = "less 0.4"
        });

        REQUIRE("less_eq 10" == match(10) {
            les(5)     = "less 5",
            leql(10.0) = "less_eq 10"
        });

        REQUIRE("greater_eq 15" == match(15.0) {
            geql(16.9) = "greater_eq 16.9",
            geql(15)   = "greater_eq 15"
        });

        REQUIRE("no_opt" == match(10) {
            eql(5)   = "equal 5",
            geql(20) = "greater_eq 20",
            noopt    = "no_opt"
        });

        REQUIRE("in_range 22.55 100" == match(22.55) {
            inran(4, 6)        = "in_range 4 6",
            inran(20.5, 22.55) = "in_range 20.5 22.55",
            inran(22.55, 100)  = "in_range 22.55 100"
        });
    }

    SECTION("Strings") {
        REQUIRE(match("str") {
            eql("kek")      = false,
            eql(u"16 byte") = false,
            eql("str"s)     = true,
            eql(U"str2")    = false
        });

        REQUIRE(match("str") {
            eql("str")      = true,
            eql(u"16 byte") = false,
            eql("str4"s)    = false,
            eql(U"str2")    = false
        });


        std::string str = "str";
        REQUIRE(match(str) {
            eql("str5")     = false,
            eql(u"16 byte") = false,
            eql("str4"s)    = false,
            eql("str")      = true
        });

        std::string_view str2 = "hit";
        auto str3 = "hit";
        REQUIRE(match(str2) {
            eql("str5")     = false,
            eql(u"16 byte") = false,
            eql("str4"s)    = false,
            eql(U"str")     = false,
            eql(str3)       = true
        });
    }

    SECTION("With lambda") {
        std::string test;

        match(150) {
                inran(0, 100)   = doo { test = "0, 100"; },
                inran(100, 200) = doo { test = "hit!"; }
        };

        REQUIRE(test == "hit!");
    }

    SECTION("Inner match") {
        // Do not use this, inner matchs isn't lazy
        auto answ = match(228) {
                eql(4) = match(20) {
                        eql(21) = "false",
                        eql(52) = "false",
                        noopt   = "false"
                },
                eql(228) = match(20) {
                        eql(22) = "false",
                        grt(15) = "true",
                        noopt   = "false"
                }
        };

        REQUIRE("true"s == answ);


        // Lazy inner matchs via lambdas
        // doo = []()
        static std::string lazy_test;

        auto answ2 = match(228) {
                eql(4) = doo {
                    lazy_test += "first";
                    return match(20) {
                            eql(21) = "false",
                            eql(52) = "false",
                            noopt   = "false"
                    };},
                eql(228) = doo {
                    lazy_test += "second";
                    return match(20) {
                            eql(22) = "false",
                            grt(15) = "true",
                            noopt   = "false"
                    };}
        };
        REQUIRE("true"s == answ2);
        REQUIRE("second"s == lazy_test);


        // Same as previous, use in_match macro
        auto answ3 = match(228) {
                eql(4) = lazy(match(20) {
                    eql(21) = "false",
                    eql(52) = "false",
                    noopt   = "false"
                }),
                eql(228) = in_match(20, {
                    eql(22) = "false",
                    grt(15) = "true",
                    noopt   = "false"
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
                eql("very exp") = lazy(very_expensive_operation()),
                eql("exp")      = lazy(expensive_operation())
        };
        REQUIRE("EXPENSIVE"s == lazy_check);
        REQUIRE(answ);

        lazy_check = "";

        // Not lazy
        auto answ2 = match("exp") {
                eql("very exp") = very_expensive_operation(),
                eql("exp")      = expensive_operation()
        };
        REQUIRE("VERY EXPENSIVEEXPENSIVE"s == lazy_check);
        REQUIRE(answ);
    }
}