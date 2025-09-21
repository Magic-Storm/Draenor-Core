/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SocketMgr_h__
#define SocketMgr_h__

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <memory>

template<class SocketType>
class SocketMgr
{
public:
    virtual ~SocketMgr() = default;

    virtual bool StartNetwork(boost::asio::io_service& service, std::string const& bindIp, uint16 port, int threadCount = 1) = 0;
    virtual void StopNetwork() = 0;

protected:
    virtual int GetNetworkThreadCount() const { return 1; }
    virtual SocketType* GetSocketForAccept() { return nullptr; }
    virtual void OnSocketOpen(SocketType&& sock, uint32 threadIndex) = 0;
};

template<class SocketType>
class NetworkThread
{
public:
    virtual ~NetworkThread() = default;
};

// Forward declaration for acceptor
template<class SocketType>
class SocketAcceptor;

template<class SocketType>
class SocketAcceptor
{
public:
    SocketAcceptor() = default;
    virtual ~SocketAcceptor() = default;

    void SetSocketFactory(std::function<SocketType*()> factory) 
    {
        _socketFactory = factory;
    }

    template<void(*Callback)(typename SocketType::socket_type&&, uint32)>
    void AsyncAcceptWithCallback()
    {
        // Placeholder implementation
    }
};

#endif // SocketMgr_h__
