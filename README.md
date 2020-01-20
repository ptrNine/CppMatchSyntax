CppMatchSyntax
==============

[![Build Status](https://travis-ci.com/ptrNine/CppMatchSyntax.svg?branch=master)](https://travis-ci.com/ptrNine/CppMatchSyntax)

Pattern match syntax for C++


A little bit more than switch.
You can do this:
```c++
auto val = match("four") {
    equal("one")   = 1,
    equal("two")   = 2,
    equal("three") = 3,
    equal("four")  = 4,
    no_opt         = -1 // Default case
};
std::cout << val << std::endl; // prints 4
```

or this:
```c++
int score = 6;
auto val = match(score) {
    in_range(0, 3) = "too low",
    less(5)        = "low",
    less(7)        = "normal",
    less(9)        = "high",
    less_eq(10)    = "very high",
    no_opt         = "Invalid score"
};
std::cout << val << std::endl; // prints "normal"
```

and this:
```c++
int score = 6;
auto val = match(score) {
    in_range(0, 3) = "too low",
    less(5)        = "low",
    less(7)        = doo { score = 0; return "normal"; },
    less(9)        = "high",
    less_eq(10)    = "very high",
    no_opt         = "Invalid score"
};
std::cout << val << std::endl; // prints "normal"
std::cout << score << std::endl; // prints 0
```
and etc...

See [examples.cpp](https://github.com/ptrNine/CppMatchSyntax/blob/master/examples.cpp) and
[tests.cpp](https://github.com/ptrNine/CppMatchSyntax/blob/master/tests.cpp)
for other examples
