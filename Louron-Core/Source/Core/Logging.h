#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <mutex>
#include <string>
#include <vector>
#include <memory>

// External Vendor Library Headers
#pragma warning( push )
#pragma warning( disable : 4996 )
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/log_msg.h>

namespace Louron {


    // Structure to store a log entry's details.
    struct LogEntry {
        std::string text;
        spdlog::level::level_enum level;
        std::string logger_name;
    };

    class CustomLoggingSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        // Singleton accessor
        static CustomLoggingSink& GetInstance() {
            static CustomLoggingSink instance;
            return instance;
        }

        // Returns all log entries
        const std::vector<LogEntry>& GetLogs() const { return m_LogBuffer; }
        void Clear() { m_LogBuffer.clear(); }

    protected:
        // Called when a new log message is emitted
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);
            LogEntry entry;
            entry.text = fmt::to_string(formatted);
            entry.level = msg.level;
            entry.logger_name = msg.logger_name.data(); // logger name is stored in msg.logger_name
            m_LogBuffer.push_back(std::move(entry));
        }

        void flush_() override {}

    private:
        std::vector<LogEntry> m_LogBuffer;
    };

	class LoggingSystem {

	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetApplicationLogger() { return s_ApplicationLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetScriptingLogger() { return s_ScriptingLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ApplicationLogger;
		static std::shared_ptr<spdlog::logger> s_ScriptingLogger;

	};

}

#define L_CORE_INFO(...)   ::Louron::LoggingSystem::GetCoreLogger()->info(__VA_ARGS__)
#define L_CORE_TRACE(...)  ::Louron::LoggingSystem::GetCoreLogger()->trace(__VA_ARGS__)
#define L_CORE_WARN(...)   ::Louron::LoggingSystem::GetCoreLogger()->warn(__VA_ARGS__)
#define L_CORE_ERROR(...)  ::Louron::LoggingSystem::GetCoreLogger()->error(__VA_ARGS__)
#define L_CORE_FATAL(...)  ::Louron::LoggingSystem::GetCoreLogger()->critical(__VA_ARGS__)

#define L_APP_INFO(...)   ::Louron::LoggingSystem::GetApplicationLogger()->info(__VA_ARGS__)
#define L_APP_TRACE(...)  ::Louron::LoggingSystem::GetApplicationLogger()->trace(__VA_ARGS__)
#define L_APP_WARN(...)   ::Louron::LoggingSystem::GetApplicationLogger()->warn(__VA_ARGS__)
#define L_APP_ERROR(...)  ::Louron::LoggingSystem::GetApplicationLogger()->error(__VA_ARGS__)
#define L_APP_FATAL(...)  ::Louron::LoggingSystem::GetApplicationLogger()->critical(__VA_ARGS__)

#define L_SCRIPT_INFO(...)   ::Louron::LoggingSystem::GetScriptingLogger()->info(__VA_ARGS__)
#define L_SCRIPT_TRACE(...)  ::Louron::LoggingSystem::GetScriptingLogger()->trace(__VA_ARGS__)
#define L_SCRIPT_WARN(...)   ::Louron::LoggingSystem::GetScriptingLogger()->warn(__VA_ARGS__)
#define L_SCRIPT_ERROR(...)  ::Louron::LoggingSystem::GetScriptingLogger()->error(__VA_ARGS__)
#define L_SCRIPT_FATAL(...)  ::Louron::LoggingSystem::GetScriptingLogger()->critical(__VA_ARGS__)

#pragma warning( pop )