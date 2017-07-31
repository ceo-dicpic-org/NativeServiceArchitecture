#pragma once

#include <functional>
#include <thread>
#include <future>

#include "BlockingQueue.hpp"

namespace NSA
{

/**
 * @brief Abstract base class of a service
 * @details A service is defined by a promise and a future. The 
 * service promises to complete a job and in the future the
 * client can call upon the future to get the completed job.
 * 
 * Each service has a job list. This list is a basic queue
 * that will be worked in FIFO order.
 * 
 */

class Service
{
public:
	template <class T> using Promise = std::shared_ptr<std::promise<T>>;
	template <class T> using Future  = std::shared_ptr<std::future<T>>;

	/**
	 * @brief Default constructor creates a deactivated service.
	 * @details Services can be started and stipped via the
	 * detach and join functions.
	 * @param name Each service should have name.
	 */
	Service(const std::string name, const std::size_t jobLimit = 0) : running(false), name(name),
		jobCount(0), jobList(jobLimit)
	{}

	/**
	 * @brief Start a service and detach the process from the
	 * current thread.
	 * @details Sets the running member variable to true and opens
	 * the queue for more job input.
	 */
	void detach(const std::size_t workers = 1)
	{
		running = true;
		for (std::size_t i = 0; i < workers; i++)
			workThreads.push_back(std::thread(&Service::work, this));	
	}

	/**
	 * @brief Close the service. Pending jobs will be resolved.
	 * @details Adds a final job to the job list queue and closes
	 * the queue for further input.
	 */
	void join()
	{
		running = false;

		std::mutex joinLock;
		std::unique_lock<std::mutex> waitLock(joinLock);

		joinCondition.wait(waitLock, [this]{return jobList.empty();});

		for (std::thread &worker : workThreads)
		{
			jobList.push([]{return;});
			worker.join();		
		}
	}

	std::size_t totalJobs() const
	{
		return jobCount;
	}

	std::size_t currentJobs() const
	{
		return jobList.size();
	}

	void jobTimeOut(std::chrono::milliseconds timeOut)
	{
		timeOut = timeOut;
	}

protected:
	/**
	 * @brief A helper function to create a promise.
	 * @details To unify the functionality of a service, this function
	 * defines how to create a promise future pair.
	 * 
	 * The job will be added to the queue, where as the future
	 * is returned to the caller.
	 * 
	 * @param  Any given function which acts as a job.
	 * @tparam T The return value type of the job.
	 * @return Returns the future for the job.
	 */
	template <class T>
	Service::Future<T> makePromise(std::function<void(Service::Promise<T>)> job)
	{
		Service::Promise<T> promise(new std::promise<T>);
		Service::Future<T> future(new std::future<T>);
		*future = promise->get_future();

		if (running)
		{
			if (!jobList.push(std::bind(job, promise), timeOut))
				printf("%s: Job timed out. Timeout is at %d\n", name.c_str(), timeOut);
		}

		return future;	
	}

	/**
	 * @brief A helper macro to create a promise.
	 * @details Using the makePromise function is a bit tricky. You have to
	 * use std::bind to create a std::function object which in turn is
	 * accepted by the makePromise function.
	 * 
	 * @param functionName The function which acts as the job.
	 * @param returnValueType The type of the return value.	
	 * @param ... every optional parameter given into the function.
	 * @return Returns a valid future for future use.
	 */
#define NSA_MAKE_PROMISE(functionName, returnValueType, ...) return makePromise<returnValueType>(std::bind(&functionName, this, std::placeholders::_1, ##__VA_ARGS__))

private:
	/**
	 * @brief The main thread of the serice.
	 * @details The thread waits for a job to be added into the
	 * job list. The thread runs until a join is called.
	 * 
	 * @param e [description]
	 */
	void work()
	{
		std::function<void()> currentJob;

		while (running || !jobList.empty())
		{	
			jobList.pop(&currentJob);
			currentJob();
			jobCount++;
		}

		joinCondition.notify_all();
	}

protected:
	std::string name; ///< The name of the job.

private:
	BlockingQueue<std::function<void()>> jobList; ///< The job list.
	std::atomic<std::size_t> jobCount;            ///< Total job count.
	std::atomic<bool> running;                    ///< Status of the service.
	std::vector<std::thread> workThreads;         ///< Collection of workers.
	std::condition_variable joinCondition;        ///< Condition for clean up.
	std::chrono::milliseconds timeOut;            ///< TimeOut to drop job.
};

} // namespace NSA