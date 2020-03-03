/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/dev/cmos.h>
#include <kernel/dev/rtc.h>

uint8_t rtc_get_update_flag(){
	uint8_t data = cmos_read_value(RTC_ST_A);
	return data & 0x80;
}

void init_rtc(){
	uint8_t data = cmos_read_value(RTC_ST_B);
	cmos_write_value(RTC_ST_B,data | 0x02 | 0x04);
}

//don't count v. years!
uint32_t rtc_current_time(){
	while(rtc_get_update_flag());
	uint32_t years_elapsed = (cmos_read_value(RTC_CENT)*100+cmos_read_value(RTC_YEAR)) - 1970;
	uint32_t month_elapsed = cmos_read_value(RTC_MONTH) - 1;
	uint32_t secs_elapsed = cmos_read_value(RTC_MDAY)*24*3600 + cmos_read_value(RTC_HRS)*3600 + cmos_read_value(RTC_MIN)*60 + cmos_read_value(RTC_SEC);
	return ((years_elapsed * 31556952)+(month_elapsed * 2629746)+secs_elapsed);
}
