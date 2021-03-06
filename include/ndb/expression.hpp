#ifndef EXPRESSION_H_NDB
#define EXPRESSION_H_NDB

//! .tpp include operators to overload them

#include <ndb/expression/category.hpp>
#include <tuple>

namespace ndb
{
    template<class Type, class Engine, expression_categories Expr_category = Engine::expr_category>
    struct expression_type;

    struct expression_base {};

    template<class Type, class... Args>
    class expression : expression_base
    {
    public:
        using type = Type;

        template <std::size_t N>
        using arg_at = typename std::tuple_element<N, std::tuple<Args...>>::type;

        inline constexpr expression(Args&&... args);
        inline constexpr expression(std::tuple<Args...> args);

        template<class... Ts>
        inline constexpr auto operator()(const Ts&... ts) const;

        inline constexpr const std::tuple<Args...>& args() const;
        inline constexpr auto arg_count() const;

        template<class F>
        inline constexpr void eval(F&& f) const;

        template<class Engine, class Native_expression>
        inline static constexpr void make(Native_expression& ne);

    private:
        std::tuple<Args...> args_;
    };

    template<class T>
    inline constexpr auto expr_make(T&& v);
} // ndb

#include <ndb/expression.tpp>

#endif // EXPRESSION_H_NDB