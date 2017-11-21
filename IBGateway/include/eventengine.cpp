#include "eventengine.h"
EventEngine::EventEngine()
{
    m_active = false;
    m_event_queue = new SynQueue<Event>;
    m_task_map = new std::multimap<std::string, TASK>;
}

EventEngine::~EventEngine()
{
    stopEngine();
    if (m_event_queue) {
        delete m_event_queue;
        m_event_queue = nullptr;
    }
    if (m_task_map) {
        delete m_task_map;
        m_task_map = nullptr;
    }
}

void EventEngine::startEngine()
{
    m_active = true;
    m_timer_thread = new std::thread(std::bind(&EventEngine::timer, this));
    m_task_pool = new std::vector<std::thread*>;
    std::function<void()>f = std::bind(&EventEngine::doTask, this);
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        std::thread* thread_worker = new std::thread(f);
        m_task_pool->push_back(thread_worker);
    }
}

void EventEngine::stopEngine()
{
    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        std::shared_ptr<Event_Exit>e = std::make_shared<Event_Exit>();
        put(e);
    }

    m_active = false;
    if (m_timer_thread) {
        m_timer_thread->join();
        delete  m_timer_thread;
        m_timer_thread = nullptr;
    }

    if (m_task_pool != nullptr) {
        for (std::vector<std::thread*>::iterator it = m_task_pool->begin(); it != m_task_pool->end(); it++) {
            (*it)->join();
            delete (*it);
        }
        delete  m_task_pool;
        m_task_pool = nullptr;
    }
}

void EventEngine::regEvent(const std::string &eventtype, const TASK &task)
{
    std::unique_lock<std::mutex>lck(m_mutex);
    m_task_map->insert(std::make_pair(eventtype, task));
}

void EventEngine::unregEvent(const std::string &eventtype)
{
    std::unique_lock<std::mutex>lck(m_mutex);
    m_task_map->erase(eventtype);
}

void EventEngine::doTask()
{
    while (m_active)
    {
        std::shared_ptr<Event>e = m_event_queue->take();
        e->GetEventType();
        if (e->GetEventType() == EVENT_QUIT)
        {
            break;
        }
        std::pair<std::multimap<std::string, TASK>::iterator, std::multimap<std::string, TASK>::iterator>ret;
        m_mutex.lock();
        ret = m_task_map->equal_range(e->GetEventType());
        m_mutex.unlock();
        for (std::multimap<std::string, TASK>::iterator it = ret.first; it != ret.second; ++it)
        {
            TASK t;
            t = it->second;
            t(e);
        }
    }
}

void EventEngine::put(std::shared_ptr<Event> e)
{
    m_event_queue->push(e);
}

void EventEngine::timer()
{
    while (m_active)
    {
        std::shared_ptr<Event_Timer>e = std::make_shared<Event_Timer>();
        put(e);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
