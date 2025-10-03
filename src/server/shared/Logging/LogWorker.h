////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef LOGWORKER_H
#define LOGWORKER_H

#include <atomic>
#include <thread>

#include "LogOperation.h"
#include "ProducerConsumerQueue.h"


class LogWorker
{
    public:
        LogWorker();
        ~LogWorker();

        void Enqueue(LogOperation *op);

    private:
        ProducerConsumerQueue<LogOperation*> _queue;

        void WorkerThread();
        std::thread _workerThread;

        std::atomic_bool _cancelationToken;
};

#endif
