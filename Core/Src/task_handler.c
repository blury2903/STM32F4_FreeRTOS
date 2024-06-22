/*
 * task_handler.c
 *
 *  Created on: Jun 19, 2024
 *      Author: blury
 */

#include "main.h"

int32_t extract_cmd(command_t *cmd);
void process_cmd(command_t *cmd);

const char* invalid_msg = "Invalid command, please enter again.\n";

void menu_task(void *param)
{
	const char* msg_menu = "\n========================\n"
			"|         Menu         |\n"
			"========================\n"
			"LED effect    ----> 0\n"
			"Date and time ----> 1\n"
			"Exit          ----> 2\n"
			"Enter your choice here : ";
	uint32_t cmd_addr;

	command_t *cmd;

	uint32_t option;
	while(1){
		xQueueSend(q_print, &msg_menu, portMAX_DELAY);

		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);

		cmd = (command_t*)cmd_addr;
		if(cmd->len == 1) {
			option = cmd->payload[0] - 48;

			switch(option){
			case 0:
				curr_state = sLedEffect;
				xTaskNotify(handle_led_task, 0, eNoAction);
				break;

			case 1:
				curr_state = sRtcMenu;
				xTaskNotify(handle_rtc_task, 0, eNoAction);
				break;

			case 2:

				break;

			default:
				xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
				continue;
			}

			//wait to run again
			xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		} else{
			xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
		}
	}
}

void cmd_task(void *param)
{
	BaseType_t ret;
	command_t cmd;
	while(1){
		/*Implement notify wait*/
		ret = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		if(ret == pdTRUE){
			/*process the user data(cmd) stored in input data queue*/
			process_cmd(&cmd);
		}

	}
}

void process_cmd(command_t *cmd)
{
	extract_cmd(cmd);

	switch(curr_state){
	case sMainMenu:
		xTaskNotify(handle_menu_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sLedEffect:
		xTaskNotify(handle_led_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sRtcMenu:
		xTaskNotify(handle_rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sRtcTimeConfig:
		xTaskNotify(handle_rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sRtcDateConfig:
		xTaskNotify(handle_rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sRtcReport:
		xTaskNotify(handle_rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	}
}

int32_t extract_cmd(command_t *cmd)
{
	uint8_t item;
	BaseType_t status;

	status = uxQueueMessagesWaiting(q_data);
	if(!status) return -1;
	uint8_t i = 0;
	do {
		status = xQueueReceive(q_data, &item, 0);
		if(status == pdTRUE) cmd->payload[i++] = item;

	} while(item != '\n');

	cmd->payload[i - 1] = '\0';
	cmd->len = i - 1;

	return 0;

}

void print_task(void *param)
{
	uint32_t *msg;
	while(1){
		xQueueReceive(q_print, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen((char*)msg), HAL_MAX_DELAY);
	}
}

void led_task(void *param)
{
	uint32_t cmd_addr;
	command_t *cmd;
	const char* msg_led = "========================\n"
			"|      LED Effect     |\n"
			"========================\n"
			"(none,e1,e2,e3,e4)\n"
			"Enter your choice here : ";

	while(1){
		/* Wait for notification (Notify wait) */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/* Print LED menu */
		xQueueSend(q_print, &msg_led, portMAX_DELAY);

		/* wait for LED command (Notify wait) */
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (command_t*)cmd_addr;

		if(cmd->len <= 4)
		{
			if(! strcmp((char*)cmd->payload,"none"))
				led_effect_stop();
			else if (! strcmp((char*)cmd->payload,"e1"))
				led_effect(1);
			else if (! strcmp((char*)cmd->payload,"e2"))
				led_effect(2);
			else if (! strcmp((char*)cmd->payload,"e3"))
				led_effect(3);
			else if (! strcmp((char*)cmd->payload,"e4"))
				led_effect(4);
			else
				/* print invalid message */
				xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
		}else
			/* print invalid message */
			xQueueSend(q_print, &invalid_msg, portMAX_DELAY);

		/* update state variable */
		curr_state = sMainMenu;

		/* Notify menu task */
		xTaskNotify(handle_menu_task,0,eNoAction);

	}
}

uint8_t cmd2number(command_t *cmd)
{
	uint8_t number;
	if(cmd->len > 1){
		number = ((cmd->payload[0] - 48) * 10) + (cmd->payload[1] - 48);
	} else{
		number = cmd->payload[0] - 48;
	}
	return number;
}

void rtc_task(void *param)
{
	const char* msg_rtc1 = "========================\n"
			"|         RTC          |\n"
			"========================\n";

	const char* msg_rtc2 = "Configure Time            ----> 0\n"
			"Configure Date            ----> 1\n"
			"Enable reporting          ----> 2\n"
			"Exit                      ----> 3\n"
			"Enter your choice here : ";


	const char *msg_rtc_hh = "Enter hour(1-12):";
	const char *msg_rtc_mm = "Enter minutes(0-59):";
	const char *msg_rtc_ss = "Enter seconds(0-59):";

	const char *msg_rtc_dd  = "Enter date(1-31):";
	const char *msg_rtc_mo  ="Enter month(1-12):";
	const char *msg_rtc_yr  = "Enter year(0-99):";

	const char *msg_conf = "Configuration successful\n";
	const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


	uint32_t cmd_addr;
	command_t *cmd;

	RTC_TimeTypeDef time_set;
	RTC_DateTypeDef date_set;
	int option;
	rtc_config_t rtc_state;


	while(1){
		/* Notify wait (wait till someone notifies)		 */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/* Print the menu and show current date and time information */
		show_time_date();

		xQueueSend(q_print, &msg_rtc1, portMAX_DELAY);

		xQueueSend(q_print, &msg_rtc2, portMAX_DELAY);



		while(curr_state != sMainMenu){

			/* Wait for command notification (Notify wait) */
			xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
			cmd = (command_t*)cmd_addr;

			switch(curr_state)
			{
			case sRtcMenu:{

				/* process RTC menu commands */
				if(cmd->len == 1){
					option = cmd->payload[0] - 48;
					if(option == 0){
						curr_state = sRtcTimeConfig;
						xQueueSend(q_print, &msg_rtc_hh, portMAX_DELAY);
						rtc_state = hh_Config;
					} else if(option == 1){
						curr_state = sRtcDateConfig;
						xQueueSend(q_print, &msg_rtc_dd, portMAX_DELAY);
						rtc_state = dd_Config;
					} else if(option == 2){
						curr_state = sRtcReport;
						xQueueSend(q_print, &msg_rtc_report, portMAX_DELAY);
					} else if(option == 3){
						curr_state = sMainMenu;
					} else
						xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
				}
				break;}

			case sRtcTimeConfig:{
				/*TODO : get hh, mm, ss infor and configure RTC */
				switch (rtc_state)
				{
				case hh_Config:
					uint8_t hour = cmd2number(cmd);
					time_set.Hours = hour;
					xQueueSend(q_print, &msg_rtc_mm, portMAX_DELAY);
					rtc_state = mm_Config;
					break;
				case mm_Config:
					uint8_t minute = cmd2number(cmd);
					time_set.Minutes = minute;
					xQueueSend(q_print, &msg_rtc_ss, portMAX_DELAY);
					rtc_state = ss_Config;
					break;
				case ss_Config:
					uint8_t second = cmd2number(cmd);
					time_set.Seconds = second;
					if(!validate_rtc_information(&time_set, NULL)){
						rtc_configure_time(&time_set);
						xQueueSend(q_print, &msg_conf, portMAX_DELAY);
						show_time_date();
						curr_state = sMainMenu;
						rtc_state = df_Config;
					} else{
						xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
						curr_state = sMainMenu;
						rtc_state = df_Config;
					}
					break;
				default:
					break;
				}
				break;}

			case sRtcDateConfig:{

				switch (rtc_state)
				{
				case dd_Config:
					uint8_t day = cmd2number(cmd);
					date_set.Date = day;
					xQueueSend(q_print, &msg_rtc_mo, portMAX_DELAY);
					rtc_state = mo_Config;
					break;
				case mo_Config:
					uint8_t month = cmd2number(cmd);
					date_set.Month = month;
					xQueueSend(q_print, &msg_rtc_yr, portMAX_DELAY);
					rtc_state = yr_Config;
					break;
				case yr_Config:
					uint8_t year = cmd2number(cmd);
					date_set.Year = year;
					if(!validate_rtc_information(NULL, &date_set)){
						rtc_configure_date(&date_set);
						xQueueSend(q_print, &msg_conf, portMAX_DELAY);
						show_time_date();
					} else{
						xQueueSend(q_print, &invalid_msg, portMAX_DELAY);

					}
					curr_state = sMainMenu;
					rtc_state = df_Config;
					break;
				default:
					break;
				}
				break;}

			case sRtcReport:{
				if(cmd->len == 1){
					if(cmd->payload[0] == 'y'){
						if(xTimerIsTimerActive(rtc_timer) == pdFALSE){
							xTimerStart(rtc_timer, portMAX_DELAY);
							xQueueSend(q_print, &msg_conf, portMAX_DELAY);
						}
					} else if(cmd->payload[0] == 'n'){
						xTimerStop(rtc_timer, portMAX_DELAY);
						xQueueSend(q_print, &msg_conf, portMAX_DELAY);
					} else
						xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
				} else
					xQueueSend(q_print, &invalid_msg, portMAX_DELAY);
				curr_state = sMainMenu;

				break;}
			default:
				break;


			}// switch end

		} //while end

		/* Notify menu task */
		xTaskNotify(handle_menu_task, 0, eNoAction);

	}//while super loop end
}


