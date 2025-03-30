#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <deque>
#include <array>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <thread>

// External Vendor Library Headers


//TODO: job parking / yielding, e.g., if we need to wait for all child jobs to complete, we do not want to have the entire thread sleep which all child jobs are completed, we should yield the parent job, and then walk through local queue ?
namespace Louron
{
    enum class JobPriority : uint8_t
    {
        High = 0,
        Medium = 1,
        Low = 2
    };

    using JobFunction = std::function<void()>;

    class JobCounter
    {
    public:
        void Increment(uint32_t value = 1) { m_Count.fetch_add(value); }

        void Decrement()
        {
            if (m_Count.fetch_sub(1) == 1)
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Condition.notify_all();
            }
        }

        void Wait()
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Condition.wait(lock, [&]() { return m_Count.load() == 0; });
        }

    private:
        std::atomic<uint32_t> m_Count = 0;
        std::mutex m_Mutex;
        std::condition_variable m_Condition;
    };

    struct JobProfileEvent
    {
        double start_time;
        double end_time;
        JobPriority priority;
        std::string name;
    };

    struct FrameProfile
    {
        std::chrono::high_resolution_clock::time_point frame_start;
        double frame_duration; // milliseconds
        std::vector<std::vector<JobProfileEvent>> thread_events;
    };

    class JobSystem
    {

    public:

        static void Init();
        static void Shutdown();

        static JobSystem* Get();

        void SubmitJob(const std::string& name, const JobFunction& func, JobCounter* counter, JobPriority priority, bool persistent = false);
        void SubmitJobs(const std::vector<std::pair<std::string, JobFunction>>& jobs, JobCounter* counter, JobPriority priority, bool persistent = false);

        void FinishJobs();

        void BeginFrameProfile();
        void EndFrameProfile();
        FrameProfile GetProfileResults(int how_many_frames_ago = 1);

    private:

        JobSystem();
        ~JobSystem();

        struct Job
        {
            JobFunction Func;
            JobCounter* Counter = nullptr;
            JobPriority Priority = JobPriority::Medium;
            bool IsPersistent = false;
            std::string Name = "";

            Job() = default;
            Job(const std::string& n, const JobFunction& f, JobCounter* c, JobPriority p, bool pers)
                : Name(n), Func(f), Counter(c), Priority(p), IsPersistent(pers) {
            }
        };

        struct WorkerContext
        {
            std::deque<Job> LocalQueue;
        };

        static JobSystem* s_Instance;

        std::vector<std::thread> m_Threads;
        std::vector<std::unique_ptr<WorkerContext>> m_WorkerContexts;

        std::array<std::deque<Job>, 3> m_GlobalQueues;
        std::mutex m_GlobalQueueMutex;

        std::mutex m_QueueMutex;
        std::condition_variable m_JobAvailable;
        std::atomic<bool> m_Shutdown = false;

        int m_NumWorkerThreads = std::thread::hardware_concurrency();

        void WorkerThread(size_t index);

        bool GetJob(Job& out_job, WorkerContext& context);
        bool StealJob(size_t thief_index, Job& out_job);
        bool HasJobs();

        std::mutex m_ProfileMutex;
        std::deque<FrameProfile> m_FrameProfiles;
        FrameProfile m_CurrentFrameProfile;
        std::atomic<bool> m_ProfilingActive{ false };
    };
}