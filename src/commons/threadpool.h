#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sagittar::commons {

    class ThreadPool {
       public:
        struct task_cancelled: std::runtime_error {
            task_cancelled() :
                std::runtime_error("ThreadPool: task cancelled before start") {}
        };

        explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency()) :
            accepting_(true),
            immediate_(false) {
            if (thread_count == 0)
                thread_count = 1;
            workers_.reserve(thread_count);

            for (std::size_t i = 0; i < thread_count; ++i)
            {
                workers_.emplace_back([this](std::stop_token st) { worker_loop(st); });
            }
        }

        ThreadPool(const ThreadPool&)            = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&)                 = delete;
        ThreadPool& operator=(ThreadPool&&)      = delete;

        ~ThreadPool() {
            shutdown();
            // std::jthread auto-joins
        }

        // Graceful: stop accepting new tasks; workers drain the queue.
        void shutdown() noexcept {
            {
                std::lock_guard<std::mutex> lk(mtx_);
                accepting_ = false;
            }
            request_stop_all();
            cv_.notify_all();
        }

        // Immediate: cancel queued tasks (not yet started) and request stop.
        void shutdown_now() noexcept {
            std::queue<Job> to_cancel;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                accepting_ = false;
                immediate_ = true;
                to_cancel.swap(jobs_);
            }

            while (!to_cancel.empty())
            {
                to_cancel.front().cancel();
                to_cancel.pop();
            }

            request_stop_all();
            cv_.notify_all();
        }

        [[nodiscard]] bool accepting() const {
            std::lock_guard<std::mutex> lk(mtx_);
            return accepting_;
        }

        [[nodiscard]] std::size_t queued_jobs() const {
            std::lock_guard<std::mutex> lk(mtx_);
            return jobs_.size();
        }

        // --- Return type deduction for submit() ---
        // We must be LAZY to avoid instantiating invoke_result_t on the wrong branch
        // (libc++ will otherwise fail substitution).
        template<bool WithStopToken, class F, class... Args>
        struct submit_result_impl;

        template<class F, class... Args>
        struct submit_result_impl<true, F, Args...> {
            using type =
              std::invoke_result_t<std::decay_t<F>, std::stop_token, std::decay_t<Args>...>;
        };

        template<class F, class... Args>
        struct submit_result_impl<false, F, Args...> {
            using type = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
        };

        template<class F, class... Args>
        using submit_result_t = typename submit_result_impl<
          std::is_invocable_v<std::decay_t<F>, std::stop_token, std::decay_t<Args>...>,
          F,
          Args...>::type;

        // submit() supports either:
        //   R f(Args...)
        // or
        //   R f(std::stop_token, Args...)
        template<class F, class... Args>
        auto submit(F&& f, Args&&... args) -> std::future<submit_result_t<F, Args...>> {
            using R = submit_result_t<F, Args...>;

            auto state = std::make_shared<State<R>>();

            // Decay/capture callable and args once.
            auto fn  = std::decay_t<F>(std::forward<F>(f));
            auto tup = std::make_tuple(std::decay_t<Args>(std::forward<Args>(args))...);

            // Run: invoke with stop_token if invocable, otherwise without.
            auto run = [state, fn = std::move(fn),
                        tup = std::move(tup)](std::stop_token st) mutable {
                state->run_once([&]() -> R {
                    return std::apply(
                      [&](auto&&... unpacked) -> R {
                          if constexpr (std::is_invocable_v<decltype(fn), std::stop_token,
                                                            decltype(unpacked)...>)
                          {
                              return std::invoke(fn, st,
                                                 std::forward<decltype(unpacked)>(unpacked)...);
                          }
                          else
                          {
                              return std::invoke(fn, std::forward<decltype(unpacked)>(unpacked)...);
                          }
                      },
                      tup);
                });
            };

            auto cancel = [state] {
                state->cancel_once(std::make_exception_ptr(task_cancelled{}));
            };

            std::future<R> fut = state->promise.get_future();

            {
                std::lock_guard<std::mutex> lk(mtx_);
                if (!accepting_)
                {
                    cancel();
                    return fut;
                }
                jobs_.push(Job{std::move(run), std::move(cancel)});
            }

            cv_.notify_one();
            return fut;
        }

       private:
        struct Job {
            std::function<void(std::stop_token)> run;
            std::function<void()>                cancel;
        };

        template<class R>
        struct State {
            std::promise<R>   promise;
            std::atomic<bool> completed{false};

            template<class Thunk>
            void run_once(Thunk&& thunk) {
                if (completed.exchange(true, std::memory_order_acq_rel))
                    return;

                try
                {
                    if constexpr (std::is_void_v<R>)
                    {
                        std::forward<Thunk>(thunk)();
                        promise.set_value();
                    }
                    else
                    {
                        promise.set_value(std::forward<Thunk>(thunk)());
                    }
                } catch (...)
                { promise.set_exception(std::current_exception()); }
            }

            void cancel_once(std::exception_ptr eptr) {
                if (completed.exchange(true, std::memory_order_acq_rel))
                    return;
                promise.set_exception(std::move(eptr));
            }
        };

        void request_stop_all() noexcept {
            for (auto& w : workers_)
                w.request_stop();
        }

        void worker_loop(std::stop_token st) {
            for (;;)
            {
                Job job;

                {
                    std::unique_lock<std::mutex> lk(mtx_);
                    cv_.wait(lk, st, [&] {
                        return immediate_ || !jobs_.empty() || !accepting_ || st.stop_requested();
                    });

                    // Immediate mode: cancel remaining queue and exit.
                    if (immediate_)
                    {
                        auto q = std::queue<Job>{};
                        q.swap(jobs_);
                        lk.unlock();

                        while (!q.empty())
                        {
                            q.front().cancel();
                            q.pop();
                        }
                        return;
                    }

                    // Graceful exit: no longer accepting and nothing left.
                    if (!accepting_ && jobs_.empty())
                        return;

                    if (!jobs_.empty())
                    {
                        job = std::move(jobs_.front());
                        jobs_.pop();
                    }
                    else
                    {
                        continue;  // spurious wake
                    }
                }

                // Run outside lock.
                job.run(st);
            }
        }

       private:
        mutable std::mutex          mtx_;
        std::condition_variable_any cv_;
        std::queue<Job>             jobs_;
        bool                        accepting_;
        bool                        immediate_;
        std::vector<std::jthread>   workers_;
    };

}
