/*************************************************************************************************************************************************/
Author: Martin Lennartz

len_hw3_1:
	The function will create and run 1 task and 1 timer.  The timer is setup to auto-reload and pass the time
to the task using the defined queue.  The timer also will resume the task.
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
#define TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define TIMER_FREQUENCY_MS	pdMS_TO_TICKS( 2500UL )

// Handles
static TimerHandle_t xTimer      = NULL;
// Used to suspend resume task
static TaskHandle_t  xProcessing = NULL;

//Task / Timer definition
static void t_processing(void *params);
static void t_timerCallback(TimerHandle_t xTimerHandle);

//Added after rereading the problem description
//Handle, size of queue
#define QUEUE_LENGTH		( 10 )
static  QueueHandle_t xQueue = NULL;

// Global variable to pass data
int data_to_process;

void len_hw3_1(void){
	const TickType_t xTimerPeriod = TIMER_FREQUENCY_MS;
		  BaseType_t xTimerStarted;

	//Create tasks, queue, timer
	xTaskCreate(t_processing, "Processing", configMINIMAL_STACK_SIZE, NULL, TASK_PRIORITY, &xProcessing);
	xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
	//Auto-reload timer (opposed to one-shot timer; would use pdFalse)
	xTimer = xTimerCreate("Timer",xTimerPeriod,pdTRUE,NULL,t_timerCallback);
	//Start timer
	xTimerStarted = xTimerStart(xTimer, 0);

	vTaskStartScheduler();
}
static void t_processing(void *params){
	(void)params;
	char* log = "[PROCESS]";
	uint32_t ulReceivedValue;

	for (;;){
		// Suspending task assocaited with "xProcessing"
		// which happens to be this task
		vTaskSuspend(xProcessing); 
		xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);
		printf("%s Processing task received : %0d\n", log, ulReceivedValue);
		printf("%s Processing task running\n", log);
		// Slight delay to act as processing
		vTaskDelay(1500);	
		printf("%s Data output = %0d\n", log, data_to_process);
	}
}

static void t_timerCallback(TimerHandle_t xTimerHandle){
	uint32_t ulValueToSend;
	
	(void)xTimerHandle;

	char* log = "[ TIMER ]";
	printf("%s Data updating\n", log);
	data_to_process += 10;
	printf("%s Timer resuming processing\n", log); 
	ulValueToSend = xTaskGetTickCount();
	xQueueSend(xQueue, &ulValueToSend, 0U);
	vTaskResume(xProcessing);
}