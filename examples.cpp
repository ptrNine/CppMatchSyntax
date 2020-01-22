#include <iostream>
#include <cassert>
#include "match_syntax.hpp"

void match_distances() {
    int year;
    std::cout << "Enter you birth year: ";
    std::cin >> year;

    auto generation = match(year) {
        les(1928)         = "unnamed (too old :) )",
        inran(1928, 1946) = "silent",
        inran(1946, 1965) = "boomers",
        inran(1965, 1981) = "X",
        inran(1981, 1997) = "millennials",
        geql(1997)        = "Z"
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
            anyof("+", "-", "*", "/") = true,
            noopt = false
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
            mstest(is_number) = doo {
                auto num = std::stod(str);

                match(operation) {
                    eql("+") = doo { result += num; },
                    eql("-") = doo { result -= num; },
                    eql("*") = doo { result *= num; },
                    eql("/") = doo { result /= num; }
                };
            },
            mstest(is_sign) = doo { operation = str; },
            noopt = doo {} // Do nothing if no match
        };
    }

    std::cout << "Result = " << result << std::endl;
}


template <typename T>
bool is_three(const T& val) {
    return match(val) {
        anyof("three", '3', 3) = true,
        noopt = false
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
        eql("operation1") = ExpensiveConstructor(1),
        eql("operation2") = ExpensiveConstructor(2),
        eql("operation3") = ExpensiveConstructor(3)
    };

    // All constructors have been called :/
    assert(state == "EXPENSIVE EXPENSIVE EXPENSIVE ");

    // Fix that with "lazy"
    state = "";

    rc = match("operation3") {
        eql("operation1") = lazy(ExpensiveConstructor(1)),
        eql("operation2") = lazy(ExpensiveConstructor(2)),
        eql("operation3") = lazy(ExpensiveConstructor(3))
    };

    // Only one constructor has been called
    assert(state == "EXPENSIVE ");
}


void variant_test() {
    std::variant<std::string, int, float> val;
    val = "hello";

    // For unhandled use typ(decltype(val)) or [](auto)
    auto str = match(val) {
        typ(std::string) = "string",
        typ(int)         = "int",
    };

    std::cout << str << std::endl; // Prints "string"

    val = 2.f;

    // Prints float
    match(val) {
        [](float) { std::cout << "float"; },
        [](int)   { std::cout << "int"; },
        [](auto)  { std::cout << "unhandled"; }
    };
}

int main() {
    //match_distances();
    //calculator();
    //template_match();
    //lazify_example();
    //variant_test();
    return 0;
}