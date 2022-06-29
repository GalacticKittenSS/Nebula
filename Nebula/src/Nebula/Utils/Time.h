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
		static void Init() { m_Start = Now(); }
		static void Update() {
			m_Timestep = Elapsed() - m_LastFrameTime;
			m_LastFrameTime = Elapsed();
		}

		static float Now();
		static inline float Elapsed() { return Now() - m_Start; }
		static inline Timestep DeltaTime() { return m_Timestep; }
	private:
		static float m_Start;
		static Timestep m_Timestep;
		static float m_LastFrameTime;
	};

	class Timer {
	public:
		Timer() {
			Reset();
		}

		void Reset() {
			m_Start = Time::Now();
		}

		float Elapsed() {
			return (Time::Now() - m_Start) / 1000.0f;
		}
	private:
		float m_Start;
	};
}