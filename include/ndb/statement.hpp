#ifndef STATEMENT_NDB
#define STATEMENT_NDB

#include <ndb/expression.hpp>

namespace ndb
{
    static constexpr ndb::expression<expressions::statement_> statement;

    struct dynamic_statement
    {
        std::string output_;

        template<class Expr>
        dynamic_statement& operator<<(const Expr& e)
        {
            //Expr::make(output_);
            return *this;
        }

        auto& native()
        {
            return output_;
        }
    };
} // ndb


#endif // STATEMENT_NDB