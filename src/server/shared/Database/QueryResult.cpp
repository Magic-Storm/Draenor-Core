////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "DatabaseEnv.h"
#include "Log.h"

ResultSet::ResultSet(MYSQL_RES *result, MYSQL_FIELD *fields, uint64 rowCount, uint32 fieldCount) :
_rowCount(rowCount),
_fieldCount(fieldCount),
_result(result),
_fields(fields)
{
    _currentRow = new Field[_fieldCount];
#ifdef TRINITY_DEBUG
    for (uint32 i = 0; i < _fieldCount; i++)
        _currentRow[i].SetMetadata(&_fields[i], i);
#endif
}

PreparedResultSet::PreparedResultSet(MYSQL_STMT* stmt, MYSQL_RES *result, uint64 rowCount, uint32 fieldCount) :
m_rowCount(rowCount),
m_rowPosition(0),
m_fieldCount(fieldCount),
m_rBind(NULL),
m_stmt(stmt),
m_metadataResult(result),
m_isNull(NULL),
m_length(NULL)
{
    if (!m_metadataResult)
        return;

    if (m_stmt->bind_result_done)
    {
        delete[] m_stmt->bind->length;
        delete[] m_stmt->bind->is_null;
    }

    m_rBind = new MYSQL_BIND[m_fieldCount];
    m_isNull = new bool[m_fieldCount];
    m_length = new unsigned long[m_fieldCount];

    memset(m_isNull, 0, sizeof(bool) * m_fieldCount);
    memset(m_rBind, 0, sizeof(MYSQL_BIND) * m_fieldCount);
    memset(m_length, 0, sizeof(unsigned long) * m_fieldCount);

    //- This is where we store the (entire) resultset
    if (mysql_stmt_store_result(m_stmt))
    {
        TC_LOG_WARN("sql.sql", "%s:mysql_stmt_store_result, cannot bind result from MySQL server. Error: %s", __FUNCTION__, mysql_stmt_error(m_stmt));
        return;
    }

    m_rowCount = mysql_stmt_num_rows(m_stmt);

    //- This is where we prepare the buffer based on metadata
    MYSQL_FIELD* field = mysql_fetch_fields(m_metadataResult);
    for (uint32 i = 0; i < m_fieldCount; ++i)
    {
        size_t size = Field::SizeForType(&field[i]);

        m_rBind[i].buffer_type = field[i].type;
        m_rBind[i].buffer = new char[size * m_rowCount];
        m_rBind[i].buffer_length = size;
        m_rBind[i].length = &m_length[i];
        m_rBind[i].is_null = &m_isNull[i];
        m_rBind[i].error = NULL;
        m_rBind[i].is_unsigned = field[i].flags & UNSIGNED_FLAG;
    }

    //- This is where we bind the bind the buffer to the statement
    if (mysql_stmt_bind_result(m_stmt, m_rBind))
    {
        TC_LOG_WARN("sql.sql", "%s:mysql_stmt_bind_result, cannot bind result from MySQL server. Error: %s", __FUNCTION__, mysql_stmt_error(m_stmt));
        mysql_stmt_free_result(m_stmt);
        CleanUp();
        delete[] m_isNull;
        delete[] m_length;
        return;
    }

    m_rowCount = mysql_stmt_num_rows(m_stmt);

    m_rows.resize(uint32(m_rowCount) * m_fieldCount);
    while (_NextRow())
    {
        for (uint32 fIndex = 0; fIndex < m_fieldCount; ++fIndex)
        {
            unsigned long buffer_length = m_rBind[fIndex].buffer_length;
            unsigned long fetched_length = *m_rBind[fIndex].length;
            if (!*m_rBind[fIndex].is_null)
            {
                void* buffer = m_stmt->bind[fIndex].buffer;
                switch (m_rBind[fIndex].buffer_type)
                {
                    case MYSQL_TYPE_TINY_BLOB:
                    case MYSQL_TYPE_MEDIUM_BLOB:
                    case MYSQL_TYPE_LONG_BLOB:
                    case MYSQL_TYPE_BLOB:
                    case MYSQL_TYPE_STRING:
                    case MYSQL_TYPE_VAR_STRING:
                        // warning - the string will not be null-terminated if there is no space for it in the buffer
                        // when mysql_stmt_fetch returned MYSQL_DATA_TRUNCATED
                        // we cannot blindly null-terminate the data either as it may be retrieved as binary blob and not specifically a string
                        // in this case using Field::GetCString will result in garbage
                        // TODO: remove Field::GetCString and use boost::string_ref (currently proposed for TS as string_view, maybe in C++17)
                        if (fetched_length < buffer_length)
                            *((char*)buffer + fetched_length) = '\0';
                        break;
                    default:
                        break;
                }

                m_rows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetByteValue(
                                                                                   buffer,
                                                                                   m_rBind[fIndex].buffer_type,
                                                                                   fetched_length);

                // move buffer pointer to next part
                m_stmt->bind[fIndex].buffer = (char*)buffer + buffer_length;
            }
            else
            {
                m_rows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetByteValue(
                                                                                   nullptr,
                                                                                   m_rBind[fIndex].buffer_type,
                                                                                   *m_rBind[fIndex].length);
            }
            
#ifdef TRINITY_DEBUG
            m_rows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetMetadata(&field[fIndex], fIndex);
#endif
        }
        m_rowPosition++;
    }
    m_rowPosition = 0;

    /// All data is buffered, let go of mysql c api structures
    mysql_stmt_free_result(m_stmt);
}

ResultSet::~ResultSet()
{
    CleanUp();
}

PreparedResultSet::~PreparedResultSet()
{
    CleanUp();
}

bool ResultSet::NextRow()
{
    MYSQL_ROW row;

    if (!_result)
        return false;

    row = mysql_fetch_row(_result);
    if (!row)
    {
        CleanUp();
        return false;
    }

    for (uint32 i = 0; i < _fieldCount; i++)
        _currentRow[i].SetStructuredValue(row[i], _fields[i].type);

    return true;
}

bool PreparedResultSet::NextRow()
{
    /// Only updates the m_rowPosition so upper level code knows in which element
    /// of the rows vector to look
    if (++m_rowPosition >= m_rowCount)
        return false;

    return true;
}

bool PreparedResultSet::_NextRow()
{
    /// Only called in low-level code, namely the constructor
    /// Will iterate over every row of data and buffer it
    if (m_rowPosition >= m_rowCount)
        return false;

    int retval = mysql_stmt_fetch( m_stmt );

    if (!retval || retval == MYSQL_DATA_TRUNCATED)
        retval = true;

    if (retval == MYSQL_NO_DATA)
        retval = false;

    return retval;
}

void ResultSet::CleanUp()
{
    if (_currentRow)
    {
        delete [] _currentRow;
        _currentRow = NULL;
    }

    if (_result)
    {
        mysql_free_result(_result);
        _result = NULL;
    }
}

void PreparedResultSet::CleanUp()
{
    if (m_metadataResult)
        mysql_free_result(m_metadataResult);

    if (m_rBind)
    {
        for (uint32 i = 0; i < m_fieldCount; ++i)
        {
            delete[]((char*)m_rBind[i].buffer);
            m_rBind[i].buffer = nullptr;
        }

        delete[] m_rBind;
        m_rBind = nullptr;
    }
}
