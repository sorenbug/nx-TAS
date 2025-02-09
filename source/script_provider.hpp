#pragma once
#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <istream>
#include "script_init.hpp"

class ScriptProvider
{
private:
    mutable std::mutex mutex;
    std::queue<std::shared_ptr<struct controlMsg>> internalQueue;

protected:
    void pushToQueue(std::shared_ptr<struct controlMsg> msg)
    {
        std::lock_guard<std::mutex> guard(mutex);
        return internalQueue.push(msg);
    }

    std::shared_ptr<struct controlMsg> pullFromQueue()
    {
        std::lock_guard<std::mutex> guard(mutex);
        std::shared_ptr<struct controlMsg> msg = internalQueue.front();
        internalQueue.pop();
        return msg;
    }

public:
    bool queueIsEmpty()
    {
        std::lock_guard<std::mutex> guard(mutex);
        return internalQueue.empty();
    }
    int queueSize()
    {
        std::lock_guard<std::mutex> guard(mutex);
        return internalQueue.size();
    }
    virtual bool isGood() { return false; };

    virtual std::shared_ptr<struct controlMsg> nextLine();
    virtual bool hasNextLine();
    virtual void populateQueue();

};

class LineStreamScriptProvider: public ScriptProvider
{
private:
    std::istream& stream;

protected:
    virtual void afterEOF()
    {}
    virtual bool shouldPopulate()
    {
        return queueSize() < 15;
    }

public:
    LineStreamScriptProvider(std::istream& mStream)
    : stream(mStream)
    {}

    bool isGood() { return stream.good(); }
    bool hasNextLine();
    void populateQueue();

};

class LineFileScriptProvider: public LineStreamScriptProvider
{
    std::ifstream stream;

protected:
    void afterEOF()
    {
        stream.close();
    }
    bool shouldPopulate()
    {
        return stream.is_open() && queueSize() < 15;
    }

public:
    LineFileScriptProvider(std::string fileName)
    : LineStreamScriptProvider(stream), stream(fileName)
    {}

};
