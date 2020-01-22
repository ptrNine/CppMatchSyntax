#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <any>

#ifdef MATCH_SYNTAX_USE_SCL
    #include <scl/traits.hpp>
    #include <scl/string.hpp>
#endif

namespace {
    template <typename T>
    inline constexpr bool is_number_v = !std::is_same_v<T, char> && (std::is_integral_v<T> || std::is_floating_point_v<T>);

#ifdef MATCH_SYNTAX_USE_SCL
    #define STR_EQUAL_ITER(TYPE) \
    if (str2.type() == typeid(TYPE)) \
        return !scl::strcmp_any(str1, std::any_cast<TYPE>(str2))

    #define DECLARE_BOOL_TRAIT_ALIAS(name) \
    template<typename T> inline constexpr bool name = scl::name<T>

    DECLARE_BOOL_TRAIT_ALIAS(is_function_v);
    DECLARE_BOOL_TRAIT_ALIAS(is_c_string_v);
    DECLARE_BOOL_TRAIT_ALIAS(is_any_string_v);

    template <typename T>
    inline bool str_any_equal(const T& str1, const std::any& str2) {
        STR_EQUAL_ITER(const char*);
        else STR_EQUAL_ITER(scl::String);
        else STR_EQUAL_ITER(std::string);
        else STR_EQUAL_ITER(std::string_view);
        else STR_EQUAL_ITER(scl::String16);
        else STR_EQUAL_ITER(scl::String32);
        else STR_EQUAL_ITER(const char16_t*);
        else STR_EQUAL_ITER(const char32_t*);

        return false;
    }

    template <typename T>
    inline bool str_any_equal(const T* str1, const std::any& str2) {
        return str_any_equal(scl::String(str1), str2);
    }

    #undef DECLARE_BOOL_ALIAS
#else
    #define STR_EQUAL_ITER(TYPE) \
    if (str2.type() == typeid(TYPE)) \
        return std::string(str1) == std::any_cast<TYPE>(str2)

    #define SICB static inline constexpr bool

    template <typename T>
    struct is_c_string { SICB value = false; };

    template <typename T, size_t _Size>
    struct is_c_string<const T(&)[_Size]> { SICB value = std::is_same_v<char, T>; };

    template <typename T, size_t _Size>
    struct is_c_string<T[_Size]> { SICB value = std::is_same_v<char, T>; };

    template <typename T>
    SICB is_c_string_v = is_c_string<T>::value;

    template <typename T, template <typename...> typename T2>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> typename T, typename... ArgsT>
    struct is_specialization_of<T<ArgsT...>, T>: std::true_type {};

    template <typename T>
    static inline constexpr bool is_any_string_v =
            is_c_string_v<T> ||
            is_specialization_of<T, std::basic_string>::value ||
            is_specialization_of<T, std::basic_string_view>::value ||
            std::is_same_v<char*, T> || std::is_same_v<const char*, T>;


    template <typename T = void>
    struct Void { using type = void; };


    template <typename T, typename = void>
    struct is_function : std::false_type {};

    template <typename FunctorT>
    struct is_function<FunctorT, typename Void<decltype(&FunctorT::operator())>::type>
            : public is_function<decltype(&FunctorT::operator())> {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...)> : std::true_type {};

    template <typename FunctorT, typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(FunctorT::*)(ArgsT...) const> : std::true_type {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(ArgsT...)> : std::true_type {};

    template <typename ReturnT, typename... ArgsT>
    struct is_function<ReturnT(*)(ArgsT...)> : std::true_type {};

    template <typename T>
    SICB is_function_v = is_function<T>::value;


    template <typename T>
    inline bool str_any_equal(const T& str1, const std::any& str2) {
        STR_EQUAL_ITER(const char*);
        else STR_EQUAL_ITER(std::string);
        else STR_EQUAL_ITER(std::string_view);
        return false;
    }

    template <typename T>
    inline bool str_any_equal(const T* str1, const std::any& str2) {
        return str_any_equal(std::string(str1), str2);
    }

    #undef SICB
#endif
}

#undef STR_EQUAL_ITER

enum class MatchCaseType {
    Default, Equal, Greater, Less, GreaterEq, LessEq, InRange, Test, AnyOf
};

template <typename ArgT>
class MatchCase {
public:
    MatchCase(MatchCaseType type, std::any&& value, std::any&& value2, const ArgT& arg):
            _val(std::move(value)), _val2(std::move(value2)), _arg(arg), _type(type) {}

    const ArgT& arg() const {
        return _arg;
    }

    template <typename T>
    bool test(MatchCaseType type, const T& val, const std::any& any1, const std::any& any2) const {
        if (type == MatchCaseType::Default)
            return true;

        if constexpr (is_number_v<T>) {
            if (any1.type() == typeid(double)) {
                auto number = std::any_cast<double>(any1);

                switch (type) {
                    case MatchCaseType::Equal:
                        return val == number;
                    case MatchCaseType::Greater:
                        return val > number;
                    case MatchCaseType::Less:
                        return val < number;
                    case MatchCaseType::GreaterEq:
                        return val >= number;
                    case MatchCaseType::LessEq:
                        return val <= number;
                    case MatchCaseType::InRange:
                        return val >= number && val < std::any_cast<double>(any2);

                    case MatchCaseType::Default: default:
                        break;
                }
            }
        }
        else {
            switch (type) {
                case MatchCaseType::Equal:
                    if constexpr (is_any_string_v<T>)
                        return str_any_equal(val, any1);

                    if (any1.type() == typeid(T))
                        return val == std::any_cast<const T&>(any1);

                case MatchCaseType::Default: default:
                    break;
            }
        }

        if (type == MatchCaseType::AnyOf) {
            auto vec = std::any_cast<const std::vector<std::any>&>(any1);

            bool res = false;
            for (auto& v : vec) {
                if (test(MatchCaseType::Equal, val, v, {})) {
                    res = true;
                    break;
                }
            }

            return res;
        }

        // Todo: strings
        if (type == MatchCaseType::Test)
            return std::any_cast<std::function<bool(const T&)>>(any1)(val);

        return false;
    }

    template <typename T>
    bool test(const T& val) const {
        return test(_type, val, _val, _val2);
    }

private:
    std::any      _val;
    std::any      _val2;
    ArgT          _arg;
    MatchCaseType _type;
};

class MatchCondition {
public:
    template <typename T, typename std::enable_if_t<!is_number_v<T>>...>
    MatchCondition(MatchCaseType type, const T& value): _val(value), _type(type) {}

    template <typename T, typename std::enable_if_t<is_number_v<T>>...>
    MatchCondition(MatchCaseType type, const T& value): _val(double(value)), _type(type) {}

    template <typename T, typename T2, typename std::enable_if_t<is_number_v<T> && is_number_v<T2>>...>
    MatchCondition(MatchCaseType type, const T& value, const T2& value2):
            _val(double(value)), _val2(double(value2)), _type(type) {}

    static MatchCondition Default() {
        return MatchCondition(MatchCaseType::Default, 0);
    }

#define DEF_CONDITION_ONE_ARG(NAME) \
    template <typename T> \
    static MatchCondition NAME(const T& value) { \
        return MatchCondition(MatchCaseType::NAME, value); \
    }

#define DEF_CONDITION_TWO_ARG(NAME) \
    template <typename T, typename T2> \
    static MatchCondition NAME(const T& value1, const T2& value2) { \
        return MatchCondition(MatchCaseType::NAME, value1, value2); \
    }

    DEF_CONDITION_ONE_ARG(Equal)
    DEF_CONDITION_ONE_ARG(Greater)
    DEF_CONDITION_ONE_ARG(Less)
    DEF_CONDITION_ONE_ARG(GreaterEq)
    DEF_CONDITION_ONE_ARG(LessEq)
    DEF_CONDITION_TWO_ARG(InRange)
    DEF_CONDITION_ONE_ARG(Test)

    template <typename T>
    static auto any_of_helper(const T& v) {
        if constexpr (is_number_v<T>)
            return double(v);
        else
            return v;
    }

    template <typename... ArgsT>
    static MatchCondition AnyOf(ArgsT&&... args) {
        return MatchCondition(MatchCaseType::AnyOf, std::vector<std::any>{any_of_helper(args)...});
    }

#undef DEF_CONDITION_ONE_ARG
#undef DEF_CONDITION_TWO_ARG

    template <typename ArgT>
    auto operator=(const ArgT& arg) {

        if constexpr (is_function_v<ArgT>)
            return MatchCase(_type, std::move(_val), std::move(_val2), std::function{arg});
        else if constexpr (is_c_string_v<ArgT>) {
            using T = const std::remove_pointer_t<std::decay_t<ArgT>>*;
            using LambdaT = struct { T operator()() const { return a; } T a; };

            return MatchCase(_type, std::move(_val), std::move(_val2), std::function{LambdaT{arg}});
        }
        else {
            using LambdaT = struct { ArgT operator()() const { return a; } ArgT a; };

            return MatchCase(_type, std::move(_val), std::move(_val2), std::function{LambdaT{arg}});
        }
    }

private:
    std::any _val;
    std::any _val2;
    MatchCaseType _type;
};


template <typename T>
class MatchDef {
public:
    MatchDef(const T& def): _def(def) {}
    MatchDef(T&& def): _def(std::move(def)) {}

    template <typename CaseT, size_t _Sz>
    auto operator*(const std::array<CaseT, _Sz>& cases) const {
        using Type = std::decay_t<decltype(cases.front().arg())>;

        for (auto& cs : cases) {
            if (cs.test(_def)) {
                if constexpr (is_function_v<Type>) {
                    return cs.arg()();
                } else
                    return cs.arg();
            }
        }

        throw std::runtime_error("No case");
    }

private:
    T _def;
};

template <typename T>
inline auto MatchDefine(T&& value) {
    if constexpr (is_c_string_v<T>)
        return MatchDef<std::decay_t<T>>(value);
    else
        return MatchDef(std::forward<T>(value));
}

/**
 * Define match expression (same as 'switch')
 * @param VAL - value to be matched
 */
#define match(VAL) MatchDefine(VAL) * std::array

/**
 * Default case (same as 'default')
 */
#define noopt MatchCondition::Default()

/**
 * Match if value is equal with VAL
 */
#define eql(VAL) MatchCondition::Equal(VAL)

/**
 * Match if value is greater than VAL
 */
#define grt(VAL) MatchCondition::Greater(VAL)

/**
 * Match if value is less than VAL
 */
#define les(VAL) MatchCondition::Less(VAL)

/**
 * Match if value is equal to or greater than VAL
 */
#define geql(VAL) MatchCondition::GreaterEq(VAL)

/**
 * Match if value is equal to or less than VAL
 */
#define leql(VAL) MatchCondition::LessEq(VAL)

/**
 * Match if value is in range from LOW to HIGH (not inclusive)
 */
#define inran(LOW, HIGH) MatchCondition::InRange(LOW, HIGH)

/**
 * Match if test callback FUNCTION return true
 * @param FUNCTION - callback with type bool(const T&) when T is type of value to be matched
 */
#define mstest(FUNCTION) MatchCondition::Test(std::function{FUNCTION})

/**
 * Match if one of arguments is equal
 */
#define anyof(...) MatchCondition::AnyOf(__VA_ARGS__)

/**
 * Do operation
 *
 * Use after branch condition, for example:
 * equal(5) = doo { std::cout << "it's five" }
 */
#define doo [&]()

/**
 * Inner match
 *
 * Use in inner match expressions
 *
 * match(5) { equal(5) = in_match(6, { equal(6) = true }) }
 */
#define in_match(VAL, ...) doo { return match(VAL) __VA_ARGS__ ; }

/**
 * Use for "lazify" operations
 */
#define lazy(...) doo { return __VA_ARGS__; }
