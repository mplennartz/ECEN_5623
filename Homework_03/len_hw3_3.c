/*************************************************************************************************************************************************/

/*************************************************************************************************************************************************/
/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Priorities at which the tasks are created. */
//#define QUEUE_LENGTH		( 10 )
#define TASK_1_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	TASK_2_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define x01_MS_TICKS		pdMS_TO_TICKS( 1UL )
//#define x10_MS_TICKS		pdMS_TO_TICKS( 10UL )
//#define x40_MS_TICKS		pdMS_TO_TICKS( 40UL )

#define TIMER_FREQUENCY_MS	pdMS_TO_TICKS( 10UL )

#define CURRENT_MS_TIME \
	xTaskGetTickCount()/x01_MS_TICKS

//static QueueHandle_t xQueue1 = NULL;
//static QueueHandle_t xQueue2 = NULL;

static TaskHandle_t  x_t2   = NULL;
static TaskHandle_t  x_t1   = NULL;
static TimerHandle_t xTimer = NULL;

static void q_Task1(void *params);
static void q_Task2(void *params);
static void t_timerCallback(TimerHandle_t xTimerHandle);

TickType_t start_time;

void len_hw3_3(void){
	const TickType_t xTimerPeriod = TIMER_FREQUENCY_MS;
	BaseType_t xTimerStarted;
	char* log = "[PROB 3 ]";

//	xQueue1 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
//	xQueue2 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));

	xTaskCreate(q_Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK_1_PRIORITY, &x_t1);
	xTaskCreate(q_Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK_2_PRIORITY, &x_t2);

	//Auto-reload timer (opposed to one-shot timer; would use pdFalse)
	xTimer = xTimerCreate("Timer", xTimerPeriod, pdTRUE, NULL, t_timerCallback);
	//Start timer
	xTimerStarted = xTimerStart(xTimer, 0);

	printf("%5d ms %s Starting.\n", CURRENT_MS_TIME, log);
	vTaskStartScheduler();
}
static void q_Task1(void *params)
{
//	const unsigned int   ulValueToSend = 1;
	//const unsigned int   loop_cnt = 10;
	//unsigned int   ulReceivedValue, cnt, ii, jj;
	unsigned int   ii, jj;
	char* log = "[TASK 1 ]";

	/* Prevent the compiler warning about the unused parameter. */
	(void)params;

	// Loop count added to stall the system after 10 message pairs
	// to be able to grab a screen shot.
	for (;;)
	{
		vTaskSuspend(x_t1);
		printf("%5d ms %s Task 1 Resumed; Running\n", CURRENT_MS_TIME, log);
		//vTaskDelay(x10_MS_TICKS);
		//Processing portion
		//Used for as a generic delay
		for (ii = 0; ii < 9605; ii++){
			for (jj = 0; jj < 1000; jj++);
		}
		printf("%5d ms %s Task 1 finished running; Suspending\n", CURRENT_MS_TIME, log);
	}
}
static void q_Task2(void *params)
{
//	const unsigned int   ulValueToSend = 2;
//	unsigned int   ulReceivedValue, ii, jj;
	unsigned int    ii, jj;
	char* log = "[TASK 2 ]";

	/* Prevent the compiler warning about the unused parameter. */
	(void)params;

	for (;;)
	{
		vTaskSuspend(x_t2);
		printf("%5d ms %s Task 2 Resumed; Running\n", CURRENT_MS_TIME, log);
		//vTaskDelay(x40_MS_TICKS);
		//Processing portion
		//Used for as a generic delay
		for (ii = 0; ii < 32800; ii++){
			for (jj = 0; jj < 1000; jj++);
		}
		printf("%5d ms %s Task 2 finished running; Suspending\n", CURRENT_MS_TIME, log);
	}
}
static void t_timerCallback(TimerHandle_t xTimerHandle){
	(void)xTimerHandle;
	static unsigned int cnt = 0;

	if (cnt == 23){
		//		cnt = 0;
		cnt = 25;
	}
	else if (cnt == 25){}
	else
	{
		cnt += 1;
	}

	if (cnt % 3 == 0){
		vTaskResume(x_t1);
	}
	if (cnt % 8 == 0){
		vTaskResume(x_t2);
	}



}