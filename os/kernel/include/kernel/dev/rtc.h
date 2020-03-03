/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#define RTC_SEC 0x0
#define RTC_MIN 0x2
#define RTC_HRS 0x4
#define RTC_WDAY 0x6
#define RTC_MDAY 0x7
#define RTC_MONTH 0x8
#define RTC_YEAR 0x9
#define RTC_CENT 0x32

#define RTC_ST_A 0x0A
#define RTC_ST_B 0x0B

uint8_t rtc_get_update_flag();
void init_rtc();
uint32_t rtc_current_time();
