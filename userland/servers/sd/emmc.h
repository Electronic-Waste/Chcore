//
// emmc.h
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
#pragma once
#include "timer.h"
#include "sd.h"
//#include <arch/aarch64/plat/raspi3/machine.h>

	//struct CGPIOPin         m_GPIO48_53[6];       // SD card

u64 volatile m_ullOffset;
u32 m_hci_ver;
	// was: struct emmc_block_dev
u32 m_device_id[4];

u32 m_card_supports_sdhc;
u32 m_card_supports_18v;
u32 m_card_ocr;
u32 m_card_rca;
u32 m_last_interrupt;
u32 m_last_error;

struct TSCR *m_pSCR;

int m_failed_voltage_switch;

u32 m_last_cmd_reg;
u32 m_last_cmd;
u32 m_last_cmd_success;
u32 m_last_r0;
u32 m_last_r1;
u32 m_last_r2;
u32 m_last_r3;

void *m_buf;
int m_blocks_to_transfer;
size_t m_block_size;
int m_card_removal;
u32 m_base_clock;

struct TSCR			// SD configuration register
{
	u32 scr[2];
	u32 sd_bus_widths;
	int sd_version;
};

bool Initialize(void);

int sd_Read(void *pBuffer, size_t nCount);
int sd_Write(const void *pBuffer, size_t nCount);

u64 Seek(u64 ullOffset);

const u32 *GetID(void);
int PowerOn(void);
void PowerOff(void);

u32 GetBaseClock(void);
	 //u32 GetClockDivider (u32 base_clock, u32 target_rate);
	 //int SwitchClockRate (u32 base_clock, u32 target_rate);

int ResetCmd(void);
int ResetDat(void);

void IssueCommandInt(u32 cmd_reg, u32 argument, int timeout);
void HandleCardInterrupt(void);
void HandleInterrupts(void);

bool IssueCommand(u32 command, u32 argument, int timeout);

int CardReset(void);
int CardInit(void);

int EnsureDataMode(void);
int DoDataCommand(int is_write, u8 * buf, size_t buf_size, u32 block_no);
int DoRead(u8 * buf, size_t buf_size, u32 block_no);
int DoWrite(u8 * buf, size_t buf_size, u32 block_no);

int TimeoutWait(unsigned long reg, unsigned mask, int value, unsigned usec);

void usDelay(unsigned usec);
