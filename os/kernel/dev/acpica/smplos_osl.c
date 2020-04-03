#include "acpi.h"
#include <kernel/global.h>
#include <kernel/memory/memory.h>
#include <kernel/proc/proc.h>
#include <kernel/proc/sync.h>
#include <kernel/interrupts/irq.h>
#include <kernel/io/io.h>
#include <kernel/dev/pit.h>
#include <kernel/dev/pci.h>

static ACPI_OSD_HANDLER ServiceRout;
static void* ServiceCtx;

#if ACPI_MACHINE_WIDTH == 32 && !defined(__div64_32)
uint32_t __attribute__((weak)) __div64_32(uint64_t *n, uint32_t base)
{
	uint64_t rem = *n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t) high << 32;
		rem -= (uint64_t) (high*base) << 32;
	}

	while ((int64_t)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}
#endif


ACPI_STATUS AcpiOsInitialize(){
	return AE_OK;
}
ACPI_STATUS AcpiOsTerminate(){
	return AE_OK;
}
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
	ACPI_PHYSICAL_ADDRESS  Ret;
	Ret = 0;
	AcpiFindRootPointer(&Ret);
	return Ret;
}
ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue){
	*NewValue = 0;
	return AE_OK;
}
ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable){
	*NewTable = 0;
	return AE_OK;
}
ACPI_STATUS AcpiOsPhysicalTableOverride (ACPI_TABLE_HEADER *ExistingTable,ACPI_PHYSICAL_ADDRESS *NewAddress,UINT32 *NewTableLength){
	*NewAddress = 0;
	return AE_OK;
}
void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length){
	if(!Length){
		Length = 1;
	}
	uint32_t ret_value = ACPICA_BASE_ADDRESS + PhysicalAddress;
	uint32_t aligned_size = 0;
	uint32_t old_paddr = PhysicalAddress;
	while(aligned_size < Length){
		aligned_size  += 4096; //There should be better way, but i'm too lazy to think about it
	}
	//PhysicalAddress -= PhysicalAddress%4096;
	uint32_t v_addr = ACPICA_BASE_ADDRESS + PhysicalAddress - PhysicalAddress%4096;
	for(uint32_t i=0;i<aligned_size/4096;i++){
		kmpalloc(v_addr+i*4096,PhysicalAddress+i*4096,0);
	}
	return ret_value;
}
void AcpiOsUnmapMemory(void* v_addr, ACPI_SIZE Length){
	uint32_t aligned_size = 0;
	while(aligned_size < Length){
		aligned_size  += 4096; //There should be better way, but i'm too lazy to think about it
	}
	v_addr = (void*)((uint32_t)v_addr - (uint32_t)v_addr%4096);
	//kinfo("ACPI Trying to free %a\n",v_addr);
	for(uint32_t i=0;i<aligned_size/4096;i++){
		
		//kpfree_virtual((uint32_t)v_addr+i*4096);
	}
}
ACPI_STATUS AcpiOsGetPhysicalAddress(void *LogicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress){
	*PhysicalAddress =  virtual2physical((uint32_t)LogicalAddress);
	return AE_OK;
}
void *AcpiOsAllocate(ACPI_SIZE Size){
	//kinfo("ACPI ALLOC(%d)\n",Size);
	return kmalloc(Size);
}
void AcpiOsFree(void* mem){
	kfree(mem);
}
BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length){
	uint32_t size_in_pages = 0;
	while(size_in_pages < Length){
		size_in_pages += 4096; //There should be better way, but i'm too lazy to think about it
	}
	for(uint32_t i=0;i<size_in_pages;i++){
		if(!virtual2physical((uint32_t)Memory + i*4096)){
			return 0;
		}
	}
	return 1;
}
BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length){
	return AcpiOsReadable(Memory,Length);
}

ACPI_THREAD_ID AcpiOsGetThreadId(){
	if(!get_current_process()){
		return 1;
	}
	return get_current_process()->pid;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void *Context){
	//TODO
	kwarn("NON_IMPL_CALL: AcpiOsExecute()\n");
	return AE_OK;
}

void AcpiOsSleep(UINT64 Milliseconds){
	process_sleep(get_current_process(),Milliseconds); //Todo convert to ticks
}

ACPI_STATUS AcpiOsEnterSleep(UINT8 State,UINT32 ra,UINT32 rb){
	return AE_OK;
}

void AcpiOsStall(UINT32 Microseconds){
	//???
	kwarn("NON_IMPL_CALL: AcpiOsStall()\n");
}

ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle){
	
	return AcpiOsCreateLock(OutHandle);
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle){
	
	AcpiOsDeleteLock(&Handle);
	return AE_OK;
}
ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout){
	if((*(spinlock_t*)Handle) && !Timeout){
		return AE_OK;
	} 
	if((*(spinlock_t*)Handle) && Timeout < 0){
		return AcpiOsAcquireLock(&Handle);
	}
	if((*(spinlock_t*)Handle) && Timeout > 0){
		while(Timeout > 0){
			if(!(*(spinlock_t*)Handle)){
				return AcpiOsAcquireLock(&Handle);
			}
			Timeout--;
		} 
		return AE_OK;
	}
}
ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units){
	
	AcpiOsReleaseLock(Handle,0);
}
ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle){
	*OutHandle = kmalloc(sizeof(ACPI_SPINLOCK));
	return AE_OK;
}
void AcpiOsDeleteLock(ACPI_SPINLOCK Handle){
	kfree(Handle);
}
ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle){
	asm("cli");
	lock_spin(Handle);
	return 0;
}
void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags){
	unlock_spin(Handle);
	asm("sti");
}

static void acpi_irq_wrapper(regs_t regs)
{
	ServiceRout(ServiceCtx);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNum, ACPI_OSD_HANDLER Handler, void *Context){
	ServiceRout = Handler;
	ServiceCtx = Context;
	irq_set_handler(InterruptNum,acpi_irq_wrapper);
	return AE_OK;
}
ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler){
	irq_unset_handler(InterruptNumber);
	return AE_OK;
}
ACPI_STATUS AcpiOsReadPort (ACPI_IO_ADDRESS Address,UINT32 *Value,UINT32 Width){
	switch(Width){
		case 8:
			*Value = inb(Address);
		break;
		case 16:
			*Value = inw(Address);
		break;
		case 32:
			*Value = inl(Address);
		break;
	}
	return AE_OK;
}
ACPI_STATUS AcpiOsWritePort (ACPI_IO_ADDRESS Address,UINT32 Value,UINT32 Width){
	switch(Width){
		case 8:
			outb(Address,Value);
		break;
		case 16:
			outw(Address,Value);
		break;
		case 32:
			outl(Address,Value);
		break;
	}
	return AE_OK;
}

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf (const char *Format,...){
	va_list argptr;
	va_start(argptr,Format);
	vprintf(Format,argptr);
	va_end(argptr);
}
void AcpiOsVprintf (const char *Format,va_list Args){
	vprintf(Format,Args);
}

ACPI_STATUS AcpiOsReadMemory (ACPI_PHYSICAL_ADDRESS Address,UINT64 *Value,UINT32 Width){
	if(AcpiOsReadable(Address,Width/4)){
		memset(Value,0,16);
		memcpy(Value,Address,Width/4);
		return AE_OK;
	}
	return AE_ERROR;
}
ACPI_STATUS AcpiOsWriteMemory (ACPI_PHYSICAL_ADDRESS Address,UINT64 Value,UINT32 Width){
	if(AcpiOsWritable(Address,Width/4)){
		memcpy(Address,Value,Width/4);
		return AE_OK;
	}
	return AE_ERROR;
}
UINT64 AcpiOsGetTimer (void){
	return pit_system_ticks();
} 
void AcpiOsWaitEventsComplete (void){
	//Waitpid?
	kwarn("NON_IMPL_CALL: AcpiOsWaitEventsComplete()\n");
}
ACPI_STATUS AcpiOsSignal (UINT32 Function,void *Info){
	//TODO
	kwarn("NON_IMPL_CALL: AcpiOsSignal()\n");
	return AE_OK;
}
ACPI_STATUS AcpiOsReadPciConfiguration (ACPI_PCI_ID* PciId,UINT32 Register,UINT64 *Value,UINT32 Width){
	pci_device_t* dev = pci_seek_device3(PciId->Bus,PciId->Device,PciId->Function);
	if(!dev){
		return AE_ERROR;
	}
	*Value = pci_read_value(dev,Register,Width/4);
	return AE_OK;
}
ACPI_STATUS AcpiOsWritePciConfiguration (ACPI_PCI_ID* PciId,UINT32 Register,UINT64 Value,UINT32 Width){
	pci_device_t* dev = pci_seek_device3(PciId->Bus,PciId->Device,PciId->Function);
	if(!dev){
		return AE_ERROR;
	}
	switch(Width){
		case 8:
			pci_write_value8(dev,Register,Value);
		break;
		case 16:
			pci_write_value16(dev,Register,Value);
		break;
		case 32:
			pci_write_value32(dev,Register,Value);
		break;
		default:
			return AE_ERROR;
		break;
	}
	
	return AE_OK;
}
