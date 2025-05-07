#pragma once

#include "../Engine Callbacks.h"

namespace Louron
{
	/**
	* @brief Static Time Methods
	*
	* This provides static functions for time handling.
	* This class is not meant to be instantiated, rather provides
	* methods that are called statically.
	*/
	class Time
	{
		Time() = delete; // Prevent instantiation of this class
		Time(const Time&) = delete; // Prevent copying
		Time& operator=(const Time&) = delete; // Prevent assignment
		Time(Time&&) = delete; // Prevent moving
		Time& operator=(Time&&) = delete; // Prevent move assignment
		~Time() = delete; // Prevent destruction

	public:

		/**
		* @brief Get the scaled delta time of the last frame.
		*
		* This returns the time in seconds between the current and previous frame, scaled by the current time scale.
		* - If time scale is `0.0f`, this returns `0.0f`.
		* - If time scale is `1.0f`, this returns the actual frame delta time.
		* - If time scale is `2.0f`, this returns double the delta time (i.e., simulating fast-forwarding).
		*
		* Use this for gameplay elements that should respect time scaling (e.g., player movement, cooldowns).
		*
		* Example Usage:
		* @code
		* float timer = 5.0f;
		*
		* void OnUpdate()
		* {
		*     if (timer > 0.0f)
		*     {
		*         timer -= Time::GetDeltaTime();
		*     }
		*     else
		*     {
		*         timer = 5.0f;
		*         Debug::Log("Timer has finished!");
		*     }
		* }
		* @endcode
		*
		* @return float Scaled delta time from the last frame.
		*/
		static float GetDeltaTime() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetDeltaTime); }

		/**
		* @brief Get the fixed delta time between physics updates.
		*
		* This returns the amount of time (in seconds) that passes between each fixed update (physics tick).
		* The fixed timestep is independent of frame rate and is used for deterministic updates such as physics.
		*
		* Example Usage:
		* @code
		* void FixedUpdate()
		* {
		*     velocity += acceleration * Time::GetFixedDeltaTime();
		*     position += velocity * Time::GetFixedDeltaTime();
		* }
		* @endcode
		*
		* @return float Fixed delta time used in physics or fixed update loops.
		*/
		static float GetFixedDeltaTime() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetFixedDeltaTime); }

		/**
		* @brief Get the unscaled delta time of the last frame.
		*
		* This returns the actual frame delta time in seconds, ignoring any time scaling.
		* Useful when you need time information that is unaffected by slow-motion, pausing, or time manipulation.
		*
		* Example Usage:
		* @code
		* float fade_alpha = 0.0f;
		* bool is_entity_fading_in = true;
		*
		* void OnUpdate()
		* {
		*     if (is_entity_fading_in)
		*     {
		*         fade_alpha += Time::GetUnscaledDeltaTime();
		*         if (fade_alpha > 1.0f)
		*         {
		*             fade_alpha = 1.0f;
		*             is_entity_fading_in = false;
		*         }
		*     }
		* }
		* @endcode
		*
		* @return float Unscaled delta time from last frame.
		*/
		static float GetUnscaledDeltaTime() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetUnscaledDeltaTime); }

		/**
		* @brief Get the unscaled fixed delta time.
		*
		* This returns the fixed timestep (in seconds) between physics updates, ignoring time scaling.
		* Useful when time-sensitive systems must continue updating even when time scale is modified or paused.
		*
		* Example Usage:
		* @code
		* bool not_affected_by_time_scale = true;
		* 
		* void OnFixedUpdate()
		* {
		*     if (not_affected_by_time_scale)
		*     {
		*	      velocity += acceleration * Time::GetUnscaledFixedDeltaTime();
		*     }
		*     else
		*     {
		*         velocity += acceleration * Time::GetFixedDeltaTime();
		*     }
		* }
		* @endcode
		*
		* @return float Unscaled fixed delta time between physics updates.
		*/
		static float GetUnscaledFixedDeltaTime() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetUnscaledFixedDeltaTime); }

		/**
		* @brief Get the current time scale.
		*
		* The time scale controls how fast or slow time progresses in the simulation.
		* - 1.0 means real-time
		* - 0.0 means paused
		* - >1.0 speeds up time
		* - <1.0 slows time down
		*
		* Example Usage:
		* @code
		* if (Input::GetKeyDown(KeyCode::T))
		* {
		*     float scale = Time::GetTimeScale();
		*     Debug::Log("Current Time Scale: " + std::to_string(scale));
		* }
		* @endcode
		*
		* @return float Current time scale multiplier.
		*/
		static float GetTimeScale() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetTimeScale); }

		/**
		* @brief Set the current time scale.
		*
		* Adjusts how quickly time passes for all time-dependent systems.
		* This can be used to implement slow-motion, pausing, or fast-forwarding effects.
		*
		* Example Usage:
		* @code
		* bool paused = false
		* 
		* void TogglePause()
		* {
		*     if (Input::GetKeyDown(KeyCode::P))
		*     {
		*         paused = !paused;
		* 
		*         if (paused)
		*             Time::SetTimeScale(0.0f); // Pause
		*         else
		*             Time::SetTimeScale(1.0f); // Resume
		*     }
		* }
		* @endcode
		*
		* @param time_scale New time scale multiplier to set.
		*/
		static void SetTimeScale(float time_scale) { ENGINE_SAFE_CALL_VOID(void(*)(float), Time_SetTimeScale, time_scale); }

		/**
		* @brief Get the current frame rate (frames per second).
		*
		* This returns an integer value representing how many frames are being rendered per second.
		* It can be useful for debugging performance or displaying a simple FPS counter.
		*
		* Example Usage:
		* @code
		* void OnUpdate()
		* {
		*     Debug::Log("FPS: " + std::to_string(Time::GetFrameRate()));
		* }
		* @endcode
		*
		* @return int Current frame rate in frames per second.
		*/
		static int GetFrameRate() { return ENGINE_SAFE_CALL_RET(int, int(*)(), Time_GetFrameRate); }

		/**
		* @brief Get the current time since epoch.
		*
		* This returns the current runtime in seconds since epoch.
		* Useful for timers, animations, analytics, or time-based effects.
		*
		* Example Usage:
		* @code
		* float start_time = 0.0f;
		* float end_time = 0.0f;
		* 
		* void OnUpdate()
		* {
		*	  if (start_time == 0.0f) start_time = Time::GetCurrTime();
		* 
		*     end_time = Time::GetCurrTime();
		* 
		*     float rudimentary_delta_time = end_time - start_time;
		* 
		*     // Some other code...
		* 
		*	  start_time = Time::GetCurrTime();
		* }
		* @endcode
		*
		* @return float Total time in seconds since application started.
		*/
		static float GetCurrTime() { return ENGINE_SAFE_CALL_RET(float, float(*)(), Time_GetCurrTime); }

		/**
		* @brief Set the fixed delta time for the fixed update loops.
		* 
		* This is at default set to 1.0f / 60.0f (60Hz).
		* 
		* This can be adjusted to change the frequency of fixed updates.
		* 
		* @param fixed_delta_time New fixed delta time to set. You must ensure that you enter a valid value, typically as '1.0f / X', where X is the desired frequency in Hz.
		*/
		static void SetFixedDeltaTime(float fixed_delta_time = 1.0f / 60.0f) { ENGINE_SAFE_CALL_VOID(float(*)(float), Time_SetFixedDeltaTime, fixed_delta_time); }

	};
}