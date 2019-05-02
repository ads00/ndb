#ifndef UTILITY_H_NDB
#define UTILITY_H_NDB

//! \brief utility functions

#include <ndb/fwd.hpp>
#include <ndb/cx_error.hpp>
#include <ndb/scope.hpp>

#include <string>
#include <utility>

#ifdef __GNUC__
    #include <cxxabi.h>
#endif

//#include <boost/algorithm/string/replace.hpp>

namespace ndb
{
////////////////////////////////////////////////////////////////////////////////
////////////////////////             GENERIC            ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    // compile time str _len
    constexpr size_t cx_str_len(const char* str)
    {
        return *str ? 1 + cx_str_len(str + 1) : 0;
    }

    namespace internal
    {
        template <class T, class = void>
        struct is_complete : std::false_type{};

        template <class T>
        struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type{};

        // T is a complete type and base_of T
        template<class Base, class T, class = void>
        struct is_base_of : std::false_type {};

        template<class Base, class T>
        struct is_base_of<Base, T, std::enable_if_t<is_complete<T>::value>> : std::conditional_t<std::is_base_of_v<Base, T>, std::true_type, std::false_type> {};
    } // internal

    // index_of
    template<class T, class Container>
    struct index_of;

    template<class T, template<class...> class Container>
    struct index_of<T, Container<>>
    {
        constexpr static auto value = ncx_error_value(index_of, cx_err_type_not_found, T, Container);
    };

    template<class T, template<class...> class Container, class... Ts>
    struct index_of<T, Container<T, Ts...>>
    {
        constexpr static auto value = 0;
    };

    template<class T, template<class...> class Container, class T_test, class... Ts>
    struct index_of<T, Container<T_test, Ts...>>
    {
        constexpr static auto value = 1 + index_of<T, Container<Ts...>>::value;
    };

    namespace internal
    {
        template<class Needle, class Haystack>
        struct has_type;

        template<class Needle, template<class...> class Haystack, class T1, class... T>
        struct has_type<Needle, Haystack<T1, T...>> { static constexpr bool value = has_type<Needle, Haystack<T...>>::value; };

        template<class Needle, template<class...> class Haystack>
        struct has_type<Needle, Haystack<>> : std::false_type {};

        template<class Needle, template<class...> class Haystack, class... T>
        struct has_type<Needle, Haystack<Needle, T...>> : std::true_type {};
    } // internal

    template<class Needle, class Haystack>
    constexpr auto has_type_v = internal::has_type<Needle, Haystack>::value;

    // is_same_tpl (T<> == U<>)
    template<template<class...> class T, class U>
    struct is_same_tpl
    {
        static constexpr const bool value = false;;
    };

    template<template<class...> class T, template<class...> class U, class... Us>
    struct is_same_tpl<T, U<Us...>>
    {
        static constexpr const bool value = std::is_same_v<T<Us...>, U<Us...>>;
    };

    template<template<class...> class T, class U>
    constexpr auto is_same_tpl_v = is_same_tpl<T, U>::value;

    struct void_{};
    template<class F, class... Args>
    constexpr auto call(F&& f, Args... args)
    {
        if constexpr (std::is_void_v<decltype(f(args...))>)
        {
            return void_{};
        }
        else return f(std::forward<Args>(args)...);
    }

////////////////////////////////////////////////////////////////////////////////
////////////////////////              NDB               ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    template<class T>
    static constexpr bool is_query = internal::is_base_of<ndb::query_base, T>::value;

    template<class T>
    static constexpr bool is_table = internal::is_base_of<ndb::table_base, T>::value;

    template<class T>
    static constexpr bool is_field =  internal::is_base_of<ndb::field_base, std::decay_t<T>>::value;

    template<class T>
    static constexpr bool is_field_entity = internal::is_base_of<ndb::table_base, typename T::value_type>::value;

    template<class T>
    static constexpr bool is_field_entity_vector = is_field_entity<T> && (T{}.detail_.size == 0);

    template<class T>
    static constexpr bool is_database =  internal::is_base_of<ndb::database_base, T>::value;

    template<class T>
    static constexpr bool is_option = internal::is_base_of<ndb::option_base, T>::value;

    template<class T>
    static constexpr bool is_expression = internal::is_base_of<ndb::expression_base, std::decay_t<T>>::value;

/////////////////////////////////////////////////////////////////////////////////
////////////////////////           EXPRESSION            ////////////////////////
/////////////////////////////////////////////////////////////////////////////////
    namespace internal
    {
        template<class Expr, class T>
        struct expr_type_check { static constexpr auto value = std::is_same<typename std::decay_t<Expr>::type, T>::value; };

        template<class Expr>
        struct expr_is_value :
            std::conditional_t<is_expression<Expr>, expr_type_check<Expr, expressions::value_>, std::false_type>{};

        template<class Expr>
        struct expr_is_field :
            std::conditional_t<is_expression<Expr>, expr_type_check<Expr, expressions::field_>, std::false_type>{};

        template<class Expr>
        struct expr_is_table :
            std::conditional_t<is_expression<Expr>, expr_type_check<Expr, expressions::table_>, std::false_type>{};

        template<class Expr>
        struct expr_is_scalar
        {
            static constexpr bool value = expr_is_field<Expr>::value || expr_is_value<Expr>::value || expr_is_table<Expr>::value;
        };
    } // internal


    template<class Expr> constexpr bool expr_is_value = internal::expr_is_value<Expr>::value;
    template<class Expr> constexpr bool expr_is_field = internal::expr_is_field<Expr>::value;
    template<class Expr> constexpr bool expr_is_table = internal::expr_is_table<Expr>::value;
    template<class Expr> constexpr bool expr_is_scalar = internal::expr_is_scalar<Expr>::value;


////////////////////////////////////////////////////////////////////////////////
////////////////////////            FOR_EACH            ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template<class Pack>
        struct for_each;

        template<template<class...> class Pack, class... Ts>
        struct for_each<Pack<Ts...>>
        {
            template<class F>
            static void process(F&& f)
            {
                process(std::index_sequence_for<Ts...>{}, std::forward<F>(f));
            }

            template<std::size_t... Ns, class F>
            static void process(std::index_sequence<Ns...>, F&& f)
            {
                (std::forward<F>(f)(std::integral_constant<std::size_t, Ns>{}, Ts{}), ...);
            }
        };

        // call f for each type
        template<class... Ts, std::size_t... Ns, class F>
        void for_each_impl(std::index_sequence<Ns...>, F&& f)
        {
            using expand = int[];
            (void)expand{1, (std::forward<F>(f)(std::integral_constant<std::size_t, Ns>{}, Ts{}), 0)...};
        }

        // call f for each table or field
        template<class Entity, std::size_t... Ns, class F>
        void for_each_entity_impl(std::index_sequence<Ns...>&&, F&& f)
        {
            using expand = int[];
            (void)expand{1, ((void)std::forward<F>(f)(std::integral_constant<std::size_t, Ns>{}, typename Entity::template item_at<Ns>{}), 0)...};
        }

        // call f for each arg
        template<std::size_t... N, class F, class... Ts>
        void for_each_impl(std::index_sequence<N...>, F&& f, Ts&&... args)
        {
            using expand = int[];
            (void)expand{((void)std::forward<F>(f)(std::integral_constant<std::size_t, N>{}, std::forward<Ts>(args)), 0)...};
        }
    } // detail

    // for each on pack type
    template<class Pack, class F>
    void for_each(F&& f)
    {
        detail::for_each<Pack>::process(std::forward<F>(f));
    }

    // for_each on model entity
    template<class DB_Entity, class F>
    void for_each_entity(DB_Entity&&, F&& f)
    {
        using Entity = typename std::decay_t<DB_Entity>::Detail_::entity;
        using Ns = std::make_index_sequence<Entity::count()>;
        detail::for_each_entity_impl<Entity>(Ns{}, std::forward<F>(f));
    }

    // for each on model entity
    template<class DB_Entity, class F>
    void for_each_entity(F&& f)
    {
        for_each_entity(DB_Entity{}, std::forward<F>(f));
    }

    // for each on arguments
    template<class F, class... Ts>
    void for_each(F&& f, Ts&&... args)
    {
        detail::for_each_impl(std::index_sequence_for<Ts...>{}, std::forward<F>(f), std::forward<Ts>(args)...);
    }
////////////////////////////////////////////////////////////////////////////////
////////////////////////             TYPES              ////////////////////////
////////////////////////////////////////////////////////////////////////////////
    template<class T>
    struct is_proxy_type { static constexpr bool value = std::is_base_of<ndb::proxy_type_base, T>::value; };

    // type is native for the engine (no storage type mapping)
    template<class Engine, class T>
    struct is_native_type { static constexpr bool value = is_proxy_type<T>::value
                                                           && std::is_same_v<T, ndb::storage_type_t<Engine, T>>; };

    template<class Engine, class T>
    struct is_storage_type { static constexpr bool value = is_proxy_type<T>::value
                                                           && std::is_same_v<T, ndb::storage_type_t<Engine, T>>; };
    template<class Engine, class T>
    static constexpr bool is_storage_type_v = is_storage_type<Engine, T>::value;

    template<class T, class Database>
    struct is_custom_type { static constexpr bool value = !std::is_same<ndb::internal::custom_type_not_found, typename ndb::custom_type<T, Database>::ndb_type>::value; };

    template<class T, class Database>
    static constexpr auto is_custom_type_v = is_custom_type<T, Database>::value;


    template<class T>
    std::string type_str()
    {
        std::string output;
        #ifdef __GNUC__
            output = abi::__cxa_demangle(typeid(T).name(), 0, 0, 0);
        #else
            output = typeid(T).name();
        #endif

            /*
        boost::algorithm::replace_all(output, "struct ", "");

        boost::algorithm::replace_all(output, "ndb::expressions::", "type::");
        boost::algorithm::replace_all(output, "ndb::tables::movie<ndb::models::library_>::", "movie::");

        boost::algorithm::replace_all(output, "ndb::expression<", "expr<");

        boost::algorithm::replace_all(output, ",expr<", ",\nexpr<");

        boost::algorithm::replace_all(output, "> >", ">\n>");
             */


        return output;
    }
} // ndb

#endif // UTILITY_H_NDB