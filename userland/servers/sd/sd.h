#pragma once
#include <sys/types.h>
// #include <errno.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
// #include <chcore-internal/sd_defs.h>
#include <stdio.h>
typedef unsigned long uintptr;
#define USE_PHYSICAL_COUNTER
#define RASPPI 3
#define AARCH 64
#if RASPPI == 1
#define ARM_IO_BASE		0x20000000
#elif RASPPI <= 3
#define ARM_IO_BASE		0x3F000000
#else
#define ARM_IO_BASE		0xFE000000
#endif
#define ARM_IO_END		(ARM_IO_BASE + 0xFFFFFF)


#if RASPPI == 1
#ifdef GPU_L2_CACHE_ENABLED
#define GPU_MEM_BASE	GPU_CACHED_BASE
#else
#define GPU_MEM_BASE	GPU_UNCACHED_BASE
#endif
#else
#define GPU_MEM_BASE	GPU_UNCACHED_BASE
#endif

// Convert ARM address to GPU bus address (does also work for aliases)
#define BUS_ADDRESS(addr)	(((addr) & ~0xC0000000) | GPU_MEM_BASE)

//
// External Mass Media Controller (SD Card)(not exist in chcore)
//
#define ARM_EMMC_BASE		(ARM_IO_BASE + 0x300000)

#define PACKED		__attribute__ ((packed))
#define ALIGN(n) __attribute__((__aligned__(n)))
#define NORETURN	__attribute__ ((noreturn))
#define NOOPT		__attribute__ ((optimize (0)))
#define STDOPT		__attribute__ ((optimize (2)))
#define MAXOPT		__attribute__ ((optimize (3)))
#define WEAK		__attribute__ ((weak))

#if RASPPI >= 2

#if RASPPI <= 3
#define ARM_LOCAL_BASE			0x40000000
#else
#define ARM_LOCAL_BASE			0xFF800000
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

#define InvalidateInstructionCache()	asm volatile ("ic iallu" ::: "memory")
#define FlushPrefetchBuffer()		asm volatile ("isb" ::: "memory")

//
// Barriers
//
#define DataSyncBarrier()	asm volatile ("dsb sy" ::: "memory")
#define DataMemBarrier() 	asm volatile ("dmb sy" ::: "memory")

#define InstructionSyncBarrier() asm volatile ("isb" ::: "memory")
#define InstructionMemBarrier()	asm volatile ("isb" ::: "memory")

#define CompilerBarrier()	asm volatile ("" ::: "memory")

//
// Wait for interrupt and event
//
#define WaitForInterrupt()	asm volatile ("wfi")
#define WaitForEvent()		asm volatile ("wfe")
#define SendEvent()		asm volatile ("sev")

#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////////////////////// Raspberry Pi 1 and Zero/////////////////////////////////////////////////////////////////////////
#if RASPPI == 1
// ARM_STRICT_ALIGNMENT activates the memory alignment check. If an// unaligned memory access occurs an exception is raised with this// option defined. This should normally not be activated, because// newer Circle images are not tested with it.//#define ARM_STRICT_ALIGNMENT// GPU_L2_CACHE_ENABLED has to be defined, if the L2 cache of the GPU// is enabled, which is normally the case. Only if you have disabled// the L2 cache of the GPU in config.txt this option must be undefined.
#ifndef NO_GPU_L2_CACHE_ENABLED
#define GPU_L2_CACHE_ENABLED
#endif
// USE_PWM_AUDIO_ON_ZERO can be defined to use GPIO12/13 for PWM audio// output on RPi Zero (W). Some external circuit is needed to use this.// WARNING: Do not feed voltage into these GPIO pins with this option//          defined on a RPi Zero, because this may destroy the pins.//#define USE_PWM_AUDIO_ON_ZERO
#endif
/////////////////////////////////////////////////////////////////////////// Raspberry Pi 2, 3 and 4/////////////////////////////////////////////////////////////////////////
#if RASPPI >= 2
// USE_RPI_STUB_AT enables the debugging support for rpi_stub and// defines the address where rpi_stub is loaded. See doc/debug.txt// for details! Kernel images built with this option defined do// normally not run without rpi_stub loaded.//#define USE_RPI_STUB_AT       0x1F000000
#ifndef USE_RPI_STUB_AT
// ARM_ALLOW_MULTI_CORE has to be defined to use multi-core support// with the class CMultiCoreSupport. It should not be defined for// single core applications, because this may slow down the system// because multiple cores may compete for bus time without use.//#define ARM_ALLOW_MULTI_CORE
#endif
// USE_PHYSICAL_COUNTER enables the use of the CPU internal physical// counter, which is only available on the Raspberry Pi 2, 3 and 4. Reading// this counter is much faster than reading the BCM2835 system timer// counter (which is used without this option). It reduces the I/O load// too. For some QEMU versions this is the only supported timer option,// for other older QEMU versions it does not work. On the Raspberry Pi 4// setting this option is required.
#ifndef NO_PHYSICAL_COUNTER
#define USE_PHYSICAL_COUNTER
#endif
#endif
/////////////////////////////////////////////////////////////////////////// Timing/////////////////////////////////////////////////////////////////////////// REALTIME optimizes the IRQ latency of the system, which could be// useful for time-critical applications. This will be accomplished// by disabling some features (e.g. USB low-/full-speed device support).// See doc/realtime.txt for details!//#define REALTIME// USE_USB_SOF_INTR improves the compatibility with low-/full-speed// USB devices. If your application uses such devices, this option// should normally be set. Unfortunately this causes a heavily changed// system timing, because it triggers up to 8000 IRQs per second. For// USB plug-and-play operation this option must be set in any case.// This option has no influence on the Raspberry Pi 4.
#ifndef NO_USB_SOF_INTR
#define USE_USB_SOF_INTR
#endif
// SCREEN_DMA_BURST_LENGTH enables using DMA for scrolling the screen// contents and set the burst length parameter for the DMA controller.// Using DMA speeds up the scrolling, especially with a burst length// greater than 0. The parameter can be 0-15 theoretically, but values// over 2 are normally not useful, because the system bus gets congested// with it.
#ifndef SCREEN_DMA_BURST_LENGTH
#define SCREEN_DMA_BURST_LENGTH	2
#endif
// CALIBRATE_DELAY activates the calibration of the delay loop. Because// this loop is normally not used any more in Circle, the only use of// this option is that the "SpeedFactor" of your system is displayed.// You can reduce the time needed for booting, if you disable this.
#ifndef NO_CALIBRATE_DELAY
#define CALIBRATE_DELAY
#endif
/////////////////////////////////////////////////////////////////////////// Scheduler/////////////////////////////////////////////////////////////////////////// MAX_TASKS is the maximum number of tasks in the system.
#ifndef MAX_TASKS
#define MAX_TASKS		20
#endif
// TASK_STACK_SIZE is the stack size for each task.
#ifndef TASK_STACK_SIZE
#define TASK_STACK_SIZE		0x8000
#endif
/////////////////////////////////////////////////////////////////////////// USB keyboard/////////////////////////////////////////////////////////////////////////// DEFAULT_KEYMAP selects the default keyboard map (enable only one).// The default keyboard map can be overwritten in with the keymap=// option in cmdline.txt.
#ifndef DEFAULT_KEYMAP
#define DEFAULT_KEYMAP		"DE"
//#define DEFAULT_KEYMAP                "ES"//#define DEFAULT_KEYMAP                "FR"//#define DEFAULT_KEYMAP                "IT"//#define DEFAULT_KEYMAP                "UK"//#define DEFAULT_KEYMAP                "US"
#endif
/////////////////////////////////////////////////////////////////////////// Other/////////////////////////////////////////////////////////////////////////// SCREEN_HEADLESS can be defined, if your Raspberry Pi runs without// a display connected. Most Circle sample programs normally expect// a display connected to work, but some can be used without a display// available. Historically the screen initialization was working even// without a display connected, without returning an error, but// especially on the Raspberry Pi 4 this is not the case any more. Here// it is required to define this option, otherwise the program// initialization will fail without notice. In your own headless// applications you should just not use the CScreenDevice class instead// and direct the logger output to CSerialDevice or CNullDevice.//#define SCREEN_HEADLESS// SERIAL_GPIO_SELECT selects the TXD GPIO pin used for the serial// device (UART0). The RXD pin is (SERIAL_GPIO_SELECT+1). Modifying// this setting can be useful for Compute Modules. Select only one// definition.
#ifndef SERIAL_GPIO_SELECT
#define SERIAL_GPIO_SELECT	14	// and 15
//#define SERIAL_GPIO_SELECT    32      // and 33//#define SERIAL_GPIO_SELECT    36      // and 37
#endif
// USE_SDHOST selects the SDHOST device as interface for SD card// access. Otherwise the EMMC device is used for this purpose. The// SDHOST device is supported by Raspberry Pi 1-3 and Zero, but// not by QEMU. If you rely on a small IRQ latency, USE_SDHOST should// be disabled.
#if RASPPI <= 3 && !defined (REALTIME)
#ifndef NO_SDHOST
//#define USE_SDHOST
#endif
#endif
// SAVE_VFP_REGS_ON_IRQ enables saving the floating point registers// on entry when an IRQ occurs and will restore these registers on exit// from the IRQ handler. This has to be defined, if an IRQ handler// modifies floating point registers. IRQ handling will be a little// slower then.//#define SAVE_VFP_REGS_ON_IRQ// SAVE_VFP_REGS_ON_FIQ enables saving the floating point registers// on entry when an FIQ occurs and will restore these registers on exit// from the FIQ handler. This has to be defined, if the FIQ handler// modifies floating point registers. FIQ handling will be a little// slower then.//#define SAVE_VFP_REGS_ON_FIQ// LEAVE_QEMU_ON_HALT can be defined to exit QEMU when halt() is// called or main() returns EXIT_HALT. QEMU has to be started with the// -semihosting option, so that this works. This option must not be// defined for Circle images which will run on real Raspberry Pi boards.//#define LEAVE_QEMU_ON_HALT// USE_QEMU_USB_FIX fixes an issue when using Circle images inside// QEMU. If you encounter Circle freezing when using USB in QEMU// you should activate this option. It must not be defined for// Circle images which will run on real Raspberry Pi boards.//#define USE_QEMU_USB_FIX/////////////////////////////////////////////////////////////////////////#include <circle/memorymap.h>
#ifdef __cplusplus
extern "C" {
#endif

	static inline u8 read8(uintptr nAddress) {
		return *(u8 volatile *)nAddress;
	} static inline void write8(uintptr nAddress, u8 uchValue) {
		*(u8 volatile *)nAddress = uchValue;
	}

	static inline u16 read16(uintptr nAddress) {
		return *(u16 volatile *)nAddress;
	}

	static inline void write16(uintptr nAddress, u16 usValue) {
		*(u16 volatile *)nAddress = usValue;
	}

	static inline u32 read32(uintptr nAddress) {
		return *(u32 volatile *)nAddress;
	}

	static inline void write32(uintptr nAddress, u32 nValue) {
		*(u32 volatile *)nAddress = nValue;
	}

#ifdef __cplusplus
}
#endif