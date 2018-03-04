#include "Runtime.h"
#include <cassert>

Runtime::Runtime(TimePoint init) 
	: m_start(init),
	  m_now(init)
{
}

Runtime::~Runtime()
{
}

void Runtime::schedule(TimePoint p_tp, const Callback& p_cb, const std::string& p_name)
{
	m_queue.emplace(queue_type::value_type{ p_tp, p_cb, p_name });
}

void Runtime::schedule(Duration p_d, const Callback& p_cb, const std::string& p_name)
{
	schedule(m_now + p_d, p_cb, p_name);
}

void Runtime::schedule(TimePoint p_tp, const Callback& p_cb, const void* p_scope)
{
	m_queue.emplace(queue_type::value_type{ p_tp, p_cb, p_scope });
}

void Runtime::schedule(Duration p_d, const Callback& p_cb, const void* p_scope)
{
	schedule(m_now + p_d, p_cb, p_scope);
}

void Runtime::cancel(const std::string& p_name)
{
	auto l_name_match_range = m_queue.get<by_name>().equal_range(p_name);
	m_queue.get<by_name>().erase(l_name_match_range.first, l_name_match_range.second);
}

void Runtime::cancel(const void* p_scope)
{
	if (p_scope) {
		auto l_scope_match_range = m_queue.get<by_scope>().equal_range(p_scope);
		m_queue.get<by_scope>().erase(l_scope_match_range.first, l_scope_match_range.second);
	}
}

Runtime::TimePoint Runtime::next() const
{
	Runtime::TimePoint l_next_tp = TimePoint::max();
	if (!m_queue.empty()) {
		l_next_tp = m_queue.get<by_deadline>().begin()->deadline;
	}
	return l_next_tp;
}

const Runtime::TimePoint& Runtime::now() const
{
	return m_now;
}

const Runtime::TimePoint& Runtime::start() const
{
	return m_start;
}

void Runtime::trigger(TimePoint p_tp)
{
	// require monotonic clock
	assert(p_tp >= m_now);
	m_now = p_tp;
	trigger();
}

void Runtime::trigger(Duration p_d)
{
	m_now += p_d;
	trigger();
}

int64_t Runtime::curTtiRelNow()
{
	return (m_now - m_start) / 1ms - 1;
}

int64_t Runtime::curTti(TimePoint p_tp)
{
	return (p_tp - m_start) / 1ms - 1;
}

Runtime::TimePoint Runtime::curTtiPointRelNow()
{
	return m_start + (curTtiRelNow() + 1) * 1ms;
}

Runtime::TimePoint Runtime::curTtiPoint(TimePoint p_tp)
{
	return m_start + (curTti(p_tp) + 1) * 1ms;
}

Runtime::TimePoint Runtime::nextTtiPointRelNow()
{
	return m_start + (curTtiRelNow() + 2) * 1ms;
}

Runtime::TimePoint Runtime::nextTtiPoint(TimePoint p_tp)
{
	return m_start + (curTti(p_tp) + 2) * 1ms;
}

Runtime::TimePoint Runtime::timePointForTti(uint64_t p_tti)
{
	return m_start + (p_tti + 1) * 1ms;
}

std::chrono::microseconds Runtime::microsecondsFromStart(TimePoint p_tp)
{
	return std::chrono::time_point_cast<std::chrono::microseconds>(p_tp) -
		std::chrono::time_point_cast<std::chrono::microseconds>(m_start) - 1ms;
}

void Runtime::trigger()
{
	// process queue elements separately because callback might modify runtime
	while (!m_queue.empty()) {
		auto top = m_queue.get<by_deadline>().begin();
		const auto deadline = top->deadline; // copy of deadline on purpose (erase before callback)
		if (deadline <= m_now) 
		{
			Callback cb = top->callback;
			m_queue.get<by_deadline>().erase(top);
			// callback invocation has to be last action because it might modify runtime
			cb(deadline);
		}
		else 
		{
			break;
		}
	}
}

void Runtime::reset(TimePoint p_tp)
{
	m_now = p_tp;
	queue_type l_queue;
	swap(l_queue, m_queue);

	// invoke all callbacks once so they can re-schedule
	for (auto& l_item : l_queue) {
		const auto& deadline = l_item.deadline;
		auto& callback = l_item.callback;
		// callback might modify m_queue
		callback(deadline);
	}
}
