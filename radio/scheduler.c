/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <at32f421.h>
#include "app/uart.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "bsp/tmr.h"
#include "driver/key.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "task/alarm.h"
#include "task/cursor.h"
#include "task/lock.h"
#include "task/noaa.h"
#include "task/scanner.h"
#include "task/vox.h"

static uint16_t SCHEDULER_Tasks;
static uint16_t SCHEDULER_Counter;

uint32_t gPttTimeout;
uint16_t ENCRYPT_Timer;
uint32_t STANDBY_Counter;
uint32_t TMR1_Counter_6;
uint16_t gGreenLedTimer;

volatile uint16_t gSpecialTimer;
uint16_t VOX_Timer;
uint16_t gIncomingTimer;
uint16_t gVoxRssiUpdateTimer;
uint16_t gBatteryTimer;
uint16_t gSaveModeTimer;
uint32_t TMR1_Countdown_9;
uint16_t gDetectorTimer;

static void SetTask(uint16_t Task)
{
	SCHEDULER_Tasks |= Task;
}

bool SCHEDULER_CheckTask(uint16_t Task)
{
	if (SCHEDULER_Tasks & Task) {
		return true;
	}

	return false;
}

void SCHEDULER_ClearTask(uint16_t Task)
{
	SCHEDULER_Tasks &= ~Task;
}

void SCHEDULER_Init(void)
{
	tmr_para_init_ex0_type init;

	tmr_para_init_ex0(&init);
	init.period = 999;
	init.division = 72;
	init.clock_division = TMR_CLOCK_DIV1;
	init.count_mode = TMR_COUNT_UP;
	tmr_reset_ex0(TMR1, &init);
	tmr_period_buffer_enable(TMR1, TRUE);
	tmr_interrupt_enable(TMR1, TMR_OVF_INT, TRUE);
	tmr_counter_enable(TMR1, TRUE);
}

void HandlerTMR1_BRK_OVF_TRG_HALL(void)
{
	tmr_flag_clear(TMR1, TMR_OVF_FLAG);
	KEY_ReadButtons();
	KEY_ReadSideKeys();
	BEEP_Interrupt();

	if (gSpecialTimer) {
		gSpecialTimer--;
	}
	if (gEnableLocalAlarm && !gSendTone) {
		gAlarmCounter++;
	}
	if (gAudioTimer) {
		gAudioTimer--;
	}
	if (VOX_Timer) {
		VOX_Timer--;
	}
	if (gCursorCountdown) {
		gCursorCountdown--;
	}
	if (gIncomingTimer) {
		gIncomingTimer--;
	}
	if (gVoxRssiUpdateTimer) {
		gVoxRssiUpdateTimer--;
	}
	if (gBatteryTimer) {
		gBatteryTimer--;
	}
	if (NOAA_NextChannelCountdown) {
		NOAA_NextChannelCountdown--;
	}
	if (gSaveModeTimer) {
		gSaveModeTimer--;
	}
	if (TMR1_Countdown_9) {
		TMR1_Countdown_9--;
	}
	if (SCANNER_Countdown) {
		SCANNER_Countdown--;
	}
	if (gDetectorTimer) {
		gDetectorTimer--;
	}
	if (UART_Timer) {
		UART_Timer--;
	} else {
		if (UART_IsRunning) {
			UART_IsRunning = false;
		}
	}
	if (!VOX_IsTransmitting && gRadioMode == RADIO_MODE_TX) {
		gPttTimeout++;
	}
	gLockTimer++;
	SCHEDULER_Counter++;
	ENCRYPT_Timer++;
	STANDBY_Counter++;
	TMR1_Counter_6++;
	if (gBlinkGreen) {
		gGreenLedTimer++;
	}
	SetTask(TASK_CHECK_SIDE_KEYS | TASK_CHECK_KEY_PAD | TASK_CHECK_PTT);
	if ((SCHEDULER_Counter & 1) == 0) {
		SetTask(TASK_CHECK_RSSI | TASK_CHECK_INCOMING);
	}
	if ((SCHEDULER_Counter & 15) == 0) {
		SetTask(TASK_VOX);
	}
	if ((SCHEDULER_Counter & 255) == 0) {
		SetTask(TASK_FM_SCANNER | TASK_SCANNER);
	}
	if ((SCHEDULER_Counter & 0x3FF) == 0) {
		SetTask(TASK_1024_c | TASK_1024_b | TASK_CHECK_BATTERY);
		SCHEDULER_Counter = 0;
	}
}
