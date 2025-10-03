////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "LogWorker.h"
#include <thread>

LogWorker::LogWorker()
{
    _cancelationToken = false;
    _workerThread = std::thread(&LogWorker::WorkerThread, this);
}

LogWorker::~LogWorker()
{
    _cancelationToken = true;

    _queue.Cancel();

    _workerThread.join();
}

void LogWorker::Enqueue(LogOperation* op)
{
    return _queue.Push(op);
}

void LogWorker::WorkerThread()
{
    while (1)
    {
        LogOperation* operation = nullptr;
        
        _queue.WaitAndPop(operation);

        if (_cancelationToken)
            return;

        operation->call();

        delete operation;
    }
}
