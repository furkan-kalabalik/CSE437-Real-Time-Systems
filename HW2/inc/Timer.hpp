#include "TimerInterface.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <atomic>
#include <iterator>

enum TimerType {TimerType1, TimerType2, TimerType3, TimerType4};

class TimerEvent
{
    public:
        TimerEvent(Timepoint tp, TTimerCallback cb);
        TimerEvent(Millisecs per, TTimerCallback cb);
        TimerEvent(Timepoint tp, Millisecs per, TTimerCallback cb);
        TimerEvent(TPredicate pred, Millisecs per, TTimerCallback cb);
        TimerType timerType;
        Timepoint when;
        TTimerCallback callback;
        Millisecs period;
        TPredicate predicate;
        Timepoint until;
};

struct compare{
    bool operator()(const TimerEvent& lhs, const TimerEvent& rhs)
    {
        return lhs.when > rhs.when;
    }
};

class Timer: public ITimer{
public:
	Timer();
	~Timer();
	void registerTimer(const Timepoint &tp, const TTimerCallback &cb);
	void registerTimer(const Millisecs &period, const TTimerCallback &cb);
	void registerTimer(const Timepoint &tp, const Millisecs & period, const TTimerCallback &cb);
	void registerTimer(const TPredicate &pred, const Millisecs & period, const TTimerCallback &cb);        
private:
    bool terminate;
    std::thread mainThread;
    void timerMain();
    std::priority_queue<TimerEvent, std::vector<TimerEvent>, compare> eventQueue;
    std::condition_variable cond;
    std::mutex mtx;
};