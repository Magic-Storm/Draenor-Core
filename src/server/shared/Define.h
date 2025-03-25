////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef TRINITY_DEFINE_H
#define TRINITY_DEFINE_H

#if COMPILER_GNU == COMPILER_GNU
#  if !defined(__STDC_FORMAT_MACROS)
#    define __STDC_FORMAT_MACROS
#  endif
#endif

#include <cstddef>
#include <cinttypes>

#include "CompilerDefs.h"

#define TRINITY_LITTLEENDIAN 0
#define TRINITY_BIGENDIAN    1

#if !defined(TRINITY_ENDIAN)
#  if defined (BOOST_ENDIAN_BIG_BYTE)
#    define TRINITY_ENDIAN TRINITY_BIGENDIAN
#  else
#    define TRINITY_ENDIAN TRINITY_LITTLEENDIAN
#  endif
#endif

#if PLATFORM == PLATFORM_WINDOWS
#  define TRINITY_PATH_MAX MAX_PATH
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#else //PLATFORM != PLATFORM_WINDOWS
#  define TRINITY_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#  define DECLSPEC_DEPRECATED
#endif //PLATFORM

#if !defined(COREDEBUG)
#  define TRINITY_INLINE inline
#else //COREDEBUG
#  if !defined(TRINITY_DEBUG)
#    define TRINITY_DEBUG
#  endif //TRINITY_DEBUG
#  define TRINITY_INLINE
#endif //!COREDEBUG

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))
#  define ATTR_DEPRECATED __attribute__((deprecated))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#  define ATTR_DEPRECATED
#endif //COMPILER == COMPILER_GNU

#ifdef TRINITY_API_USE_DYNAMIC_LINKING
#  if TRINITY_COMPILER == TRINITY_COMPILER_MICROSOFT
#    define TC_API_EXPORT __declspec(dllexport)
#    define TC_API_IMPORT __declspec(dllimport)
#  elif TRINITY_COMPILER == TRINITY_COMPILER_GNU
#    define TC_API_EXPORT __attribute__((visibility("default")))
#    define TC_API_IMPORT
#  else
#    error compiler not supported!
#  endif
#else
#  define TC_API_EXPORT
#  define TC_API_IMPORT
#endif

#ifdef TRINITY_API_EXPORT_SHARED
 #  define TC_SHARED_API TC_API_EXPORT
 #else
 #  define TC_SHARED_API TC_API_IMPORT
 #endif
 
#define UI64FMTD PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD PRId64
#define SI64LIT(N) INT64_C(N)

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

enum DBCFormer
{
    FT_NA = 'x',                                              //not used or unknown, 4 byte size
    FT_NA_BYTE = 'X',                                         //not used or unknown, byte
    FT_STRING = 's',                                          //char*
    FT_FLOAT = 'f',                                           //float
    FT_INT = 'i',                                             //uint32
    FT_BYTE = 'b',                                            //uint8
    FT_SHORT = 'w',                                           //uint16 aka word
    FT_SORT = 'd',                                            //sorted by this field, field is not included
    FT_INDEX = 'n',                                             //the same, but parsed to data
    FT_LOGIC = 'l',                                           //Logical (boolean)
    FT_SQL_PRESENT = 'p',                                     //Used in sql format to mark column present in sql dbc
    FT_SQL_ABSENT = 'a',                                      //Used in sql format to mark column absent in sql dbc
    FT_SQL_SUP = 'o',                                         // Supp sql row (not in dbc)

    FT_ARR_2 = '2',
    FT_ARR_3 = '3',
    FT_ARR_4 = '4',
    FT_ARR_5 = '5',
    FT_ARR_6 = '6',
    FT_ARR_7 = '7',
    FT_ARR_8 = '8',

    FT_END = '\0'
};
#endif //TRINITY_DEFINE_H
