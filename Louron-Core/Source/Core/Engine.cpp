#include "Engine.h"

// Louron Core Headers
#include "Time.h"
#include "Logging.h"
#include "../Debug/Profiler.h"
#include "../Jobs/Jobs.h"

#include "../Physics/Physics.h"
#include "../Project/Project.h"
#include "../Renderer/Renderer.h"
#include "../Scripting/Script Manager.h"

// C++ Standard Library Headers
#include <filesystem>

// External Vendor Library Headers

namespace Louron {

    Engine* Engine::s_Instance = nullptr;

    Engine::Engine(const EngineConfig& specification) : m_Specification(specification) {
        s_Instance = this;

        L_CORE_INFO("Initialising Louron Engine");

        if (!m_Specification.WorkingDirectory.empty())
            std::filesystem::current_path(m_Specification.WorkingDirectory);

        m_Window = Window::Create(WindowProps(m_Specification.Name));

        m_GuiLayer = new GuiLayer();
        PushOverlay(m_GuiLayer);

        // Init Time Manager
        Time::Init();

        // Init Physics Manager
        Physics::Init();

        // Init Audio System
        Audio::Init();

        // Renderer Init Debug VAOs
        Renderer::Init();

        // Init Job System
        JobSystem::Init();

        // Init Input Manager
        m_Input = std::make_unique<InputManager>();
        m_Input->Init((GLFWwindow*)m_Window->GetNativeWindow());
    }

    void Engine::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Engine::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Engine::Close()
    {
        m_Running = false;
    }

    void Engine::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    void Engine::Run()
    {
        while (m_Running) {
            L_PROFILE_SCOPE("Engine: Overall Loop");

            Profiler::Get().NewFrame();
			JobSystem::Get()->BeginFrameProfile();

            Time::Get().UpdateTime();

            {
                L_PROFILE_SCOPE("Engine: 1. Execute Main Thread");
                ExecuteMainThreadQueue();
            }

            {
                L_PROFILE_SCOPE("Engine: 2. Standard Update Loop");

                for (Layer* layer : m_LayerStack) {
                    layer->OnUpdate();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: 3. Fixed Update Loop");

                m_FixedUpdateTimer += Time::GetDeltaTime();
                          
                while (m_FixedUpdateTimer >= Time::GetUnscaledFixedDeltaTime()) {

                    for (Layer* layer : m_LayerStack) {
                        layer->OnFixedUpdate();
                    }

                    m_FixedUpdateTimer -= Time::GetUnscaledFixedDeltaTime();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: 4. Late Update Loop");

                for (Layer* layer : m_LayerStack) {
                    layer->OnLateUpdate();
                }
            }

            {
                L_PROFILE_SCOPE("Engine: 5. GUI Update Loop");
                m_GuiLayer->Begin();
                {

                    for (Layer* layer : m_LayerStack) {
                        layer->OnGuiRender();
                    }
                }
                m_GuiLayer->End();
            }

            {
                L_PROFILE_SCOPE("Engine: 6. Update Window (Finish GL Commands)");
                glFinish();
                m_Input->EndFrame();
                m_Window->OnUpdate();
            }

            {
                L_PROFILE_SCOPE("Engine: 7. Finish Jobs");
                JobSystem::Get()->FinishJobs();
            }

            JobSystem::Get()->EndFrameProfile();
        }

        Audio::Shutdown();
        Time::Shutdown();
        Renderer::Shutdown();
        JobSystem::Shutdown();
        Project::Shutdown();
        ScriptManager::Shutdown();
    }

    bool Engine::OnWindowClose()
    {
        m_Running = false;
        return true;
    }

    bool Engine::OnWindowResize()
    {
        return false;
    }

    void Engine::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }
}

