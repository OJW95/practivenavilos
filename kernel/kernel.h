#ifndef KERNEL_KERNEL_H_
#define KERNEL_KERNEL_H_

#include "task.h"
#include "event.h"
#include "msg.h"
#include "synch.h"

// scheduling
void Kernel_yield(void);
// task 0
void Kernel_start(void);
// event
void Kernel_send_events(uint32_t event_list);
KernelEventFlag_t Kernel_wait_events(uint32_t waiting_list);
void Kernel_event_init(void);

// messaging
bool Kernel_send_msg(KernelMsgQ_t Qname, void* data, uint32_t count);
uint32_t Kernel_recv_msg(KernelMsgQ_t Qname, void* out_data, uint32_t count);

// semaphore
void Kernel_lock_sem(void);
void Kernel_unlock_sem(void);

// mutex
void Kernel_lock_mutex(void);
void Kernel_unlock_mutex(void);

#endif