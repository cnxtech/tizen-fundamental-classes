/*
 * Async.cpp
 *
 *  Created on: Mar 1, 2016
 *      Contributor:
 *        Gilang M. Hamidy (g.hamidy@samsung.com)
 *        Kevin Winata (k.winata@samsung.com)
 */
#include "TFC/Core.h"
#include "TFC/Async.h"

#include <Elementary.h>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <map>


#define ASYNC_LOG_TAG "TFCFW-Async"


namespace TFC {

class AsyncTaskObj;

std::map<pthread_t, AsyncTaskObj*> ecoreThreadMap;

enum class TaskState
{
	Undefined = 0, Created = 1, Running = 2, Completed = 3, Cancelling = 4, Cancelled = 5
};

class AsyncTaskObj
{
public:
	Ecore_Thread* handle;
	void* retVal;
	TaskState state;
	bool dwait;
	std::function<void(void*, void*)> dwaitCaller;
	std::mutex taskMutex;
	pthread_t threadId;

	AsyncTaskObj()
	{
		handle = nullptr;
		retVal = nullptr;
		state = TaskState::Created;
		dwait = false;
		threadId = 0;
		dlog_print(DLOG_DEBUG, ASYNC_LOG_TAG, "Task created %d", this);
	}

	void Start()
	{
		threadId = pthread_self();
		ecoreThreadMap.insert(std::make_pair(threadId, this));
	}

	void End()
	{
		threadId = 0;
		ecoreThreadMap.erase(threadId);
	}

	~AsyncTaskObj()
	{
		dlog_print(DLOG_DEBUG, ASYNC_LOG_TAG, "Task destroyed %d", this);
	}

};

LIBAPI void* AwaitImpl(AsyncTaskObj* task)
{
	{
		std::lock_guard < std::mutex > lock(task->taskMutex);

		// If it was dwaited, clear dwait
		if (task->dwait)
			return nullptr;
	}

	while (task->state == TaskState::Running || task->state == TaskState::Cancelling)
		usleep(100);

	auto retVal = task->retVal;

	delete task;
	return retVal;
}

template<class R>
struct TaskContext
{
	std::function<R(void)> func;
	AsyncTaskObj* task;
	//Event<R> event;

	TaskContext() : task(nullptr)
	{
		dlog_print(DLOG_DEBUG, ASYNC_LOG_TAG, "Context created");
	}

	~TaskContext()
	{
		dlog_print(DLOG_DEBUG, ASYNC_LOG_TAG, "Context destroyed");
	}
};

template<class R>
void AsyncTaskWorker(void* data, Ecore_Thread *thread);

template<class R>
void AsyncTaskEnd(void* data, Ecore_Thread* thread);

template<>
void AsyncTaskWorker<void*>(void* data, Ecore_Thread *thread)
{
	TaskContext<void*>* context = reinterpret_cast<TaskContext<void*>*>(data);

	// Sync with other thread to tell that this thread is start processing
	{
		std::lock_guard < std::mutex > lock(context->task->taskMutex);
		context->task->state = TaskState::Running;
		context->task->Start();
	}

	auto retVal = context->func();

	{
		std::lock_guard < std::mutex > lock(context->task->taskMutex);
		context->task->retVal = retVal;
		if (context->task->state == TaskState::Cancelling)
		{
			context->task->state = TaskState::Cancelled;
			delete context;
		}
		else
			context->task->state = TaskState::Completed;
	}
	context->task->End();
}

template<>
void AsyncTaskWorker<void>(void* data, Ecore_Thread *thread)
{
	TaskContext<void*>* context = reinterpret_cast<TaskContext<void*>*>(data);

	// Sync with other thread to tell that this thread is start processing
	{
		std::lock_guard < std::mutex > lock(context->task->taskMutex);
		context->task->state = TaskState::Running;
		context->task->Start();
	}

	context->func();

	// Sync with other thread before notifying that this task is completed
	{
		std::lock_guard < std::mutex > lock(context->task->taskMutex);
		if (context->task->state == TaskState::Cancelling)
		{
			context->task->state = TaskState::Cancelled;
			delete context;
		}
		else
			context->task->state = TaskState::Completed;
	}
	context->task->End();
}

template<class T>
void AsyncTaskEnd(void* data, Ecore_Thread* thread)
{
	TaskContext<T>* context = reinterpret_cast<TaskContext<T>*>(data);

	if (context->task->dwait)
	{
		context->task->dwaitCaller(context->task, context->task->retVal);
		delete context->task;
	}

	delete context;
}

template<class T>
void AsyncTaskCancel(void* data, Ecore_Thread* thread)
{
	TaskContext<T>* context = reinterpret_cast<TaskContext<T>*>(data);

	if (context->task->dwait)
	{
		delete context->task;
	}

	delete context;
}

/*
 template<>
 LIBAPI void await(AsyncTask<void>* task)
 {
 AwaitImpl(reinterpret_cast<AsyncTaskObj*>(task));
 }
 */

template<class T>
AsyncTaskObj* CreateAsyncTaskGeneric(std::function<T(void)> func, std::function<void(void*, void*)> dispatcher, bool priority)
{
	auto context = new TaskContext<T>();
	context->func = func;
	auto task = new AsyncTaskObj();
	context->task = task;

	if (dispatcher)
	{
		task->dwait = true;
		task->dwaitCaller = dispatcher;
	}

	task->handle = ecore_thread_feedback_run(AsyncTaskWorker<T>, nullptr, AsyncTaskEnd<T>, nullptr, context, (priority ? EINA_TRUE : EINA_FALSE));
	return task;
}

template<>
LIBAPI AsyncTaskObj* CreateAsyncTask(std::function<void*(void)> func, std::function<void(void*, void*)> dispatcher, bool priority)
{
	return CreateAsyncTaskGeneric(func, dispatcher, priority);
}

template<>
LIBAPI AsyncTaskObj* CreateAsyncTask(std::function<void(void)> func, std::function<void(void*, void*)> dispatcher, bool priority)
{
	return CreateAsyncTaskGeneric(func, dispatcher, priority);
}

LIBAPI void DwaitImplVal(AsyncTaskObj* task, std::function<void(void*, void*)> dispatcher)
{
	std::lock_guard < std::mutex > lock(task->taskMutex);

	// If it is already dispatched, just return
	// TODO try to implement exception later
	if (task->dwait)
		return;

	if (task->state == TaskState::Completed)
	{
		// If the task is already completed, just perform the dispatching
		dispatcher(task, task->retVal);
		delete task;
	}
	else if (task->state != TaskState::Cancelled)
	{
		// If the task is not completed, dispatch the completion notification
		// to event
		task->dwait = true;
		task->dwaitCaller = dispatcher;
	}
}

LIBAPI void dwait(AsyncTask<void>* task, Event<AsyncTask<void>*>& ev)
{
	auto dispatcher = [ev] (void* t, void* r)
	{
		ev(reinterpret_cast<AsyncTask<void>*>(t), nullptr);
	};

	DwaitImplVal(reinterpret_cast<AsyncTaskObj*>(task), dispatcher);
}

template<>
LIBAPI AsyncTask<void>* AsyncCall<void>(std::function<void(void)> func, std::function<void(void*, void*)> dispatcher, bool priority)
{
	return reinterpret_cast<AsyncTask<void>*>(CreateAsyncTask(std::function<void(void)>([func] () -> void
	{
		func();
	}), dispatcher, priority));
}

LIBAPI void AbortImpl(AsyncTaskObj* task)
{
	{
		std::lock_guard < std::mutex > lock(task->taskMutex);
		// If it was dwaited, cancel
		task->dwait = false;
	}

	ecore_thread_cancel(task->handle);

	AwaitImpl(task);
}

LIBAPI void AbortImplNoBlock(AsyncTaskObj* task)
{
	{
		std::lock_guard < std::mutex > lock(task->taskMutex);
		// Redirect the waiting to a dummy empty event
		task->dwait = true;
		task->dwaitCaller = [] (void* a, void* b) { };
	}

	// Sign it to cancel the thread
	ecore_thread_cancel(task->handle);
}

LIBAPI std::function<void(void*, void*)> GetDispatcher(SharedEvent<AsyncTask<void> *> ev)
{
	return [ev] (void* t, void* r)
	{
		ev(reinterpret_cast<AsyncTask<void>*>(t), nullptr);
	};
}



bool IsAborting()
{
	auto task = ecoreThreadMap[pthread_self()];

	if (task)
	{
		auto cancelling = ecore_thread_check(task->handle);
		if (cancelling)
		{
			std::lock_guard < std::mutex > lock(task->taskMutex);
			task->state = TaskState::Cancelling;
		}

		return cancelling;
	}

	return false;
}

}
