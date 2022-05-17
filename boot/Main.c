// define native type
#include "stdint.h"
// define Uart API
#include "HalUart.h"
// define debug_printf()
#include "stdio.h"
// define Interrupt API
#include "HalInterrupt.h"
// define Timer API
#include "HalTimer.h"
// define true or false
#include "stdbool.h"
// define delay()
#include "stdlib.h"
// define TSK API
#include "task.h"
// define Kernel scheduling API
#include "kernel.h"
#include "event.h"

static uint32_t shared_value;
static void Test_critical_section(uint32_t p, uint32_t taskId);

static void Hw_init(void);
static void Printf_test(void);
static void Timer_test(void);
static void Kernel_init(void);

void User_task0(void);
void User_task1(void);
void User_task2(void);

void main(void)
{
	Hw_init();

	uint32_t i = 100;
	while(i--)
	{
		Hal_uart_put_char('N');
	}
	Hal_uart_put_char('\n');

	putstr("Hello World!\n");

	Printf_test();

	Timer_test();

	Kernel_init();

	//while(true);

	i = 100;
	while(i--)
	{
		uint8_t ch = Hal_uart_get_char();
		Hal_uart_put_char(ch);
	}
}

static void Hw_init(void)
{
	Hal_interrupt_init();
	Hal_uart_init();
	Hal_timer_init();
}

static void Printf_test(void)
{
	char* str = "printf pointer test";
	char* nullptr = 0;
	uint32_t i = 5;

	debug_printf("%s\n", "Hello printf");
	debug_printf("output string pointer: %s\n", str);
	debug_printf("%s is null pointer, %u number\n", nullptr, 10);
	debug_printf("%u = 5\n", i);
	debug_printf("dec=%u hex=%x\n", 0xff, 0xff);
	debug_printf("print zero %u\n", 0);
}

static void Timer_test(void)
{
	for (uint32_t i = 0; i < 5; i++)
	{
		debug_printf("current count : %u\n", Hal_timer_get_1ms_counter());
		delay(1000);
	}
}

static void Kernel_init(void)
{
	uint32_t taskId;

	Kernel_task_init();
	Kernel_event_init();
	Kernel_msgQ_init();
	Kernel_sem_init(1);

	taskId = Kernel_task_create(User_task0);
	if (NOT_ENOUGH_TASK_NUM == taskId)
	{
		putstr("Task0 creation fail\n");
	}

	taskId = Kernel_task_create(User_task1);
	if (NOT_ENOUGH_TASK_NUM == taskId)
	{
		putstr("Task1 creation fail\n");
	}
	taskId = Kernel_task_create(User_task2);
	if (NOT_ENOUGH_TASK_NUM == taskId)
	{
		putstr("Task2 creation fail\n");
	}

	Kernel_start();
}

static void Test_critical_section(uint32_t p, uint32_t taskId)
{
	//Kernel_lock_sem();
	Kernel_lock_mutex();

	debug_printf("User Task #%u Send=%u\n", taskId, p);
	shared_value = p;
	Kernel_yield();
	delay(1000);
	debug_printf("User Task #%u Shared Value=%u\n", taskId, shared_value);

	//Kernel_unlock_sem();
	Kernel_unlock_mutex();
}

void User_task0(void)
{
	uint32_t local = 0;
	
	debug_printf("User Task #0 SP=0x%x\n", &local);

	uint8_t cmdBuf[16];
	uint32_t cmdBufIdx = 0;
	uint8_t uartch = 0;

	while(true)
	{
		KernelEventFlag_t handle_event = Kernel_wait_events(KernelEventFlag_UartIn|
				KernelEventFlag_CmdOut);
		switch(handle_event)
		{
			case KernelEventFlag_UartIn:
				Kernel_recv_msg(KernelMsgQ_Task0, &uartch, 1);
				if (uartch == '\r')
				{
					cmdBuf[cmdBufIdx] = '\0';

					Kernel_send_msg(KernelMsgQ_Task1, &cmdBufIdx, 1);
					Kernel_send_msg(KernelMsgQ_Task1, cmdBuf, cmdBufIdx);			
					Kernel_send_events(KernelEventFlag_CmdIn);		

					cmdBufIdx = 0;
				}
				else
				{
					cmdBuf[cmdBufIdx] = uartch;
					cmdBufIdx++;
					cmdBufIdx %= 16;
				}
				break;
			case KernelEventFlag_CmdOut:
				Test_critical_section(5, 0);
				break;
		}
		Kernel_yield();
	}
}

void User_task1(void)
{
	uint32_t local = 0;
	
	debug_printf("User Task #1 SP=0x%x\n", &local);
	
	uint8_t cmdlen = 0;
	uint8_t cmd[16] = {0};

	while(true)
	{
		KernelEventFlag_t handle_event = Kernel_wait_events(
				KernelEventFlag_CmdIn|KernelEventFlag_Unlock);
		switch(handle_event)
		{
			case KernelEventFlag_CmdIn:
				memclr(cmd, 16);
				Kernel_recv_msg(KernelMsgQ_Task1, &cmdlen, 1);
				Kernel_recv_msg(KernelMsgQ_Task1, cmd, cmdlen);
				debug_printf("\nRecv Cmd: %s\n", cmd);
				break;
			case KernelEventFlag_Unlock:
				//Kernel_unlock_sem();
				Kernel_unlock_mutex();
				break;
		}
		Kernel_yield();
	}
}

void User_task2(void)
{
	uint32_t local = 0;

	debug_printf("User Task #2 SP=0x%x\n", &local);

	while(true)
	{
		Test_critical_section(3, 2);
		/*
		KernelEventFlag_t handle_event = Kernel_wait_events(KernelEventFlag_CmdOut);
		switch(handle_event)
		{
			case KernelEventFlag_CmdOut:
				debug_printf("\nEvent handled by Task3\n");
				break;
		}
		*/
		Kernel_yield();
	}
}