////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _ADHOCSTATEMENT_H
#define _ADHOCSTATEMENT_H

#include <future>
#include "SQLOperation.h"

typedef std::future<QueryResult> QueryResultFuture;
typedef std::promise<QueryResult> QueryResultPromise;
/*! Raw, ad-hoc query. */
class BasicStatementTask : public SQLOperation
{
    public:
        BasicStatementTask(const char* sql);
        BasicStatementTask(const char* sql, QueryResultPromise& result);
        ~BasicStatementTask();

        bool Execute() override;

    private:
        const char* m_sql;      //- Raw query to be executed
        bool m_has_result;
        QueryResultPromise m_result;
};

#endif
