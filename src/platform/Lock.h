
#ifndef ARX_PLATFORM_LOCK_H
#define ARX_PLATFORM_LOCK_H

#include "platform/Platform.h"

#ifdef HAVE_PTHREADS
#include <pthread.h>
#else
#include <windows.h>
#endif

class Lock {
	
private:
	
#ifdef HAVE_PTHREADS
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool locked;
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	HANDLE mutex;
#else
	#warning "locking not supported"
#endif
	
public:
	
	Lock();
	~Lock();
	
	void lock();
	
	void unlock();
	
};

class Autolock {
	
private:
	
	Lock * lock;
	
public:
	
	inline Autolock(Lock * _lock) : lock(_lock) {
		lock->lock();
	}
	
	inline ~Autolock() {
		lock->unlock();
	}
	
};

#endif // ARX_PLATFORM_LOCK_H
