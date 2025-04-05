#pragma once

#include "../Defines/LTypes.h"
#include "../Engine Callbacks.h"


namespace Louron

{
	/**
	* @brief Static Input Methods
	*
	* This provides static functions for input handling.
	* This class is not meant to be instantiated, rather provides
	* methods that are called statically.
	*/
	class Input
	{
		Input() = delete; // Prevent instantiation of this class
		Input(const Input&) = delete; // Prevent copying
		Input& operator=(const Input&) = delete; // Prevent assignment
		Input(Input&&) = delete; // Prevent moving
		Input& operator=(Input&&) = delete; // Prevent move assignment
		~Input() = delete; // Prevent destruction

	public:

        /**
        * @brief Check if a key is pressed
        * 
        * This will check if the key is being continuously pressed.
        * 
        * Example Usage:
        * @code
        * if(Input::GetKey(KeyCode::Enter))
        *	Debug::Log("Enter key is pressed");
        * @endcode
        * 
        * @param key_code KeyCode The key code to check.
        * @return bool True if the key is pressed, false otherwise.
        */
        static bool GetKey(KeyCode key_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(KeyCode), Input_GetKey, key_code); }

        /**
        * @brief Check if a key was just pressed
        * 
        * This will check if the key was pressed down in the current frame.
        * 
        * Example Usage:
        * @code
        * if(Input::GetKeyDown(KeyCode::Enter))
        *	Debug::Log("Enter key was just pressed");
        * @endcode
        * 
        * @param key_code KeyCode The key code to check.
        * @return bool True if the key was just pressed, false otherwise.
        */
        static bool GetKeyDown(KeyCode key_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(KeyCode), Input_GetKeyDown, key_code); }

        /**
        * @brief Check if a key was just released
        * 
        * This will check if the key was released in the current frame.
        * 
        * Example Usage:
        * @code
        * if(Input::GetKeyUp(KeyCode::Enter))
        *	Debug::Log("Enter key was just released");
        * @endcode
        * 
        * @param key_code KeyCode The key code to check.
        * @return bool True if the key was just released, false otherwise.
        */
        static bool GetKeyUp(KeyCode key_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(KeyCode), Input_GetKeyUp, key_code); }

        /**
        * @brief Check if a mouse button is pressed
        * 
        * This will check if the mouse button is being continuously pressed.
        * 
        * Example Usage:
        * @code
        * if(Input::GetMouseButton(MouseButtonCode::Left))
        *	Debug::Log("Left mouse button is pressed");
        * @endcode
        * 
        * @param button_code MouseButtonCode The mouse button code to check.
        * @return bool True if the mouse button is pressed, false otherwise.
        */
        static bool GetMouseButton(MouseButtonCode button_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(MouseButtonCode), Input_GetMouseButton, button_code); }

        /**
        * @brief Check if a mouse button was just pressed
        * 
        * This will check if the mouse button was pressed down in the current frame.
        * 
        * Example Usage:
        * @code
        * if(Input::GetMouseButtonDown(MouseButtonCode::Left))
        *	Debug::Log("Left mouse button was just pressed");
        * @endcode
        * 
        * @param button_code MouseButtonCode The mouse button code to check.
        * @return bool True if the mouse button was just pressed, false otherwise.
        */
        static bool GetMouseButtonDown(MouseButtonCode button_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(MouseButtonCode), Input_GetMouseButtonDown, button_code); }

        /**
        * @brief Check if a mouse button was just released
        * 
        * This will check if the mouse button was released in the current frame.
        * 
        * Example Usage:
        * @code
        * if(Input::GetMouseButtonUp(MouseButtonCode::Left))
        *	Debug::Log("Left mouse button was just released");
        * @endcode
        * 
        * @param button_code MouseButtonCode The mouse button code to check.
        * @return bool True if the mouse button was just released, false otherwise.
        */
        static bool GetMouseButtonUp(MouseButtonCode button_code) { return ENGINE_SAFE_CALL_RET(bool, bool(*)(MouseButtonCode), Input_GetMouseUp, button_code); }

        /**
        * @brief Get the current mouse position
        * 
        * This will return the current position of the mouse cursor.
        * 
        * Example Usage:
        * @code
        * Vector2 mousePosition = Input::GetMousePosition();
        * Debug::Log("Mouse position: " + mousePosition.ToString());
        * @endcode
        * 
        * @return Vector2 The current mouse position.
        */
        static Vectors::Vector2 GetMousePosition() { return ENGINE_SAFE_CALL_RET(Vectors::Vector2, Vectors::Vector2(*)(), Input_GetMousePosition); }

	};

}