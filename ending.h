#ifndef ENDING_H
#define ENDING_H

#include "OBSApi.h"

class ending
{
	DWORD WaitResult;
	HANDLE Mutex;
	bool ended;
public:
	ending()
	{
		ended = false;
		Mutex = CreateMutex(NULL, false, NULL);
	}

	void now()
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		ended = true;
		ReleaseMutex(Mutex);
	}
	bool state()
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		bool b = ended;
		ReleaseMutex(Mutex);
		return b;
	}
};

#endif //ENDING_H