/*************************************************************************************************************************************************/
Author: Martin Lennartz

len_hw3_2:
	The function will create and run 2 task.  The tasks will take turns passing messages on the 2 queues
that were also created.  Each queue is uni-directional so no additional checking is required when reading.
/*************************************************************************************************************************************************/
/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

//Queue size
#define QUEUE_LENGTH		( 10 )

/* Priorities at which the tasks are created. */
#define TASK_StR_PRIORITY	( tskIDLE_PRIORITY + 1 )
#define	TASK_RtS_PRIORITY	( tskIDLE_PRIORITY + 2 )

// Tick values used for timing
#define x01_MS_TICKS		pdMS_TO_TICKS( 1UL )
#define x10_MS_TICKS		pdMS_TO_TICKS( 10UL )
#define x40_MS_TICKS		pdMS_TO_TICKS( 40UL )

// Macro used to calculate current time
#define CURRENT_MS_TIME \
	xTaskGetTickCount()/x01_MS_TICKS

//Queue handles
static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;

//Task definitions
static void q_SendThenRecTask(void *params);
static void q_RecThenSendTask(void *params);

//FIXME remove
TickType_t start_time;

void len_hw3_2(void){
	char* log = "[PROB 2 ]";

	//Queue setup
	xQueue1 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
	xQueue2 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));

	//Task creation
	xTaskCreate(q_SendThenRecTask, "Tx_Rx", configMINIMAL_STACK_SIZE, NULL, TASK_StR_PRIORITY, NULL);
	xTaskCreate(q_RecThenSendTask, "Rx_Tx", configMINIMAL_STACK_SIZE, NULL, TASK_RtS_PRIORITY, NULL);

	//FIXME - remove
	start_time = xTaskGetTickCount();
	printf("%5d ms %s Starting.\n", CURRENT_MS_TIME, log);
	
	//Start tasks
	vTaskStartScheduler();
}
/****************************************************************
q_SendThenRecTask - Send a message, wait to receive a message. 
Continue in loop.
****************************************************************/
static void q_SendThenRecTask(void *params)
{
	const unsigned int   ulValueToSend = 1;
	const unsigned int   loop_cnt      = 10;
		  unsigned int   ulReceivedValue, cnt, ii,jj;
	               char* log = "[SENDREC]";

	/* Prevent the compiler warning about the unused parameter. */
	(void)params;

	// Loop count added to stall the system after 10 message pairs
	// to be able to grab a screen shot.
	for (cnt=0;cnt<loop_cnt;cnt++)
	{
		printf("%5d ms %s Sending message to Q1: %0d\n", CURRENT_MS_TIME, log, ulValueToSend);
		xQueueSend(xQueue1, &ulValueToSend, 0U);
		xQueueReceive(xQueue2, &ulReceivedValue, portMAX_DELAY);
		printf("%5d ms %s Message received on Q2: %0d\n", CURRENT_MS_TIME, log, ulReceivedValue);
		printf("%5d ms %s Task SendThenRec running\n", CURRENT_MS_TIME, log);
		//vTaskDelay(x10_MS_TICKS);
		//Processing portion
		//Used for as a generic delay
		for (ii = 0; ii < 9605; ii++){
			for (jj = 0; jj < 1000; jj++);
		}
	}
}
/****************************************************************
q_RecThenSendTask - Wait to receive a message, then delay and sent
message back. Continue in loop.
****************************************************************/
static void q_RecThenSendTask(void *params)
{
	const unsigned int   ulValueToSend = 2;
		  unsigned int   ulReceivedValue, ii,jj;
	               char* log = "[RECSEND]";

	/* Prevent the compiler warning about the unused parameter. */
	(void)params;

	for (;;)
	{
		xQueueReceive(xQueue1, &ulReceivedValue, portMAX_DELAY);
		printf("%5d ms %s Message received on Q1: %0d\n", CURRENT_MS_TIME, log, ulReceivedValue);
		printf("%5d ms %s Task RecThenSend running\n", CURRENT_MS_TIME, log);
		//vTaskDelay(x40_MS_TICKS);
		//Processing portion
		//Used for as a generic delay
		for (ii = 0; ii < 32800; ii++){
			for (jj = 0; jj < 1000; jj++);
		}
		printf("%5d ms %s Sending message to Q2: %0d\n", CURRENT_MS_TIME, log, ulValueToSend);
		xQueueSend(xQueue2, &ulValueToSend, 0U);
	}
}

