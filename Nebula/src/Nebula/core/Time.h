#pragma once

namespace Nebula {
	class Timestep {
	public:
		Timestep(float time = 0.0f) : m_Time(time) { }

		operator float() const { return m_Time; }

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }
	private:
		float m_Time;
	};

	class Timer {
	public:
		Timer() {
			reset();
		}

		void reset() {
			mStart = highResClock::now();
		}

		float elapsed() {
			return std::chrono::duration_cast<milliseconds_type>(highResClock::now() - mStart).count() / 1000.0f;
		}
	private:
		typedef std::chrono::high_resolution_clock highResClock;
		typedef std::chrono::duration<float, std::milli> milliseconds_type;
		std::chrono::time_point<highResClock> mStart;
	};
}