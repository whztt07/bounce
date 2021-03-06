/*
* Copyright (c) 2016-2016 Irlan Robson http://www.irlan.net
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <testbed/framework/profiler.h>

Profiler* g_profiler = nullptr;
ProfilerListener* g_profilerListener = nullptr;

Profiler::Profiler()
{
	m_top = nullptr;
}

Profiler::~Profiler()
{
}

void Profiler::PushEvent(const char* name)
{
	m_time.Update();

	ProfilerEvent e;
	e.tid = -1;
	e.pid = -1;
	e.t0 = m_time.GetCurrentMilis();
	e.t1 = 0.0;
	e.name = name;
	e.parent = m_top;

	ProfilerEvent* back = m_events.Push(e);
	B3_ASSERT(back);
	m_top = back;
}

void Profiler::PopEvent()
{
	B3_ASSERT(m_top);
	B3_ASSERT(m_top->t1 == 0.0);

	m_time.Update();
	m_top->t1 = m_time.GetCurrentMilis();
	B3_ASSERT(m_top->t1 != 0.0);
	m_top = m_top->parent;
}

void Profiler::Begin()
{
	// If this assert is hit then it means Profiler::End hasn't been called.
	B3_ASSERT(m_events.IsEmpty());
	B3_ASSERT(m_top == NULL);
}

void Profiler::End(ProfilerListener* listener)
{
	if (listener)
	{
		listener->BeginEvents();
	}

	while (m_events.IsEmpty() == false)
	{
		ProfilerEvent e = m_events.Front();

		m_events.Pop();

		if (listener)
		{
			listener->BeginEvent(e.tid, e.pid, e.name, e.t0);

			listener->EndEvent(e.tid, e.pid, e.name, e.t1);

			listener->Duration(e.name, e.t1 - e.t0);
		}
	}

	B3_ASSERT(m_events.IsEmpty());
	B3_ASSERT(m_top == NULL);

	if (listener)
	{
		listener->EndEvents();
	}
}