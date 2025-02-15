#pragma once

// Louron Core Headers
#include "Logging.h"

// C++ Standard Library Headers
#include <chrono>

// External Vendor Library Headers

namespace Louron {

	class Time {

	public:

		static void Init();
		static void Shutdown();

		static Time& Get();

		void UpdateTime();

		static float GetUnscaledDeltaTime();
		static float GetDeltaTime();

		static float GetUnscaledFixedDeltaTime();
		static float GetFixedDeltaTime();

		static void SetFixedDeltaTime(const float& fixedDeltaTime);

		static int GetUnscaledFixedUpdatesHz();
		static int GetFixedUpdatesHz();

		static float GetTimeScale();
		static void SetTimeScale(const float& timeScale);
		
		/// <summary>Estimate of average framerate.</summary>
		static int GetFrameRate();

		double GetCurrTime() const;

	private:

		Time();

		// Delete copy assignment and move assignment constructors
		Time(const Time&) = delete;
		Time(Time&&) = delete;

		// Delete copy assignment and move assignment operators
		Time& operator=(const Time&) = delete;
		Time& operator=(Time&&) = delete;

		double m_CurrentTime	{ 0.0 };
		double m_DeltaTime		{ 0.0 };
		float m_FixedDeltaTime	{ 1.0f / 60.0f };
		float m_TimeScale		{ 1.0f };

		int m_FrameRateEstimate	{ 0 };
		float m_FrameRateTimer	{ 1.0f };

		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastTimeClock;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_CurrentTimeClock;

	};

}