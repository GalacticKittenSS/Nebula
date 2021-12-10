#include "nbpch.h"
#include "Time.h"

namespace Nebula {
	std::chrono::time_point<Time::highResClock> Time::mStart;
	Timestep Time::m_Timestep;
	float Time::m_Time;
	float Time::m_LastFrameTime;
}