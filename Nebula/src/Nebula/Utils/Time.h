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

	class Time {
	public:
		static void Start() {
			mStart = highResClock::now();
		}
		
		inline static float Elapsed() {
			return std::chrono::duration_cast<milliseconds_type>(highResClock::now() - mStart).count() / 1000.0f;
		}
		
		inline static float DeltaTime() { return m_Timestep; }
		
		static void Update() { 
			m_Timestep = Elapsed() - m_LastFrameTime;
			m_LastFrameTime = Elapsed();
		}
	private:
		typedef std::chrono::high_resolution_clock highResClock;
		typedef std::chrono::duration<float, std::milli> milliseconds_type;
		
		static std::chrono::time_point<highResClock> mStart;
		static float m_Time;
		static Timestep m_Timestep;
		static float m_LastFrameTime;
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