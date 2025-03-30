#include "Jobs.h"

#include "../Debug/Assert.h"
#include "../Core/Platform.h"

#ifdef L_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace Louron
{
    JobSystem* JobSystem::s_Instance = nullptr;

    static void SetThreadCore(size_t index)
    {
#ifdef L_PLATFORM_WINDOWS
        DWORD_PTR mask = 1ull << index;
        SetThreadAffinityMask(GetCurrentThread(), mask);
#elif defined(L_PLATFORM_LINUX)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(index, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#endif
    }

    void JobSystem::Init()
    {
        L_CORE_INFO("Spawning Job System");
        if (!s_Instance)
            s_Instance = new JobSystem();
    }

    void JobSystem::Shutdown()
    {
        if (s_Instance)
            delete s_Instance;
    }

    JobSystem* JobSystem::Get()
    {
        if (!s_Instance)
            JobSystem::Init();

        L_CORE_ASSERT(s_Instance, "Job System Not Initalised!");
        return s_Instance;
    }

    JobSystem::JobSystem()
    {
        m_CurrentFrameProfile.thread_events.resize(m_NumWorkerThreads);
        for (auto& events : m_CurrentFrameProfile.thread_events)
            events.reserve(128);

        m_WorkerContexts.resize(m_NumWorkerThreads);

        SetThreadCore(0);

		// Start at 1 because the main thread is locked to core 0
        for (size_t i = 1; i < m_NumWorkerThreads; ++i)
        {
            m_WorkerContexts[i] = std::make_unique<WorkerContext>();
            m_Threads.emplace_back([this, i]() { WorkerThread(i); });
        }
    }

    JobSystem::~JobSystem()
    {
        m_Shutdown = true;
        m_JobAvailable.notify_all();
        for (auto& t : m_Threads)
            if (t.joinable())
                t.join();
    }

    void JobSystem::SubmitJob(const std::string& name, const JobFunction& func, JobCounter* counter, JobPriority priority, bool persistent)
    {
        SubmitJobs({ {name, func} }, counter, priority, persistent);
    }

    void JobSystem::SubmitJobs(const std::vector<std::pair<std::string, JobFunction>>& jobs, JobCounter* counter, JobPriority priority, bool persistent)
    {
        {
            std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
            for (const auto& pair : jobs)
            {
                const auto& job_name = pair.first;
                const auto& func = pair.second;
                if (counter) counter->Increment();
                m_GlobalQueues[static_cast<int>(priority)].emplace_back(job_name, func, counter, priority, persistent);
            }
        }
        m_JobAvailable.notify_all();
    }

    void JobSystem::FinishJobs()
    {
        while (true)
        {
            bool any_frame_jobs_remaining = false;

            {
                std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
                for (const auto& queue : m_GlobalQueues)
                {
                    for (const auto& job : queue)
                    {
                        if (!job.IsPersistent)
                        {
                            any_frame_jobs_remaining = true;
                            break;
                        }
                    }
                    if (any_frame_jobs_remaining) break;
                }
            }

            if (!any_frame_jobs_remaining)
            {
                for (const auto& context : m_WorkerContexts)
                {
                    if (!context) continue;

                    for (const auto& job : context->LocalQueue)
                    {
                        if (!job.IsPersistent)
                        {
                            any_frame_jobs_remaining = true;
                            break;
                        }
                    }
                    if (any_frame_jobs_remaining) break;
                }
            }

            if (!any_frame_jobs_remaining)
                break;

            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    void JobSystem::WorkerThread(size_t index)
    {
        SetThreadCore(index);

        auto& context = *m_WorkerContexts[index];

        while (!m_Shutdown)
        {
            Job job;
            bool success = false;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_JobAvailable.wait(lock, [this]() {
                    return m_Shutdown || HasJobs();
                });

                if (m_Shutdown)
                    break;

                success = GetJob(job, context);
            }

            if (!success)
                success = StealJob(index, job);

            if (success)
            {
                auto job_start_time = std::chrono::high_resolution_clock::now();
                job.Func();
                auto job_end_time = std::chrono::high_resolution_clock::now();

                if (m_ProfilingActive)
                {
                    double start_ms = std::chrono::duration<double, std::milli>(job_start_time - m_CurrentFrameProfile.frame_start).count();
                    double end_ms = std::chrono::duration<double, std::milli>(job_end_time - m_CurrentFrameProfile.frame_start).count();

                    JobProfileEvent event{ start_ms, end_ms, job.Priority, job.Name };
                    m_CurrentFrameProfile.thread_events[index].push_back(event);
                }

                if (job.Counter)
                    job.Counter->Decrement();
            }
        }
    }

    bool JobSystem::GetJob(Job& out_job, WorkerContext& context)
    {
        if (!context.LocalQueue.empty())
        {
            out_job = context.LocalQueue.back();
            context.LocalQueue.pop_back();
            return true;
        }

        std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
        for (int p = 0; p < 3; ++p)
        {
            auto& queue = m_GlobalQueues[p];
            if (!queue.empty())
            {
                out_job = queue.front();
                queue.pop_front();
                return true;
            }
        }
        return false;
    }

    bool JobSystem::StealJob(size_t thief_index, Job& out_job)
    {
        for (size_t i = 0; i < m_WorkerContexts.size(); ++i)
        {
            if (i == thief_index || !m_WorkerContexts[i])
                continue;

            auto& victim = *m_WorkerContexts[i];
            if (!victim.LocalQueue.empty())
            {
                out_job = victim.LocalQueue.front();
                victim.LocalQueue.pop_front();
                return true;
            }
        }
        return false;
    }

    bool JobSystem::HasJobs()
    {
        std::lock_guard<std::mutex> lock(m_GlobalQueueMutex);
        for (const auto& q : m_GlobalQueues)
        {
            if (!q.empty())
                return true;
        }
        return false;
    }

    void JobSystem::BeginFrameProfile()
    {
        std::lock_guard<std::mutex> lock(m_ProfileMutex);
        m_ProfilingActive = true;
        m_CurrentFrameProfile.frame_start = std::chrono::high_resolution_clock::now();

        m_CurrentFrameProfile.thread_events.resize(m_NumWorkerThreads);
        for (auto& events : m_CurrentFrameProfile.thread_events)
        {
            events.clear();
            events.reserve(128);
        }
    }

    void JobSystem::EndFrameProfile()
    {
        auto frame_end = std::chrono::high_resolution_clock::now();
        {
            std::lock_guard<std::mutex> lock(m_ProfileMutex);
            m_ProfilingActive = false;
            m_CurrentFrameProfile.frame_duration = std::chrono::duration<double, std::milli>(frame_end - m_CurrentFrameProfile.frame_start).count();
            m_FrameProfiles.push_back(m_CurrentFrameProfile);

            if (m_FrameProfiles.size() > 100)
                m_FrameProfiles.pop_front();
        }
    }

    FrameProfile JobSystem::GetProfileResults(int how_many_frames_ago)
    {
        std::lock_guard<std::mutex> lock(m_ProfileMutex);
        if (m_FrameProfiles.empty())
            return FrameProfile();
        int index = static_cast<int>(m_FrameProfiles.size()) - how_many_frames_ago;
        if (index < 0)
            index = 0;
        return m_FrameProfiles[index];
    }
}
