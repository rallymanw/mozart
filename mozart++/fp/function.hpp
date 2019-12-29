/**
 * Mozart++ Template Library
 * Licensed under MIT License
 * Copyright (c) 2019 Covariant Institute
 * Website: https://covariant.cn/
 * Github:  https://github.com/covariant-institute/
 */

#pragma once

#include <mozart++/core/base.hpp>
#include <mozart++/core/typelist.hpp>
#include <functional>

namespace mpp {
    using std::function;

    template<typename A, typename B, typename C>
    static mpp::function<C(A)> compose(const mpp::function<C(B)> &f,
                                       const mpp::function<B(A)> &g) {
        return std::bind(f, std::bind(g, std::placeholders::_1));
    }

    template<typename A>
    static mpp::function<bool(A)> boolean_compose(const mpp::function<bool(A)> &f,
                                                  const mpp::function<bool(A)> &g) {
        return [=](A x) { return f(x) && g(x); };
    }

    /**
     * An alias for mpp::function. In case that we need to
     * use our own function implementation in the future.
     */
    template <typename T>
    using function_alias = mpp::function<T>;

    template <typename ... Args>
    struct arg_type_info {
        /**
         * Raw argument types, unmodified.
         */
        using arg_types = mpp::typelist::list<Args...>;

        /**
         * Purified argument types, with all qualifiers removed.
         * @see TypePurifier
         */
        using decayed_arg_types = mpp::typelist::list<std::decay_t<Args>...>;
    };

    /**
     * Parse callable object and lambdas.
     * @tparam F functor
     */
    template <typename F>
    struct functor_parser : public functor_parser<decltype(&F::operator())> {
    };

    /**
     * Base condition of recursive, extracting all information from a function.
     */
    template <typename Class, typename R, typename... Args>
    struct functor_parser<R(Class::*)(Args...) const> : public arg_type_info<Args...> {
        using function_type = mpp::function_alias<R(Args...)>;
        using return_type = R;
        using class_type = Class;
    };

    /**
     * Parse function typename to return type and argument type(s).
     * @tparam F function typename
     */
    template <typename F>
    struct function_parser : public functor_parser<F> {
    };

    /**
     * Parse all function pointers, including
     * global functions, static class functions.
     */
    template <typename P>
    struct function_parser<P *> : public function_parser<P> {
    };

    /**
     * Parse function (pointer) lvalue references.
     */
    template <typename F>
    struct function_parser<F &> : public function_parser<F> {
    };

    /**
     * Parse function (pointer) rvalue references.
     */
    template <typename F>
    struct function_parser<F &&> : public function_parser<F> {
    };

    /**
     * Parse instance methods by converting signature to normal form.
     */
    template <typename Class, typename R, typename... Args>
    struct function_parser<R(Class::*)(Args...)> : public function_parser<R(Class &, Args...)> {
        using class_type = Class;
    };

    /**
     * Parse const instance methods by converting signature to normal form.
     */
    template <typename Class, typename R, typename... Args>
    struct function_parser<R(Class::*)(Args...) const> : public function_parser<R(const Class &, Args...) const> {
        using class_type = Class;
    };

    /**
     * Parse a normal function by converting signature to const normal form.
     */
    template <typename R, typename... Args>
    struct function_parser<R(Args...)> : public function_parser<R(Args...) const> {
    };

    /**
     * Base condition of recursion, extracting all information
     * from a normal function signature.
     */
    template <typename R, typename... Args>
    struct function_parser<R(Args...) const> : public arg_type_info<Args...> {
        using function_type = mpp::function_alias<R(Args...)>;
        using return_type = R;
    };

    /**
     * Short for {@code typename function_parser<F>::FunctionType}
     */
    template <typename F>
    using function_type = typename function_parser<F>::function_type;

    /**
     * Convert callable things to function type (aka FunctionAlias).
     * Sample usage: {@code auto f = makeFunction(sth); }
     *
     * @see FunctionAlias
     * @tparam F Function typename
     * @param f function itself
     * @return FunctionAlias object
     */
    template <typename F>
    static function_type<F> make_function(F &f) {
        return static_cast<function_type<F>>(f);
    }
}
