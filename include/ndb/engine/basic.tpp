#include <ndb/error.hpp>
#include <ndb/engine/sqlite/connection.hpp>
#include <ndb/id.hpp>
#include <ndb/utility.hpp>
#include <ndb/prepared_query.hpp>

namespace ndb
{
    template<class Engine>
    Engine* basic_engine<Engine>::instance_ = nullptr;

    template<class Engine>
    Engine& basic_engine<Engine>::instance()
    {
        if (instance_ == nullptr) ndb_error("engine not initialized");
        return *instance_;
    }

    template<class Engine>
    basic_engine<Engine>::basic_engine()
    {
        if (instance_ == nullptr)
        {
            instance_ = static_cast<Engine*>(this);
        }
        else ndb_error("engine already initialized");
    }

    template<class Engine>
    basic_engine<Engine>::~basic_engine()
    {
        instance_ = nullptr;
    }

    template<class Engine>
    template<class Database>
    ndb::engine_connection<Engine>& basic_engine<Engine>::connection() const
    {
        if (!connections_.count(ndb::name<Database>())) ndb_error("database connection not found : " + ndb::name<Database>());
        return *connections_.at(ndb::name<Database>()).get();
    }

    template<class Engine>
    template<class Database>
    void basic_engine<Engine>::make()
    {
        static_cast<Engine*>(this)->template make<Database>();
    }

    template<class Engine>
    template<class Database>
    void basic_engine<Engine>::connect(ndb::connection_param params)
    {
        // retrieve db name
        params.db_name = ndb::name<Database>();

        // create connection
        auto conn = basic_connection<Engine>::make(std::move(params));
        connections_.emplace(ndb::name<Database>(), std::move(conn));

        // create model
        make<Database>();

        // create prepared queries
        prepared_query<Database>::make();
    }
} // ndb