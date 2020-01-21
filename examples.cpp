#include <iostream>
#include <cassert>
#include "match_syntax.hpp"

void match_distances() {
    int year;
    std::cout << "Enter you birth year: ";
    std::cin >> year;

    auto generation = match(year) {
        less(1928)           = "unnamed (too old :) )",
        in_range(1928, 1946) = "silent",
        in_range(1946, 1965) = "boomers",
        in_range(1965, 1981) = "X",
        in_range(1981, 1997) = "millennials",
        greater_eq(1997)     = "Z"
    };

    std::cout << "You are belong to " << generation << " generation" << std::endl;
}


void calculator() {
    // Is number function
    auto is_number = [](const std::string& str) {
        try {
            std::stod(str);
            return true;
        } catch (...) {
            return false;
        }
    };

    // Is sign function
    auto is_sign = [](const std::string& str) {
        return match(str) {
            any_of("+", "-", "*", "/") = true,
            no_opt = false
        };
    };

    std::cout << "Expression examples: \n" <<
        "+ 1 3 5 .         -> 9 \n"
        "+ 1 3 5 * 2 .     -> 18 \n"
        "+ 6 2 * 2 - 1 4 . -> 11\n"
        "Write '.' for ending \n";
    std::cout << "Enter the expression: ";

    // Read all before .
    auto strs = std::vector<std::string>();
    while(strs.empty() || strs.back() != ".") {
        strs.emplace_back();
        std::cin >> strs.back();
    }
    strs.pop_back();

    double result = 0;
    std::string operation = "+";

    for (auto& str : strs) {
        match(str) {
            mtest(is_number) = doo {
                auto num = std::stod(str);

                match(operation) {
                    equal("+") = doo { result += num; },
                    equal("-") = doo { result -= num; },
                    equal("*") = doo { result *= num; },
                    equal("/") = doo { result /= num; }
                };
            },
            mtest(is_sign) = doo { operation = str; },
            no_opt = doo {} // Do nothing if no match
        };
    }

    std::cout << "Result = " << result << std::endl;
}


template <typename T>
bool is_three(const T& val) {
    return match(val) {
        any_of("three", '3', 3) = true,
        no_opt = false
    };
}

void template_match() {
    if (is_three("three") && is_three('3') && is_three(3) && !is_three(int('3')))
        std::cout << "It's worked!" << std::endl;
}


void lazify_example() {
    static std::string state;

    class ExpensiveConstructor {
    public:
        ExpensiveConstructor(int) {
            state += "EXPENSIVE ";
        }
    };

    auto rc = match("operation3") {
        equal("operation1") = ExpensiveConstructor(1),
        equal("operation2") = ExpensiveConstructor(2),
        equal("operation3") = ExpensiveConstructor(3)
    };

    // All constructors have been called :/
    assert(state == "EXPENSIVE EXPENSIVE EXPENSIVE ");

    // Fix that with "lazy"
    state = "";

    rc = match("operation3") {
            equal("operation1") = lazy(ExpensiveConstructor(1)),
            equal("operation2") = lazy(ExpensiveConstructor(2)),
            equal("operation3") = lazy(ExpensiveConstructor(3))
    };

    // Only one constructor has been called
    assert(state == "EXPENSIVE ");
}


int main() {
    //match_distances();
    //calculator();
    template_match();
    lazify_example();

    return 0;
}