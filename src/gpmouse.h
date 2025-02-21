#pragma once

#include <stdint.h>
#include <concurrent_queue.h>
#include "resource.h"


#define GP_STATUS_INITIALIZING	0u
#define GP_STATUS_READY			1u
#define GP_STATUS_TERMINATING	2u
#define GP_STATUS_SUSPENDED		3u

//struct app_status_t
//{
//	uint32_t 
//};

struct xinput_t
{
	int device;
	DWORD timestamp;
	WORD buttons;
};

using Concurrency::concurrent_queue;


extern void check_xinput(uint32_t* pstatus, concurrent_queue<xinput_t>* queue);
extern void handle_xinput(uint32_t* pstatus, concurrent_queue<xinput_t>* queue);

