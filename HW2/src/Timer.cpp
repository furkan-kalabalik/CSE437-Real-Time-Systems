#include "Timer.hpp"
#include <iostream>

using namespace std;

TimerEvent::TimerEvent(Timepoint tp, TTimerCallback cb)
{
    when = tp;
    callback = cb;
    timerType = TimerType1;
}

TimerEvent::TimerEvent(Millisecs per, TTimerCallback cb)
{
    when = CLOCK::now() + per;
    callback = cb;
    timerType = TimerType2;
    period = per;
}

TimerEvent::TimerEvent(Timepoint tp, Millisecs per, TTimerCallback cb)
{
    when = CLOCK::now() + per;
    callback = cb;
    timerType = TimerType3;
    period = per;
    until = tp;
}

TimerEvent::TimerEvent(TPredicate pred, Millisecs per, TTimerCallback cb){
    when = CLOCK::now() + per;
    callback = cb;
    timerType = TimerType4;
    period = per;
    predicate = pred;
}

Timer::Timer()
{
    mainThread = std::thread(&Timer::timerMain, this);
    terminate = false;
}

Timer::~Timer()
{
    terminate = true;
    if(mainThread.joinable())
        mainThread.join();
}

void Timer::registerTimer(const Timepoint &tp, const TTimerCallback &cb){
	std::unique_lock<std::mutex> lck(mtx);
    TimerEvent te = TimerEvent(tp, cb);
    eventQueue.push(te);
    lck.unlock();
    cond.notify_all();
}

void Timer::registerTimer(const Millisecs &period, const TTimerCallback &cb)
{
    std::unique_lock<std::mutex> lck(mtx);
    TimerEvent te = TimerEvent(period, cb);
    eventQueue.push(te);
    lck.unlock();
    cond.notify_all();
}

void Timer::registerTimer(const Timepoint &tp, const Millisecs & period, const TTimerCallback &cb)
{
    std::unique_lock<std::mutex> lck(mtx);
    TimerEvent te = TimerEvent(tp, period, cb);
    eventQueue.push(te);
    lck.unlock();
    cond.notify_all();
}

void Timer::registerTimer(const TPredicate &pred, const Millisecs & period, const TTimerCallback &cb)
{
    std::unique_lock<std::mutex> lck(mtx);
    TimerEvent te = TimerEvent(pred, period, cb);
    eventQueue.push(te);
    lck.unlock();
    cond.notify_all();
}

void Timer::timerMain()
{
    std::unique_lock<std::mutex> lck(mtx);
    cout << "Timer main is working!" << endl;
    while(!terminate)
    {
        if(eventQueue.empty())
        {
                cond.wait(lck);
        }
        else
        {
            TimerEvent curr = eventQueue.top();
            
            if(CLOCK::now() >= curr.when)
            {
                eventQueue.pop();

                lck.unlock();
                curr.callback();
                lck.lock();
                if(curr.timerType == TimerType2)
                {
                    curr.when = CLOCK::now() + curr.period;
                    eventQueue.push(curr);
                }
                else if(curr.timerType == TimerType3)
                {
                    if(CLOCK::now()+curr.period <= curr.until)
                    {
                        curr.when = CLOCK::now() + curr.period;
                        eventQueue.push(curr);
                    }
                }
                else if(curr.timerType == TimerType4)
                {
                    if(curr.predicate())
                    {
                        curr.when = CLOCK::now() + curr.period;
                        eventQueue.push(curr);
                    }
                }
            }
            else
                cond.wait_until(lck, curr.when);
        }
    }
}