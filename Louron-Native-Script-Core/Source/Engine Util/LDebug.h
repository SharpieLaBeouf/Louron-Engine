#pragma once

#include "../Engine Callbacks.h"

namespace Louron
{
    /**
    * @brief Static Debug Methods
    * 
	* This provides static functions for debugging purposes. 
    * This class is not meant to be instantiated, rather provides
    * methods that are called statically.
    */
    class Debug
    {
		Debug() = delete; // Prevent instantiation of this class
		Debug(const Debug&) = delete; // Prevent copying
		Debug& operator=(const Debug&) = delete; // Prevent assignment
		Debug(Debug&&) = delete; // Prevent moving
		Debug& operator=(Debug&&) = delete; // Prevent move assignment
		~Debug() = delete; // Prevent destruction

    public:


        /**
        * @brief Native Logging Function
        * 
        * This will create a log in the console at runtime.
        * 
        * @code
        * void OnCreate()
        * {
		*     Debug::Log("Default Log");
		*     Debug::Log("Warning Log", 1);
		*     Debug::Log("Error Log", 2);
		*     Debug::Log("Critical Log", 3);
        * } 
        * @endcode
        * 
		* @param message The message to log.
		* @param log_type The type of log. 0 = Info, 1 = Warning, 2 = Error, 3 = Critical. This is optional and defaults to 0.
        */
        static void Log(const std::string& message, uint8_t log_type = 0)
        {
            ENGINE_SAFE_CALL_VOID(void(*)(const char*, uint8_t), Debug_NativeLogMessage, message.c_str(), log_type);
        }

    };
}