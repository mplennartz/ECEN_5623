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
#define QUEUE_LENGTH		( 10 )
#define TASK_StR_PRIORITY	( tskIDLE_PRIORITY + 1 )
#define	TASK_RtS_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define x01_MS_TICKS		pdMS_TO_TICKS( 1UL )
#define x10_MS_TICKS		pdMS_TO_TICKS( 10UL )
#define x40_MS_TICKS		pdMS_TO_TICKS( 40UL )

#define CURRENT_MS_TIME \
	xTaskGetTickCount()/x01_MS_TICKS

static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;

static void q_SendThenRecTask(void *params);
static void q_RecThenSendTask(void *params);

TickType_t start_time;

void len_hw3_2(void){
	char* log = "[PROB 2 ]";

	xQueue1 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));
	xQueue2 = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));

	xTaskCreate(q_SendThenRecTask, "Tx_Rx", configMINIMAL_STACK_SIZE, NULL, TASK_StR_PRIORITY, NULL);
	xTaskCreate(q_RecThenSendTask, "Rx_Tx", configMINIMAL_STACK_SIZE, NULL, TASK_RtS_PRIORITY, NULL);

	start_time = xTaskGetTickCount();
	printf("%5d ms %s Starting.\n", CURRENT_MS_TIME, log);
	vTaskStartScheduler();
}
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

