#include "Logging.h"

// Louron Core Headers

// C++ Standard Library Headers

// External Vendor Library Headers
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Louron {

	std::shared_ptr<spdlog::logger> LoggingSystem::s_CoreLogger;
	std::shared_ptr<spdlog::logger> LoggingSystem::s_ApplicationLogger;
	std::shared_ptr<spdlog::logger> LoggingSystem::s_ScriptingLogger;

	void LoggingSystem::Init() 
    {
        spdlog::set_pattern("[%T][%^%l%$] %n: %v");

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto custom_sink = std::shared_ptr<CustomLoggingSink>(&CustomLoggingSink::GetInstance(), [](CustomLoggingSink*) {});

        s_CoreLogger = std::make_shared<spdlog::logger>("L_CORE", spdlog::sinks_init_list{ console_sink, custom_sink });
        s_CoreLogger->set_level(spdlog::level::trace);

        s_ApplicationLogger = std::make_shared<spdlog::logger>("L_APP", spdlog::sinks_init_list{ console_sink, custom_sink });
        s_ApplicationLogger->set_level(spdlog::level::trace);

        s_ScriptingLogger = std::make_shared<spdlog::logger>("L_SCRIPTS", spdlog::sinks_init_list{ console_sink, custom_sink });
        s_ScriptingLogger->set_level(spdlog::level::trace);
	}

}