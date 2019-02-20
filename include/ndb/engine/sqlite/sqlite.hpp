#ifndef ENGINE_SQLITE_SQLITE_H_NDB
#define ENGINE_SQLITE_SQLITE_H_NDB

#include <sqlite3.h>

#include <ndb/engine/basic.hpp>
#include <ndb/engine/sqlite/connection.hpp>
#include <ndb/error.hpp>
#include <ndb/value.hpp>
#include <ndb/result.hpp>
#include <ndb/setup.hpp>

#include <experimental/filesystem>
#include <unordered_map>
#include <ndb/expression/sql/native.hpp>
#include <ndb/expression/sql/type.hpp>
#include <ndb/expression/category.hpp>
#include <ndb/function/sql.hpp>

namespace ndb
{
    template<class Database>
    class sqlite_query;

    class sqlite : public basic_engine<sqlite>
    {
    public:
        using statement_type = std::string;

        inline sqlite() = default;

        template<class Database>
        inline auto exec(const statement_type& str_statement) const;

        template<class Database, class Result_type = ndb::line<Database>>
        inline auto exec(const sqlite_query<Database>& query) const;

        template<class Database, class Query_option, class Expr>
        inline auto exec(const Expr& expr) const;

        template<class Database>
        inline void make();

        template<class Database>
        auto last_id() const
        {
            return sqlite3_last_insert_rowid(connection<Database>());
        }

        template<class Expr>
        inline static std::string to_string(const Expr&);

        inline static constexpr expression_categories expr_category = expression_categories::sql;
    };
} // ndb

#include <ndb/engine/sqlite/sqlite.tpp>

#endif // ENGINE_SQLITE_SQLITE_H_NDB