#include <stdio.h>
#include <malloc.h>
// #include <time.h>
#include "emmc.h"

//
// emmc.c
//
// Provides an interface to the EMMC controller and commands for interacting
// with an sd card
//
// Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
//
// Modified for Circle by R. Stange
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// References:
//
// PLSS - SD Group Physical Layer Simplified Specification ver 3.00
// HCSS - SD Group Host Controller Simplified Specification ver 3.00
//
// Broadcom BCM2835 ARM Peripherals Guide
//
#define RASPPI 3

	//#include <circle/synchronize.h>
	//#include <circle/machineinfo.h>
#define be2le32		bswap32
#define bswap32		__builtin_bswap32
//
// Configuration options
//
#define EMMC_DEBUG
#define EMMC_DEBUG2

//
// According to the BCM2835 ARM Peripherals Guide the EMMC STATUS register
// should not be used for polling. The original driver does not meet this
// specification in this point but the modified driver should do so.
// Define EMMC_POLL_STATUS_REG if you want the original function!
//
//#define EMMC_POLL_STATUS_REG

// Enable 1.8V support
//#define SD_1_8V_SUPPORT

// Enable 4-bit support
#define SD_4BIT_DATA

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         400000
#define SD_CLOCK_NORMAL     25000000
#define SD_CLOCK_HIGH       50000000
#define SD_CLOCK_100        100000000
#define SD_CLOCK_208        208000000

// Enable SDXC maximum performance mode
// Requires 150 mA power so disabled on the RPi for now
#define SDXC_MAXIMUM_PERFORMANCE

// Enable card interrupts
#define SD_CARD_INTERRUPTS

// Allow old sdhci versions (may cause errors)
// Required for QEMU
#define EMMC_ALLOW_OLD_SDHCI

#if RASPPI <= 3
#define EMMC_BASE	ARM_EMMC_BASE
#else
#define EMMC_BASE	ARM_EMMC2_BASE
#endif

#define EMMC_ARG2		(EMMC_BASE + 0x00)
#define EMMC_BLKSIZECNT		(EMMC_BASE + 0x04)
#define EMMC_ARG1		(EMMC_BASE + 0x08)
#define EMMC_CMDTM		(EMMC_BASE + 0x0C)
#define EMMC_RESP0		(EMMC_BASE + 0x10)
#define EMMC_RESP1		(EMMC_BASE + 0x14)
#define EMMC_RESP2		(EMMC_BASE + 0x18)
#define EMMC_RESP3		(EMMC_BASE + 0x1C)
#define EMMC_DATA		(EMMC_BASE + 0x20)
#define EMMC_STATUS		(EMMC_BASE + 0x24)
#define EMMC_CONTROL0		(EMMC_BASE + 0x28)
#define EMMC_CONTROL1		(EMMC_BASE + 0x2C)
#define EMMC_INTERRUPT		(EMMC_BASE + 0x30)
#define EMMC_IRPT_MASK		(EMMC_BASE + 0x34)
#define EMMC_IRPT_EN		(EMMC_BASE + 0x38)
#define EMMC_CONTROL2		(EMMC_BASE + 0x3C)
#define EMMC_CAPABILITIES_0	(EMMC_BASE + 0x40)
#define EMMC_CAPABILITIES_1	(EMMC_BASE + 0x44)
#define EMMC_FORCE_IRPT		(EMMC_BASE + 0x50)
#define EMMC_BOOT_TIMEOUT	(EMMC_BASE + 0x70)
#define EMMC_DBG_SEL		(EMMC_BASE + 0x74)
#define EMMC_EXRDFIFO_CFG	(EMMC_BASE + 0x80)
#define EMMC_EXRDFIFO_EN	(EMMC_BASE + 0x84)
#define EMMC_TUNE_STEP		(EMMC_BASE + 0x88)
#define EMMC_TUNE_STEPS_STD	(EMMC_BASE + 0x8C)
#define EMMC_TUNE_STEPS_DDR	(EMMC_BASE + 0x90)
#define EMMC_SPI_INT_SPT	(EMMC_BASE + 0xF0)
#define EMMC_SLOTISR_VER	(EMMC_BASE + 0xFC)

#define SD_CMD_INDEX(a)			((a) << 24)
#define SD_CMD_TYPE_NORMAL		0
#define SD_CMD_TYPE_SUSPEND		(1 << 22)
#define SD_CMD_TYPE_RESUME		(2 << 22)
#define SD_CMD_TYPE_ABORT		(3 << 22)
#define SD_CMD_TYPE_MASK		(3 << 22)
#define SD_CMD_ISDATA			(1 << 21)
#define SD_CMD_IXCHK_EN			(1 << 20)
#define SD_CMD_CRCCHK_EN		(1 << 19)
#define SD_CMD_RSPNS_TYPE_NONE		0	// For no response
#define SD_CMD_RSPNS_TYPE_136		(1 << 16)	// For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48		(2 << 16)	// For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B		(3 << 16)	// For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK  	(3 << 16)
#define SD_CMD_MULTI_BLOCK		(1 << 5)
#define SD_CMD_DAT_DIR_HC		0
#define SD_CMD_DAT_DIR_CH		(1 << 4)
#define SD_CMD_AUTO_CMD_EN_NONE		0
#define SD_CMD_AUTO_CMD_EN_CMD12	(1 << 2)
#define SD_CMD_AUTO_CMD_EN_CMD23	(2 << 2)
#define SD_CMD_BLKCNT_EN		(1 << 1)
#define SD_CMD_DMA          		1

#ifndef USE_SDHOST

#define SD_ERR_CMD_TIMEOUT	0
#define SD_ERR_CMD_CRC		1
#define SD_ERR_CMD_END_BIT	2
#define SD_ERR_CMD_INDEX	3
#define SD_ERR_DATA_TIMEOUT	4
#define SD_ERR_DATA_CRC		5
#define SD_ERR_DATA_END_BIT	6
#define SD_ERR_CURRENT_LIMIT	7
#define SD_ERR_AUTO_CMD12	8
#define SD_ERR_ADMA		9
#define SD_ERR_TUNING		10
#define SD_ERR_RSVD		11

#define SD_ERR_MASK_CMD_TIMEOUT		(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_CMD_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_CMD_END_BIT		(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CMD_INDEX		(1 << (16 + SD_ERR_CMD_INDEX))
#define SD_ERR_MASK_DATA_TIMEOUT	(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_DATA_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_DATA_END_BIT	(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CURRENT_LIMIT	(1 << (16 + SD_ERR_CMD_CURRENT_LIMIT))
#define SD_ERR_MASK_AUTO_CMD12		(1 << (16 + SD_ERR_CMD_AUTO_CMD12))
#define SD_ERR_MASK_ADMA		(1 << (16 + SD_ERR_CMD_ADMA))
#define SD_ERR_MASK_TUNING		(1 << (16 + SD_ERR_CMD_TUNING))

#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)

#endif

#define SD_RESP_NONE        SD_CMD_RSPNS_TYPE_NONE
#define SD_RESP_R1          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R1b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R2          (SD_CMD_RSPNS_TYPE_136 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R3          SD_CMD_RSPNS_TYPE_48
#define SD_RESP_R4          SD_CMD_RSPNS_TYPE_136
#define SD_RESP_R5          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R5b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R6          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R7          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)

#define SD_DATA_READ        (SD_CMD_ISDATA | SD_CMD_DAT_DIR_CH)
#define SD_DATA_WRITE       (SD_CMD_ISDATA | SD_CMD_DAT_DIR_HC)

#define SD_CMD_RESERVED(a)  0xffffffff

#define SUCCESS          (m_last_cmd_success)
#define FAIL             (m_last_cmd_success == 0)

#ifndef USE_SDHOST

#define TIMEOUT          (FAIL && (m_last_error == 0))
#define CMD_TIMEOUT      (FAIL && (m_last_error & (1 << 16)))
#define CMD_CRC          (FAIL && (m_last_error & (1 << 17)))
#define CMD_END_BIT      (FAIL && (m_last_error & (1 << 18)))
#define CMD_INDEX        (FAIL && (m_last_error & (1 << 19)))
#define DATA_TIMEOUT     (FAIL && (m_last_error & (1 << 20)))
#define DATA_CRC         (FAIL && (m_last_error & (1 << 21)))
#define DATA_END_BIT     (FAIL && (m_last_error & (1 << 22)))
#define CURRENT_LIMIT    (FAIL && (m_last_error & (1 << 23)))
#define ACMD12_ERROR     (FAIL && (m_last_error & (1 << 24)))
#define ADMA_ERROR       (FAIL && (m_last_error & (1 << 25)))
#define TUNING_ERROR     (FAIL && (m_last_error & (1 << 26)))

#else

#define TIMEOUT          (FAIL && (m_last_error == ETIMEDOUT))

#endif

#define SD_VER_UNKNOWN      0
#define SD_VER_1            1
#define SD_VER_1_1          2
#define SD_VER_2            3
#define SD_VER_3            4
#define SD_VER_4            5

// The actual command indices
#define GO_IDLE_STATE           0
#define ALL_SEND_CID            2
#define SEND_RELATIVE_ADDR      3
#define SET_DSR                 4
#define IO_SET_OP_COND          5
#define SWITCH_FUNC             6
#define SELECT_CARD             7
#define DESELECT_CARD           7
#define SELECT_DESELECT_CARD    7
#define SEND_IF_COND            8
#define SEND_CSD                9
#define SEND_CID                10
#define VOLTAGE_SWITCH          11
#define STOP_TRANSMISSION       12
#define SEND_STATUS             13
#define GO_INACTIVE_STATE       15
#define SET_BLOCKLEN            16
#define READ_SINGLE_BLOCK       17
#define READ_MULTIPLE_BLOCK     18
#define SEND_TUNING_BLOCK       19
#define SPEED_CLASS_CONTROL     20
#define SET_BLOCK_COUNT         23
#define WRITE_BLOCK             24
#define WRITE_MULTIPLE_BLOCK    25
#define PROGRAM_CSD             27
#define SET_WRITE_PROT          28
#define CLR_WRITE_PROT          29
#define SEND_WRITE_PROT         30
#define ERASE_WR_BLK_START      32
#define ERASE_WR_BLK_END        33
#define ERASE                   38
#define LOCK_UNLOCK             42
#define APP_CMD                 55
#define GEN_CMD                 56

#define IS_APP_CMD              0x80000000
#define ACMD(a)                 (a | IS_APP_CMD)
#define SET_BUS_WIDTH           (6 | IS_APP_CMD)
#define SD_STATUS               (13 | IS_APP_CMD)
#define SEND_NUM_WR_BLOCKS      (22 | IS_APP_CMD)
#define SET_WR_BLK_ERASE_COUNT  (23 | IS_APP_CMD)
#define SD_SEND_OP_COND         (41 | IS_APP_CMD)
#define SET_CLR_CARD_DETECT     (42 | IS_APP_CMD)
#define SEND_SCR                (51 | IS_APP_CMD)

#ifndef USE_SDHOST

#define SD_RESET_CMD            (1 << 25)
#define SD_RESET_DAT            (1 << 26)
#define SD_RESET_ALL            (1 << 24)

#define SD_GET_CLOCK_DIVIDER_FAIL	0xffffffff

#endif

#define SD_BLOCK_SIZE		512

char *sd_versions[] = {
	"unknown",
	"1.0 or 1.01",
	"1.10",
	"2.00",
	"3.0x",
	"4.xx"
};

#ifndef USE_SDHOST

#ifdef EMMC_DEBUG2
char *err_irpts[] = {
	"CMD_TIMEOUT",
	"CMD_CRC",
	"CMD_END_BIT",
	"CMD_INDEX",
	"DATA_TIMEOUT",
	"DATA_CRC",
	"DATA_END_BIT",
	"CURRENT_LIMIT",
	"AUTO_CMD12",
	"ADMA",
	"TUNING",
	"RSVD"
};
#endif

#endif

const u32 sd_commands[] = {
	SD_CMD_INDEX(0),
	SD_CMD_RESERVED(1),
	SD_CMD_INDEX(2) | SD_RESP_R2,
	SD_CMD_INDEX(3) | SD_RESP_R6,
	SD_CMD_INDEX(4),
	SD_CMD_INDEX(5) | SD_RESP_R4,
	SD_CMD_INDEX(6) | SD_RESP_R1,
	SD_CMD_INDEX(7) | SD_RESP_R1b,
	SD_CMD_INDEX(8) | SD_RESP_R7,
	SD_CMD_INDEX(9) | SD_RESP_R2,
	SD_CMD_INDEX(10) | SD_RESP_R2,
	SD_CMD_INDEX(11) | SD_RESP_R1,
	SD_CMD_INDEX(12) | SD_RESP_R1b | SD_CMD_TYPE_ABORT,
	SD_CMD_INDEX(13) | SD_RESP_R1,
	SD_CMD_RESERVED(14),
	SD_CMD_INDEX(15),
	SD_CMD_INDEX(16) | SD_RESP_R1,
	SD_CMD_INDEX(17) | SD_RESP_R1 | SD_DATA_READ,
	SD_CMD_INDEX(18) | SD_RESP_R1 | SD_DATA_READ | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN | SD_CMD_AUTO_CMD_EN_CMD12,	// SD_CMD_AUTO_CMD_EN_CMD12 not in original driver
	SD_CMD_INDEX(19) | SD_RESP_R1 | SD_DATA_READ,
	SD_CMD_INDEX(20) | SD_RESP_R1b,
	SD_CMD_RESERVED(21),
	SD_CMD_RESERVED(22),
	SD_CMD_INDEX(23) | SD_RESP_R1,
	SD_CMD_INDEX(24) | SD_RESP_R1 | SD_DATA_WRITE,
	SD_CMD_INDEX(25) | SD_RESP_R1 | SD_DATA_WRITE | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN | SD_CMD_AUTO_CMD_EN_CMD12,	// SD_CMD_AUTO_CMD_EN_CMD12 not in original driver
	SD_CMD_RESERVED(26),
	SD_CMD_INDEX(27) | SD_RESP_R1 | SD_DATA_WRITE,
	SD_CMD_INDEX(28) | SD_RESP_R1b,
	SD_CMD_INDEX(29) | SD_RESP_R1b,
	SD_CMD_INDEX(30) | SD_RESP_R1 | SD_DATA_READ,
	SD_CMD_RESERVED(31),
	SD_CMD_INDEX(32) | SD_RESP_R1,
	SD_CMD_INDEX(33) | SD_RESP_R1,
	SD_CMD_RESERVED(34),
	SD_CMD_RESERVED(35),
	SD_CMD_RESERVED(36),
	SD_CMD_RESERVED(37),
	SD_CMD_INDEX(38) | SD_RESP_R1b,
	SD_CMD_RESERVED(39),
	SD_CMD_RESERVED(40),
	SD_CMD_RESERVED(41),
	SD_CMD_RESERVED(42) | SD_RESP_R1,
	SD_CMD_RESERVED(43),
	SD_CMD_RESERVED(44),
	SD_CMD_RESERVED(45),
	SD_CMD_RESERVED(46),
	SD_CMD_RESERVED(47),
	SD_CMD_RESERVED(48),
	SD_CMD_RESERVED(49),
	SD_CMD_RESERVED(50),
	SD_CMD_RESERVED(51),
	SD_CMD_RESERVED(52),
	SD_CMD_RESERVED(53),
	SD_CMD_RESERVED(54),
	SD_CMD_INDEX(55) | SD_RESP_R1,
	SD_CMD_INDEX(56) | SD_RESP_R1 | SD_CMD_ISDATA,
	SD_CMD_RESERVED(57),
	SD_CMD_RESERVED(58),
	SD_CMD_RESERVED(59),
	SD_CMD_RESERVED(60),
	SD_CMD_RESERVED(61),
	SD_CMD_RESERVED(62),
	SD_CMD_RESERVED(63)
};

const u32 sd_acommands[] = {
	SD_CMD_RESERVED(0),
	SD_CMD_RESERVED(1),
	SD_CMD_RESERVED(2),
	SD_CMD_RESERVED(3),
	SD_CMD_RESERVED(4),
	SD_CMD_RESERVED(5),
	SD_CMD_INDEX(6) | SD_RESP_R1,
	SD_CMD_RESERVED(7),
	SD_CMD_RESERVED(8),
	SD_CMD_RESERVED(9),
	SD_CMD_RESERVED(10),
	SD_CMD_RESERVED(11),
	SD_CMD_RESERVED(12),
	SD_CMD_INDEX(13) | SD_RESP_R1,
	SD_CMD_RESERVED(14),
	SD_CMD_RESERVED(15),
	SD_CMD_RESERVED(16),
	SD_CMD_RESERVED(17),
	SD_CMD_RESERVED(18),
	SD_CMD_RESERVED(19),
	SD_CMD_RESERVED(20),
	SD_CMD_RESERVED(21),
	SD_CMD_INDEX(22) | SD_RESP_R1 | SD_DATA_READ,
	SD_CMD_INDEX(23) | SD_RESP_R1,
	SD_CMD_RESERVED(24),
	SD_CMD_RESERVED(25),
	SD_CMD_RESERVED(26),
	SD_CMD_RESERVED(27),
	SD_CMD_RESERVED(28),
	SD_CMD_RESERVED(29),
	SD_CMD_RESERVED(30),
	SD_CMD_RESERVED(31),
	SD_CMD_RESERVED(32),
	SD_CMD_RESERVED(33),
	SD_CMD_RESERVED(34),
	SD_CMD_RESERVED(35),
	SD_CMD_RESERVED(36),
	SD_CMD_RESERVED(37),
	SD_CMD_RESERVED(38),
	SD_CMD_RESERVED(39),
	SD_CMD_RESERVED(40),
	SD_CMD_INDEX(41) | SD_RESP_R3,
	SD_CMD_INDEX(42) | SD_RESP_R1,
	SD_CMD_RESERVED(43),
	SD_CMD_RESERVED(44),
	SD_CMD_RESERVED(45),
	SD_CMD_RESERVED(46),
	SD_CMD_RESERVED(47),
	SD_CMD_RESERVED(48),
	SD_CMD_RESERVED(49),
	SD_CMD_RESERVED(50),
	SD_CMD_INDEX(51) | SD_RESP_R1 | SD_DATA_READ,
	SD_CMD_RESERVED(52),
	SD_CMD_RESERVED(53),
	SD_CMD_RESERVED(54),
	SD_CMD_RESERVED(55),
	SD_CMD_RESERVED(56),
	SD_CMD_RESERVED(57),
	SD_CMD_RESERVED(58),
	SD_CMD_RESERVED(59),
	SD_CMD_RESERVED(60),
	SD_CMD_RESERVED(61),
	SD_CMD_RESERVED(62),
	SD_CMD_RESERVED(63)
};

bool Initialize(void)
{
	m_pSCR = (struct TSCR*)malloc(sizeof(struct TSCR*));
	m_ullOffset = 512000000;
	if (CardInit() != 0) {
		return false;
	}

	/*
	 * here we may need to provide some data struct to store the device. Originally,
	 * the author set a class called devicemanager to manager the device msg.
	 */
	return true;
}

int sd_Read(void *pBuffer, size_t nCount)
{
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	if (m_ullOffset % SD_BLOCK_SIZE != 0) {
		return -1;
	}

	u32 nBlock = m_ullOffset / SD_BLOCK_SIZE;
	if (DoRead((u8 *)pBuffer, nCount, nBlock) != (int) nCount) {
		return -1;
	}

	// printf("nBlock is: %d\n", nBlock);
	// printf("get buf: ");
	// for (int i = 0; i < SD_BLOCK/_SIZE; ++i) printf("%c", ((char *)pBuffer)[i]);
	// printf("\n");

	return nCount;
    /* BLANK END */
    /* LAB 6 TODO END */
	return -1;
}

int sd_Write(const void *pBuffer, size_t nCount)
{
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	if (m_ullOffset % SD_BLOCK_SIZE != 0) {
		return -1;
	}

	u32 nBlock = m_ullOffset / SD_BLOCK_SIZE;
	if (DoWrite((u8 *)pBuffer, nCount, nBlock) != (int) nCount) {
		return -1;
	}

	// printf("nBlock is: %d\n", nBlock);
	// printf("write buf: ");
	// for (int i = 0; i < SD_BLOCK_SIZE; ++i) printf("%c", ((char *)pBuffer)[i]);
	// printf("\n");

	return nCount;
    /* BLANK END */
    /* LAB 6 TODO END */
	return -1;
}

u64 Seek(u64 ullOffset)
{
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	m_ullOffset = ullOffset;

	return m_ullOffset;
    /* BLANK END */
    /* LAB 6 TODO END */
	return -1;
}

int PowerOn(void)
{
	/*
	 * This func is used to restart the SDCard when set the frequency or
	 * voltage. At present, we use the default settings.
	 */
	return 0;
}

void PowerOff(void)
{
	// Power off the SD card
	u32 control0 = read32(EMMC_CONTROL0);
	control0 &= ~(1 << 8);	// Set SD Bus Power bit off in Power Control Register
	write32(EMMC_CONTROL0, control0);
}

// Get the current base clock rate in Hz
u32 GetBaseClock(void)
{
	/*
	 * We use default frequency now.
	 */
	return 0;
}

// Set the clock dividers to generate a target value
u32 GetClockDivider(u32 base_clock, u32 target_rate)
{
	u32 targetted_divisor = 1;
	if (target_rate <= base_clock) {
		targetted_divisor = base_clock / target_rate;
		if (base_clock % target_rate) {
			targetted_divisor--;
		}
	}

	// Decide on the clock mode to use
	// Currently only 10-bit divided clock mode is supported

#ifndef EMMC_ALLOW_OLD_SDHCI
	if (m_hci_ver >= 2) {
#endif
		// HCI version 3 or greater supports 10-bit divided clock mode
		// This requires a power-of-two divider

		// Find the first bit set
		int divisor = -1;
		for (int first_bit = 31; first_bit >= 0; first_bit--) {
			u32 bit_test = (1 << first_bit);
			if (targetted_divisor & bit_test) {
				divisor = first_bit;
				targetted_divisor &= ~bit_test;
				if (targetted_divisor) {
					// The divisor is not a power-of-two, increase it
					divisor++;
				}

				break;
			}
		}

		if (divisor == -1) {
			divisor = 31;
		}
		if (divisor >= 32) {
			divisor = 31;
		}

		if (divisor != 0) {
			divisor = (1 << (divisor - 1));
		}

		if (divisor >= 0x400) {
			divisor = 0x3ff;
		}

		u32 freq_select = divisor & 0xff;
		u32 upper_bits = (divisor >> 8) & 0x3;
		u32 ret = (freq_select << 8) | (upper_bits << 6) | (0 << 5);

		return ret;
#ifndef EMMC_ALLOW_OLD_SDHCI
	} else {
		//LogWrite (LogError, "Unsupported host version");

		return SD_GET_CLOCK_DIVIDER_FAIL;
	}
#endif
}

// Switch the clock rate whilst running
int SwitchClockRate(u32 base_clock, u32 target_rate)
{
	// Decide on an appropriate divider
	u32 divider = GetClockDivider(base_clock, target_rate);
	if (divider == SD_GET_CLOCK_DIVIDER_FAIL) {
		return -1;
	}

	// Wait for the command inhibit (CMD and DAT) bits to clear
	while (read32(EMMC_STATUS) & 3) {
		usDelay(1000);
	}

	// Set the SD clock off
	u32 control1 = read32(EMMC_CONTROL1);
	control1 &= ~(1 << 2);
	write32(EMMC_CONTROL1, control1);
	usDelay(2000);

	// Write the new divider
	control1 &= ~0xffe0;	// Clear old setting + clock generator select
	control1 |= divider;
	write32(EMMC_CONTROL1, control1);
	usDelay(2000);

	// Enable the SD clock
	control1 |= (1 << 2);
	write32(EMMC_CONTROL1, control1);
	usDelay(2000);

	return 0;
}

int ResetCmd(void)
{
	u32 control1 = read32(EMMC_CONTROL1);
	control1 |= SD_RESET_CMD;
	write32(EMMC_CONTROL1, control1);

	if (TimeoutWait(EMMC_CONTROL1, SD_RESET_CMD, 0, 1000000) < 0) {
		return -1;
	}
	return 0;
}

int ResetDat(void)
{
	u32 control1 = read32(EMMC_CONTROL1);
	control1 |= SD_RESET_DAT;
	write32(EMMC_CONTROL1, control1);

	if (TimeoutWait(EMMC_CONTROL1, SD_RESET_DAT, 0, 1000000) < 0) {
		return -1;
	}
	return 0;
}

void IssueCommandInt(u32 cmd_reg, u32 argument, int timeout)
{
	m_last_cmd_reg = cmd_reg;
	m_last_cmd_success = 0;

	// This is as per HCSS 3.7.1.1/3.7.2.2

#ifdef EMMC_POLL_STATUS_REG
	// Check Command Inhibit
	while (read32(EMMC_STATUS) & 1) {
		usDelay(1000);
	}

	// Is the command with busy?
	if ((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) {
		// With busy

		// Is is an abort command?
		if ((cmd_reg & SD_CMD_TYPE_MASK) != SD_CMD_TYPE_ABORT) {
			// Not an abort command

			// Wait for the data line to be free
			while (read32(EMMC_STATUS) & 2) {
				usDelay(1000);
			}
		}
	}
#endif

	// Set block size and block count
	// For now, block size = 512 bytes, block count = 1,
	if (m_blocks_to_transfer > 0xffff) {
		m_last_cmd_success = 0;
		return;
	}
	u32 blksizecnt = m_block_size | (m_blocks_to_transfer << 16);
	write32(EMMC_BLKSIZECNT, blksizecnt);

	// Set argument 1 reg
	write32(EMMC_ARG1, argument);

	// Set command reg
	write32(EMMC_CMDTM, cmd_reg);

	//usDelay (2000);

	// Wait for command complete interrupt
	TimeoutWait(EMMC_INTERRUPT, 0x8001, 1, timeout);
	u32 irpts = read32(EMMC_INTERRUPT);

	// Clear command complete status
	write32(EMMC_INTERRUPT, 0xffff0001);

	// Test for errors
	if ((irpts & 0xffff0001) != 1) {
		m_last_error = irpts & 0xffff0000;
		m_last_interrupt = irpts;

		return;
	}

	//usDelay (2000);

	// Get response data
	switch (cmd_reg & SD_CMD_RSPNS_TYPE_MASK) {
	case SD_CMD_RSPNS_TYPE_48:
	case SD_CMD_RSPNS_TYPE_48B:
		m_last_r0 = read32(EMMC_RESP0);
		break;

	case SD_CMD_RSPNS_TYPE_136:
		m_last_r0 = read32(EMMC_RESP0);
		m_last_r1 = read32(EMMC_RESP1);
		m_last_r2 = read32(EMMC_RESP2);
		m_last_r3 = read32(EMMC_RESP3);
		break;
	}

	// If with data, wait for the appropriate interrupt
	if (cmd_reg & SD_CMD_ISDATA) {
		u32 wr_irpt;
		int is_write = 0;
		if (cmd_reg & SD_CMD_DAT_DIR_CH) {
			wr_irpt = (1 << 5);	// read
		} else {
			is_write = 1;
			wr_irpt = (1 << 4);	// write
		}

		u32 *pData = (u32 *) m_buf;

		for (int nBlock = 0; nBlock < m_blocks_to_transfer; nBlock++) {
			TimeoutWait(EMMC_INTERRUPT, wr_irpt | 0x8000, 1,
				    timeout);
			irpts = read32(EMMC_INTERRUPT);
			write32(EMMC_INTERRUPT, 0xffff0000 | wr_irpt);
			if ((irpts & (0xffff0000 | wr_irpt)) != wr_irpt) {
				m_last_error = irpts & 0xffff0000;
				m_last_interrupt = irpts;

				return;
			}

			// Transfer the block
			size_t length = m_block_size;

			if (is_write) {
				for (; length > 0; length -= 4) {
					write32(EMMC_DATA, *pData++);
				}
			} else {
				for (; length > 0; length -= 4) {
					*pData++ = read32(EMMC_DATA);
				}
			}
		}
	}

	// Wait for transfer complete (set if read/write transfer or with busy)
	if ((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B
	    || (cmd_reg & SD_CMD_ISDATA)) {
#ifdef EMMC_POLL_STATUS_REG
		// First check command inhibit (DAT) is not already 0
		if ((read32(EMMC_STATUS) & 2) == 0) {
			write32(EMMC_INTERRUPT, 0xffff0002);
		} else
#endif
		{
			TimeoutWait(EMMC_INTERRUPT, 0x8002, 1, timeout);
			irpts = read32(EMMC_INTERRUPT);
			write32(EMMC_INTERRUPT, 0xffff0002);

			// Handle the case where both data timeout and transfer complete
			//  are set - transfer complete overrides data timeout: HCSS 2.2.17
			if (((irpts & 0xffff0002) != 2)
			    && ((irpts & 0xffff0002) != 0x100002)) {
				m_last_error = irpts & 0xffff0000;
				m_last_interrupt = irpts;
				return;
			}
			write32(EMMC_INTERRUPT, 0xffff0002);
		}
	}
	// Return success
	m_last_cmd_success = 1;
}

void HandleCardInterrupt(void)
{
	// Handle a card interrupt

	// Get the card status
	if (m_card_rca) {
		IssueCommandInt(sd_commands[SEND_STATUS], m_card_rca << 16,
				500000);
		if (FAIL) {
			;
		} else {
			;
		}
	} else {
		;
	}
}

void HandleInterrupts(void)
{
	u32 irpts = read32(EMMC_INTERRUPT);
	u32 reset_mask = 0;

	if (irpts & SD_COMMAND_COMPLETE) {
		reset_mask |= SD_COMMAND_COMPLETE;
	}

	if (irpts & SD_TRANSFER_COMPLETE) {
		reset_mask |= SD_TRANSFER_COMPLETE;
	}

	if (irpts & SD_BLOCK_GAP_EVENT) {
		reset_mask |= SD_BLOCK_GAP_EVENT;
	}

	if (irpts & SD_DMA_INTERRUPT) {
		reset_mask |= SD_DMA_INTERRUPT;
	}

	if (irpts & SD_BUFFER_WRITE_READY) {
		reset_mask |= SD_BUFFER_WRITE_READY;
		ResetDat();
	}

	if (irpts & SD_BUFFER_READ_READY) {
		reset_mask |= SD_BUFFER_READ_READY;
		ResetDat();
	}

	if (irpts & SD_CARD_INSERTION) {
		reset_mask |= SD_CARD_INSERTION;
	}

	if (irpts & SD_CARD_REMOVAL) {
		reset_mask |= SD_CARD_REMOVAL;
		m_card_removal = 1;
	}

	if (irpts & SD_CARD_INTERRUPT) {
		HandleCardInterrupt();
		reset_mask |= SD_CARD_INTERRUPT;
	}

	if (irpts & 0x8000) {
		reset_mask |= 0xffff0000;
	}

	write32(EMMC_INTERRUPT, reset_mask);
}

bool IssueCommand(u32 command, u32 argument, int timeout)
{
#ifndef USE_SDHOST
	// First, handle any pending interrupts
	HandleInterrupts();
	// Stop the command issue if it was the card remove interrupt that was handled
	if (m_card_removal) {
		m_last_cmd_success = 0;
		return false;
	}
#endif
	// Now run the appropriate commands by calling IssueCommandInt()
	if (command & IS_APP_CMD) {
		command &= 0xff;

		if (sd_acommands[command] == SD_CMD_RESERVED(0)) {
			m_last_cmd_success = 0;

			return false;
		}
		m_last_cmd = APP_CMD;

		u32 rca = 0;
		if (m_card_rca) {
			rca = m_card_rca << 16;
		}
		IssueCommandInt(sd_commands[APP_CMD], rca, timeout);
		if (m_last_cmd_success) {
			m_last_cmd = command | IS_APP_CMD;
			IssueCommandInt(sd_acommands[command], argument,
					timeout);
		}
	} else {

		if (sd_commands[command] == SD_CMD_RESERVED(0)) {
			m_last_cmd_success = 0;

			return false;
		}

		m_last_cmd = command;
		IssueCommandInt(sd_commands[command], argument, timeout);
	}

	return m_last_cmd_success;
}

int CardReset(void)
{
#ifndef USE_SDHOST
	u32 control1 = read32(EMMC_CONTROL1);
	control1 |= (1 << 24);
	// Disable clock
	control1 &= ~(1 << 2);
	control1 &= ~(1 << 0);
	write32(EMMC_CONTROL1, control1);
	if (TimeoutWait(EMMC_CONTROL1, 7 << 24, 0, 1000000) < 0) {
		printf("Controller did not reset properly\n");

		return -1;
	}

#if RASPPI >= 4
	// Enable SD Bus Power VDD1 at 3.3V
	u32 control0 = read32(EMMC_CONTROL0);
	control0 |= 0x0F << 8;
	write32(EMMC_CONTROL0, control0);
	usDelay(2000);
#endif

	// Check for a valid card
	TimeoutWait(EMMC_STATUS, 1 << 16, 1, 500000);
	u32 status_reg = read32(EMMC_STATUS);
	if ((status_reg & (1 << 16)) == 0) {
		printf("no card inserted\n");

		return -1;
	}

	// Clear control2
	write32(EMMC_CONTROL2, 0);
	// Get the base clock rate
	u32 base_clock = GetBaseClock();
	if (base_clock == 0) {
		base_clock = 250000000;
	}

	// Set clock rate to something slow
	control1 = read32(EMMC_CONTROL1);
	control1 |= 1;		// enable clock
	// Set to identification frequency (400 kHz)
	u32 f_id = GetClockDivider(base_clock, SD_CLOCK_ID);
	if (f_id == SD_GET_CLOCK_DIVIDER_FAIL) {
		//printf ("unable to get a valid clock divider for ID frequency\n");

		return -1;
	}
	control1 |= f_id;
	// was not masked out and or'd with (7 << 16) in original driver
	control1 &= ~(0xF << 16);
	control1 |= (11 << 16);	// data timeout = TMCLK * 2^24

	write32(EMMC_CONTROL1, control1);

	if (TimeoutWait(EMMC_CONTROL1, 2, 1, 1000000) < 0) {
		//printf ("Clock did not stabilise within 1 second\n");

		return -1;
	}

	// Enable the SD clock
	usDelay(2000);
	control1 = read32(EMMC_CONTROL1);
	control1 |= 4;
	write32(EMMC_CONTROL1, control1);
	usDelay(2000);

	// Mask off sending interrupts to the ARM
	write32(EMMC_IRPT_EN, 0);
	// Reset interrupts
	write32(EMMC_INTERRUPT, 0xffffffff);
	// Have all interrupts sent to the INTERRUPT register
	u32 irpt_mask = 0xffffffff & (~SD_CARD_INTERRUPT);
#ifdef SD_CARD_INTERRUPTS
	irpt_mask |= SD_CARD_INTERRUPT;
#endif
	write32(EMMC_IRPT_MASK, irpt_mask);
	usDelay(2000);

#else				// #ifndef USE_SDHOST

	// Set clock rate to something slow
	m_Host.SetClock(SD_CLOCK_ID);

#endif				// #ifndef USE_SDHOST

	// >> Prepare the device structure
	m_device_id[0] = 0;
	m_device_id[1] = 0;
	m_device_id[2] = 0;
	m_device_id[3] = 0;

	m_card_supports_sdhc = 0;
	m_card_supports_18v = 0;
	m_card_ocr = 0;
	m_card_rca = 0;
#ifndef USE_SDHOST
	m_last_interrupt = 0;
#endif
	m_last_error = 0;

	m_failed_voltage_switch = 0;

	m_last_cmd_reg = 0;
	m_last_cmd = 0;
	m_last_cmd_success = 0;
	m_last_r0 = 0;
	m_last_r1 = 0;
	m_last_r2 = 0;
	m_last_r3 = 0;

	m_buf = 0;
	m_blocks_to_transfer = 0;
	m_block_size = 0;
#ifndef USE_SDHOST
	m_card_removal = 0;
	m_base_clock = 0;
#endif
	// << Prepare the device structure

#ifndef USE_SDHOST
	m_base_clock = base_clock;
#endif

	// Send CMD0 to the card (reset to idle state)
	if (!IssueCommand(GO_IDLE_STATE, 0, 500000)) {
		return -1;
	}
	// Send CMD8 to the card
	// Voltage supplied = 0x1 = 2.7-3.6V (standard)
	// Check pattern = 10101010b (as per PLSS 4.3.13) = 0xAA
	IssueCommand(SEND_IF_COND, 0x1aa, 500000);
	int v2_later = 0;
	if (TIMEOUT) {
		v2_later = 0;
	}
#ifndef USE_SDHOST
	else if (CMD_TIMEOUT) {
		if (ResetCmd() == -1) {
			return -1;
		}
		write32(EMMC_INTERRUPT, SD_ERR_MASK_CMD_TIMEOUT);
		v2_later = 0;
	}
#endif
	else if (FAIL) {
		//printf ("failure sending CMD8\n");

		return -1;
	} else {
		if ((m_last_r0 & 0xfff) != 0x1aa) {
			//printf ("unusable card\n");

			return -1;
		} else {
			v2_later = 1;
		}
	}

	// Here we are supposed to check the response to CMD5 (HCSS 3.6)
	// It only returns if the card is a SDIO card
	IssueCommand(IO_SET_OP_COND, 0, 10000);
	if (!TIMEOUT) {
#ifndef USE_SDHOST
		if (CMD_TIMEOUT) {
			if (ResetCmd() == -1) {
				return -1;
			}

			write32(EMMC_INTERRUPT, SD_ERR_MASK_CMD_TIMEOUT);
		} else
#endif
		{
			//printf ("SDIO card detected - not currently supported\n");

			return -1;
		}
	}

	// Call an inquiry ACMD41 (voltage window = 0) to get the OCR
	if (!IssueCommand(ACMD(41), 0, 500000)) {
		//printf ("Inquiry ACMD41 failed\n");
		return -1;
	}

	// Call initialization ACMD41
	int card_is_busy = 1;
	while (card_is_busy) {
		u32 v2_flags = 0;
		if (v2_later) {
			// Set SDHC support
			v2_flags |= (1 << 30);

			// Set 1.8v support
#ifdef SD_1_8V_SUPPORT
			if (!m_failed_voltage_switch) {
				v2_flags |= (1 << 24);
			}
#endif
#ifdef SDXC_MAXIMUM_PERFORMANCE
			// Enable SDXC maximum performance
			v2_flags |= (1 << 28);
#endif
		}

		if (!IssueCommand(ACMD(41), 0x00ff8000 | v2_flags, 500000)) {
			//printf ("Error issuing ACMD41\n");

			return -1;
		}

		if ((m_last_r0 >> 31) & 1) {
			// Initialization is complete
			m_card_ocr = (m_last_r0 >> 8) & 0xffff;
			m_card_supports_sdhc = (m_last_r0 >> 30) & 0x1;
#ifdef SD_1_8V_SUPPORT
			if (!m_failed_voltage_switch) {
				m_card_supports_18v = (m_last_r0 >> 24) & 0x1;
			}
#endif

			card_is_busy = 0;
		} else {
			// Card is still busy
			usDelay(500000);
		}
	}

	// At this point, we know the card is definitely an SD card, so will definitely
	//  support SDR12 mode which runs at 25 MHz
#ifndef USE_SDHOST
	SwitchClockRate(base_clock, SD_CLOCK_NORMAL);
#else
	m_Host.SetClock(SD_CLOCK_NORMAL);
#endif

	// A small wait before the voltage switch
	usDelay(5000);

#ifndef USE_SDHOST

	// Switch to 1.8V mode if possible
	if (m_card_supports_18v) {
		// As per HCSS 3.6.1

		// Send VOLTAGE_SWITCH
		if (!IssueCommand(VOLTAGE_SWITCH, 0, 500000)) {
			m_failed_voltage_switch = 1;
			PowerOff();

			return CardReset();
		}

		// Disable SD clock
		control1 = read32(EMMC_CONTROL1);
		control1 &= ~(1 << 2);
		write32(EMMC_CONTROL1, control1);

		// Check DAT[3:0]
		status_reg = read32(EMMC_STATUS);
		u32 dat30 = (status_reg >> 20) & 0xf;
		if (dat30 != 0) {
			m_failed_voltage_switch = 1;
			PowerOff();

			return CardReset();
		}

		// Set 1.8V signal enable to 1
		u32 control0 = read32(EMMC_CONTROL0);
		control0 |= (1 << 8);
		write32(EMMC_CONTROL0, control0);

		// Wait 5 ms
		usDelay(5000);

		// Check the 1.8V signal enable is set
		control0 = read32(EMMC_CONTROL0);
		if (((control0 >> 8) & 1) == 0) {
			m_failed_voltage_switch = 1;
			PowerOff();

			return CardReset();
		}

		// Re-enable the SD clock
		control1 = read32(EMMC_CONTROL1);
		control1 |= (1 << 2);
		write32(EMMC_CONTROL1, control1);

		usDelay(10000);

		// Check DAT[3:0]
		status_reg = read32(EMMC_STATUS);
		dat30 = (status_reg >> 20) & 0xf;
		if (dat30 != 0xf) {
			m_failed_voltage_switch = 1;
			PowerOff();

			return CardReset();
		}

#ifdef EMMC_DEBUG2
		printf ("voltage switch complete\n");
#endif
	}

#endif				// #ifndef USE_SDHOST
	// Send CMD2 to get the cards CID
	if (!IssueCommand(ALL_SEND_CID, 0, 500000)) {
		printf("error sending ALL_SEND_CID\n");

		return -1;
	}
	m_device_id[0] = m_last_r0;
	m_device_id[1] = m_last_r1;
	m_device_id[2] = m_last_r2;
	m_device_id[3] = m_last_r3;

	// Send CMD3 to enter the data state
	if (!IssueCommand(SEND_RELATIVE_ADDR, 0, 500000)) {
		printf("error sending SEND_RELATIVE_ADDR\n");

		return -1;
	}

	u32 cmd3_resp = m_last_r0;
	m_card_rca = (cmd3_resp >> 16) & 0xffff;
	u32 crc_error = (cmd3_resp >> 15) & 0x1;
	u32 illegal_cmd = (cmd3_resp >> 14) & 0x1;
	u32 error = (cmd3_resp >> 13) & 0x1;
	u32 status = (cmd3_resp >> 9) & 0xf;
	u32 ready = (cmd3_resp >> 8) & 0x1;

	if (crc_error) {
		//printf ("CRC error\n");

		return -1;
	}

	if (illegal_cmd) {
		//printf ("illegal command\n");

		return -1;
	}

	if (error) {
		//printf ("generic error\n");

		return -1;
	}

	if (!ready) {
		//printf ("not ready for data\n");

		return -1;
	}

	// Now select the card (toggles it to transfer state)
	if (!IssueCommand(SELECT_CARD, m_card_rca << 16, 500000)) {
		//printf ("error sending CMD7\n");

		return -1;
	}

	u32 cmd7_resp = m_last_r0;
	status = (cmd7_resp >> 9) & 0xf;

	if ((status != 3) && (status != 4)) {
		//printf ("Invalid status (%d)\n", status);

		return -1;
	}

	// If not an SDHC card, ensure BLOCKLEN is 512 bytes
	if (!m_card_supports_sdhc) {
		if (!IssueCommand(SET_BLOCKLEN, SD_BLOCK_SIZE, 500000)) {
			//printf ("Error sending SET_BLOCKLEN\n");

			return -1;
		}
	}

#ifndef USE_SDHOST
	u32 controller_block_size = read32(EMMC_BLKSIZECNT);
	controller_block_size &= (~0xfff);
	controller_block_size |= 0x200;
	write32(EMMC_BLKSIZECNT, controller_block_size);
#endif

	// Get the cards SCR register
	m_buf = &m_pSCR->scr[0];
	m_block_size = 8;
	m_blocks_to_transfer = 1;
	IssueCommand(SEND_SCR, 0, 1000000);
	m_block_size = SD_BLOCK_SIZE;
	if (FAIL) {
		return -2;
	}

	// Determine card version
	// Note that the SCR is big-endian
	u32 scr0 = be2le32(m_pSCR->scr[0]);
	m_pSCR->sd_version = SD_VER_UNKNOWN;
	u32 sd_spec = (scr0 >> (56 - 32)) & 0xf;
	u32 sd_spec3 = (scr0 >> (47 - 32)) & 0x1;
	u32 sd_spec4 = (scr0 >> (42 - 32)) & 0x1;
	m_pSCR->sd_bus_widths = (scr0 >> (48 - 32)) & 0xf;
	if (sd_spec == 0) {
		m_pSCR->sd_version = SD_VER_1;
	} else if (sd_spec == 1) {
		m_pSCR->sd_version = SD_VER_1_1;
	} else if (sd_spec == 2) {
		if (sd_spec3 == 0) {
			m_pSCR->sd_version = SD_VER_2;
		} else if (sd_spec3 == 1) {
			if (sd_spec4 == 0) {
				m_pSCR->sd_version = SD_VER_3;
			} else if (sd_spec4 == 1) {
				m_pSCR->sd_version = SD_VER_4;
			}
		}
	}

	if (m_pSCR->sd_bus_widths & 4) {
		// Set 4-bit transfer mode (ACMD6)
		// See HCSS 3.4 for the algorithm
#ifdef SD_4BIT_DATA

#ifndef USE_SDHOST
		// Disable card interrupt in host
		u32 old_irpt_mask = read32(EMMC_IRPT_MASK);
		u32 new_iprt_mask = old_irpt_mask & ~(1 << 8);
		write32(EMMC_IRPT_MASK, new_iprt_mask);
#endif

		// Send ACMD6 to change the card's bit mode
		if (!IssueCommand(SET_BUS_WIDTH, 2, 500000)) {
			//printf ("Switch to 4-bit data mode failed\n");
		} else {
#ifndef USE_SDHOST
			// Change bit mode for Host
			u32 control0 = read32(EMMC_CONTROL0);
			control0 |= 0x2;
			write32(EMMC_CONTROL0, control0);

			// Re-enable card interrupt in host
			write32(EMMC_IRPT_MASK, old_irpt_mask);
#else
			// Change bit mode for Host
			m_Host.SetBusWidth(4);
#endif
		//printf ("switch to 4-bit complete");
		}
#endif
	}

#ifndef USE_SDHOST
	// Reset interrupt register
	write32(EMMC_INTERRUPT, 0xffffffff);
#endif
	return 0;
}

int CardInit(void)
{
	if (PowerOn() != 0) {

		return -1;
	}

	// Read the controller version
	u32 ver = read32(EMMC_SLOTISR_VER);
	u32 sdversion = (ver >> 16) & 0xff;
	m_hci_ver = sdversion;
	if (m_hci_ver < 2) {
#ifdef EMMC_ALLOW_OLD_SDHCI
		//printf ("Old SDHCI version detected\n");
#else
		//printf (LogError, "Only SDHCI versions >= 3.0 are supported\n");

		return -1;
#endif
	}

	// The SEND_SCR command may fail with a DATA_TIMEOUT on the Raspberry Pi 4
	// for unknown reason. As a workaround the whole card reset is retried.
	int ret;
	for (unsigned nTries = 3; nTries > 0; nTries--) {
		ret = CardReset();
		if (ret != -2) {
			break;
		}

	}
	return ret;
}

// To check whether the card's status is correct
int EnsureDataMode(void)
{
	if (m_card_rca == 0) {
		// Try again to initialise the card
		int ret = CardReset();
		if (ret != 0) {
			return ret;
		}
	}

	if (!IssueCommand(SEND_STATUS, m_card_rca << 16, 500000)) {
		//printf ("EnsureDataMode() error sending CMD13\n");
		m_card_rca = 0;

		return -1;
	}

	u32 status = m_last_r0;
	u32 cur_state = (status >> 9) & 0xf;

	if (cur_state == 3) {
		// Currently in the stand-by state - select it
		if (!IssueCommand(SELECT_CARD, m_card_rca << 16, 500000)) {
			m_card_rca = 0;

			return -1;
		}
	} else if (cur_state == 5) {
		// In the data transfer state - cancel the transmission
		if (!IssueCommand(STOP_TRANSMISSION, 0, 500000)) {
			m_card_rca = 0;

			return -1;
		}

#ifndef USE_SDHOST
		// Reset the data circuit
		ResetDat();
#endif
	} else if (cur_state != 4) {
		// Not in the transfer state - re-initialise
		int ret = CardReset();
		if (ret != 0) {
			return ret;
		}
	}

	// Check again that we're now in the correct mode
	if (cur_state != 4) {
		if (!IssueCommand(SEND_STATUS, m_card_rca << 16, 500000)) {
			m_card_rca = 0;

			return -1;
		}
		status = m_last_r0;
		cur_state = (status >> 9) & 0xf;

		if (cur_state != 4) {
			//printf ("unable to initialise SD card to data mode (state %d)\n", cur_state);
			m_card_rca = 0;

			return -1;
		}
	}

	return 0;
}

int DoDataCommand(int is_write, u8 * buf, size_t buf_size, u32 block_no)
{
	// PLSS table 4.20 - SDSC cards use byte addresses rather than block addresses
	if (!m_card_supports_sdhc) {
		block_no *= SD_BLOCK_SIZE;
	}

	// This is as per HCSS 3.7.2.1
	if (buf_size < m_block_size) {
		//printf ("DoDataCommand() called with buffer size (%d) less than block size (%d)\n", buf_size, m_block_size);

		return -1;
	}

	m_blocks_to_transfer = buf_size / m_block_size;
	if (buf_size % m_block_size) {
		//printf ("DoDataCommand() called with buffer size (%d) not an exact multiple of block size (%d)\n", buf_size, m_block_size);

		return -1;
	}
	m_buf = buf;

	// Decide on the command to use
	int command;

	// LAB6 TODO: judge the type of the command
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	if (is_write) {
		command = (m_blocks_to_transfer > 1) ? WRITE_MULTIPLE_BLOCK : WRITE_BLOCK;
	}
	else {
		command = (m_blocks_to_transfer > 1) ? READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK;
	}
    /* BLANK END */
    /* LAB 6 TODO END */

	int retry_count = 0;
	int max_retries = 3;
	while (retry_count < max_retries) {
		if (IssueCommand(command, block_no, 5000000)) {
			break;
		} else {
			//printf ("error sending CMD%d\n", command);
			//printf ("error = %x\n", m_last_error);

			if (++retry_count < max_retries) {
				//printf ("Retrying\n");
			} else {
				//printf ("Giving up\n");
			}
		}
	}

	if (retry_count == max_retries) {
		m_card_rca = 0;

		return -1;
	}

	return 0;
}

int DoRead(u8 * buf, size_t buf_size, u32 block_no)
{
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	if (EnsureDataMode() != 0) {
		return -1;
	}

	if (DoDataCommand(0, buf, buf_size, block_no) < 0) {
		return -1;
	}

	return buf_size;
    /* BLANK END */
    /* LAB 6 TODO END */
	return -1;
}

int DoWrite(u8 * buf, size_t buf_size, u32 block_no)
{
	/* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */
	if (EnsureDataMode() != 0) {
		return -1;
	}

	if (DoDataCommand(1, buf, buf_size, block_no) < 0) {
		return -1;
	}
	
	return buf_size;
    /* BLANK END */
    /* LAB 6 TODO END */
	return -1;
}

int TimeoutWait(unsigned long reg, unsigned mask, int value, unsigned usec)
{
	unsigned nStartTime = clock();
	int n = 0;
	do {
		if ((read32(reg) & mask) ? value : !value) {
			return 0;
		}
		n++;
	}
	while (clock() - nStartTime < usec);
	printf("n:%d\n", n);
	return -1;
}

void usDelay(unsigned usec)
{
	timer_usDelay(usec);
}

const u32 *GetID(void)
{
	return m_device_id;
}
