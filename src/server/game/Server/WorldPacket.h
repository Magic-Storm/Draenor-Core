////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef WORLDPACKET_H
#define WORLDPACKET_H

#include "Common.h"
#include "Opcodes.h"
#include "ByteBuffer.h"

struct z_stream_s;

extern std::mutex gPacketProfilerMutex;
extern std::map<uint32, uint32> gPacketProfilerData;

class WorldPacket : public ByteBuffer
{
    public:
                                                            // just container for later use
        WorldPacket() : ByteBuffer(0), m_opcode((uint32)UNKNOWN_OPCODE), _connection(CONNECTION_TYPE_DEFAULT)
        {
        }

        WorldPacket(uint32 opcode, size_t res = 200, ConnectionType connection = CONNECTION_TYPE_DEFAULT) : ByteBuffer(res), m_opcode(opcode), _connection(connection)
        {
        }
                                                            // copy constructor
        WorldPacket(WorldPacket const& packet) : ByteBuffer(packet), m_opcode(packet.m_opcode), _connection(packet._connection)
        {
        }

        WorldPacket(WorldPacket&& packet) : ByteBuffer(std::move(packet)), m_opcode(packet.m_opcode), _connection(packet._connection)
        {
        }

        WorldPacket& operator=(WorldPacket const& packet)
        {
            ByteBuffer::operator=(packet);
            m_opcode = packet.m_opcode;
            _connection = packet._connection;
            return *this;
        }

        void Initialize(uint32 opcode, size_t newres = 200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
            m_BaseSize = newres;
        }

        void OnSend();

        uint32 GetOpcode() const { return m_opcode; }
        void SetOpcode(uint32 opcode) { m_opcode = opcode; }
        ConnectionType GetConnection() const { return _connection; }
        void SetConnection(ConnectionType connection) { _connection = connection; }
        void Compress(z_stream_s* compressionStream);
        void Compress(z_stream_s* compressionStream, WorldPacket const* source);

    protected:
        uint32 m_opcode;
        ConnectionType _connection;
        void Compress(void* dst, uint32 *dst_size, const void* src, int src_size);
        z_stream_s* _compressionStream;
};
#endif
