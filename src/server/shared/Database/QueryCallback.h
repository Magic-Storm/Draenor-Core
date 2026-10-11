/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * Ported from dio85/TCWoD 2c69d92.
 */

#ifndef _QUERY_CALLBACK_H
#define _QUERY_CALLBACK_H

#include "Define.h"
#include "QueryResult.h"
#include <functional>
#include <future>
#include <list>
#include <queue>

class TC_DATABASE_API QueryCallback
{
public:
    explicit QueryCallback(std::future<QueryResult>&& result);
    explicit QueryCallback(std::future<PreparedQueryResult>&& result);
    QueryCallback(QueryCallback&& right);
    QueryCallback& operator=(QueryCallback&& right);
    ~QueryCallback();

    QueryCallback&& WithCallback(std::function<void(QueryResult)>&& callback);
    QueryCallback&& WithPreparedCallback(std::function<void(PreparedQueryResult)>&& callback);

    QueryCallback&& WithChainingCallback(std::function<void(QueryCallback&, QueryResult)>&& callback);
    QueryCallback&& WithChainingPreparedCallback(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback);

    void SetNextQuery(QueryCallback&& next);

    enum Status
    {
        NotReady,
        NextStep,
        Completed
    };

    Status InvokeIfReady();

private:
    QueryCallback(QueryCallback const& right) = delete;
    QueryCallback& operator=(QueryCallback const& right) = delete;

    template<typename T> friend void ConstructActiveMember(T* obj);
    template<typename T> friend void DestroyActiveMember(T* obj);
    template<typename T> friend void MoveFrom(T* to, T&& from);

    union
    {
        std::future<QueryResult> _string;
        std::future<PreparedQueryResult> _prepared;
    };
    bool _isPrepared;

    struct QueryCallbackData;
    std::queue<QueryCallbackData, std::list<QueryCallbackData>> _callbacks;
};

#endif // _QUERY_CALLBACK_H
