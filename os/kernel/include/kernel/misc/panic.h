#pragma once

#include <kernel/global.h>
#include <kernel/module/symtable.h>

void kpanic(crash_info_t crash) KERNEL_EXPORT_A1("kpanic",F_PTR_A1(void,crash_info_t),crash)
