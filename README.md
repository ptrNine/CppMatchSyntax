CppMatchSyntax
==============

[![Build Status](https://travis-ci.com/ptrNine/CppMatchSyntax.svg?branch=master)](https://travis-ci.com/ptrNine/CppMatchSyntax)

Pattern match syntax for C++


A little bit more than switch.
You can do this:
```c++
auto val = match("four") {
    eql("one")   = 1,
    eql("two")   = 2,
    eql("three") = 3,
    eql("four")  = 4,
    noopt        = -1 // Default case
};
std::cout << val << std::endl; // prints 4
```

or this:
```c++
int score = 6;
auto val = match(score) {
    inran(0, 3) = "too low",
    les(5)      = "low",
    les(7)      = "normal",
    les(9)      = "high",
    leql(10)    = "very high",
    noopt       = "Invalid score"
};
std::cout << val << std::endl; // prints "normal"
```

and this:
```c++
int score = 6;
auto val = match(score) {
    inran(0, 3) = "too low",
    les(5)      = "low",
    les(7)      = doo { score = 0; return "normal"; },
    les(9)      = "high",
    leql(10)    = "very high",
    noopt       = "Invalid score"
};
std::cout << val << std::endl; // prints "normal"
std::cout << score << std::endl; // prints 0
```
and etc...

See [examples.cpp](https://github.com/ptrNine/CppMatchSyntax/blob/master/examples.cpp) and
[tests.cpp](https://github.com/ptrNine/CppMatchSyntax/blob/master/tests.cpp)
for other examples
