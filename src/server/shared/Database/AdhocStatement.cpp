////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "AdhocStatement.h"
#include "MySQLConnection.h"

/*! Basic, ad-hoc queries. */
BasicStatementTask::BasicStatementTask(const char* sql, bool async) :
m_result(nullptr)
{
    m_sql = strdup(sql);
    m_has_result = async; // If the operation is async, then there's a result
    if (async)
        m_result = new QueryResultPromise();
}

BasicStatementTask::~BasicStatementTask()
{
    free((void*)m_sql);
    if (m_has_result && m_result != nullptr)
        delete m_result;
}

bool BasicStatementTask::Execute()
{
    if (m_has_result)
    {
        ResultSet* result = m_conn->Query(m_sql);
        if (!result || !result->GetRowCount() || !result->NextRow())
        {
            delete result;
            m_result->set_value(QueryResult(NULL));
            return false;
        }

        m_result->set_value(QueryResult(result));
        return true;
    }

    return m_conn->Execute(m_sql);
}
