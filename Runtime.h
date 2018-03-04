#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <functional>
#include <chrono>
#include <string>

using namespace std::chrono_literals;

class Runtime
{
public:
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration = Clock::duration;
	using Callback = std::function<void(TimePoint)>;

	Runtime() = default;
	explicit Runtime(TimePoint p_init);

	~Runtime();

	void schedule(TimePoint p_tp, const Callback& p_cb, const std::string& p_name = "");
	void schedule(Duration p_d, const Callback& p_cb, const std::string& p_name = "");
	void schedule(TimePoint p_tp, const Callback& p_cb, const void* p_scope);
	void schedule(Duration p_d, const Callback& p_cb, const void* p_scope);

	void cancel(const std::string& p_name);
	void cancel(const void* p_scope);

	//Get time point of next scheduled event
	TimePoint next() const;
	const TimePoint& now() const;
	const TimePoint& start() const;

	void trigger(TimePoint p_tp);
	void trigger(Duration p_d);

	void reset(TimePoint tp);

	int64_t curTtiRelNow();
	int64_t curTti(TimePoint p_tp);

	TimePoint curTtiPointRelNow();
	TimePoint curTtiPoint(TimePoint p_tp);

	TimePoint nextTtiPointRelNow();
	TimePoint nextTtiPoint(TimePoint p_tp);

	TimePoint timePointForTti(uint64_t p_tti);

	std::chrono::microseconds microsecondsFromStart(TimePoint p_tp);

private:
	struct ScheduledCallback
	{
		ScheduledCallback(TimePoint p_tp, const Callback& p_cb, const std::string& p_name) :
			deadline(p_tp), callback(p_cb), name(p_name), scope(nullptr) 
		{}
		ScheduledCallback(TimePoint p_tp, const Callback& p_cb, const void* p_scope) :
			deadline(p_tp), callback(p_cb), scope(p_scope) 
		{}

		ScheduledCallback(const ScheduledCallback&) = delete;
		ScheduledCallback& operator=(const ScheduledCallback&) = delete;

		ScheduledCallback(ScheduledCallback&&) = default;
		ScheduledCallback& operator=(ScheduledCallback&&) = default;

		TimePoint deadline;
		Callback callback;
		std::string name;
		const void* scope;
	};

	struct by_deadline {};
	using time_index = boost::multi_index::ordered_non_unique<
		boost::multi_index::tag<by_deadline>,
		boost::multi_index::member<ScheduledCallback, TimePoint, &ScheduledCallback::deadline>>;
	struct by_name {};
	using name_index = boost::multi_index::hashed_non_unique<
		boost::multi_index::tag<by_name>,
		boost::multi_index::member<ScheduledCallback, std::string, &ScheduledCallback::name>>;
	struct by_scope {};
	using scope_index = boost::multi_index::hashed_non_unique<
		boost::multi_index::tag<by_scope>,
		boost::multi_index::member<ScheduledCallback, const void*, &ScheduledCallback::scope>>;
	using queue_type = boost::multi_index_container<ScheduledCallback,
		boost::multi_index::indexed_by<time_index, name_index, scope_index>>;

	void trigger();

	TimePoint m_start;
	TimePoint m_now;
	queue_type m_queue;
};

