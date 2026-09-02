#ifndef ObjectGuid_h__
#define ObjectGuid_h__

#include "ByteBuffer.h"
#include "Guid.h"

inline ByteBuffer& operator<<(ByteBuffer& buf, ObjectGuid const& guid)
{
    buf << uint64(guid);
    return buf;
}

inline ByteBuffer& operator>>(ByteBuffer& buf, ObjectGuid& guid)
{
    uint64 v = 0;
    buf >> v;
    guid = ObjectGuid(v);
    return buf;
}

#endif
