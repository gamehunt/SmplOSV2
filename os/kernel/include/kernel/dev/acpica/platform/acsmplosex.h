#ifndef __ACSMPLOSEX_H__
#define __ACSMPLOSEX_H__

#include <kernel/memory/memory.h>
#include <kernel/proc/proc.h>
#include <kernel/proc/sync.h>

ACPI_STATUS AcpiOsInitialize();
ACPI_STATUS AcpiOsTerminate();
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer();
ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue);
ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable);
void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length);
void AcpiOsUnmapMemory(void* v_addr, ACPI_SIZE Length);
ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress);
void *AcpiOsAllocate(ACPI_SIZE Size);
void AcpiOsFree(void* mem);
BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length);
BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length);
ACPI_THREAD_ID AcpiOsGetThreadId();
#endif
