/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * Ported from dio85/TCWoD d740184 for MySQL 8 client headers.
 */

#ifndef MySQLWorkaround_h__
#define MySQLWorkaround_h__

#ifdef _WIN32 // hack for broken mysql.h not including the correct winsock header for SOCKET definition
#include <winsock2.h>
#endif
#include <mysql.h>

#endif // MySQLWorkaround_h__
