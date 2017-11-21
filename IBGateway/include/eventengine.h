#ifndef EVENTENGINE_H
#define EVENTENGINE_H

#ifdef WIN32
#ifdef  EVENTENGINE_LIBRARY
#define EVENTENGINE_API __declspec(dllexport)
#else
#define EVENTENGINE_API __declspec(dllimport)
#endif
#endif

#ifdef linux
#define EVENTENGINE_API
#endif

#include<string>
#include<queue>
#include<mutex>
#include<memory>
#include<condition_variable>
#include<map>
#include<thread>
#include<atomic>
#include<chrono>
#include<time.h>
#include"utils.h"
#include"structs.hpp"

template<typename EVENT>
class SynQueue
{
public:
    void push(std::shared_ptr<EVENT>Event)
    {
        std::unique_lock<std::recursive_mutex>lck(mutex);
        queue.push(Event);
        cv.notify_all();
    }
    std::shared_ptr<Event> take()
    {
        std::unique_lock<std::recursive_mutex>lck(mutex);
        while (queue.empty()) {
            cv.wait(lck);
        }
        std::shared_ptr<Event>e = queue.front();
        queue.pop();
        return e;
    }

private:
    std::recursive_mutex mutex;
    std::queue<std::shared_ptr<EVENT>>queue;
    std::condition_variable_any cv;
};


typedef std::function<void(std::shared_ptr<Event>)> TASK;

class EVENTENGINE_API EventEngine
{
public:
    EventEngine();
    ~EventEngine();
    void startEngine();
    void stopEngine();
    void regEvent(const std::string &eventtype,const TASK &task);
    void unregEvent(const std::string &eventtype);
    void doTask();
    void put(std::shared_ptr<Event>e);
    void timer();
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::vector<std::thread*>*m_task_pool = nullptr;
    SynQueue<Event>*m_event_queue = nullptr;
    std::multimap<std::string, TASK>*m_task_map = nullptr;
    std::thread* m_timer_thread = nullptr;
    std::atomic<bool>m_active;
};

#endif // EVENTENGINE_H
