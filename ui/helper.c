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

#include "app/css.h"
#include "app/menu.h"
#include "app/radio.h"
#include "ui/gfx.h"
#include "driver/battery.h"
#include "driver/bk4819.h"
#include "driver/serial-flash.h"
#include "driver/st7735s.h"
#include "dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "ui/font.h"
#include "ui/helper.h"
#include "ui/vfo.h"

static const uint8_t FontSmall[47][5] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x10, 0x10, 0x10, 0x10, 0x10 },
	{ 0x00, 0x00, 0x80, 0x00, 0x00 },
	{ 0x40, 0x20, 0x10, 0x80, 0x40 },
	{ 0xFE, 0x82, 0x82, 0x82, 0xFE },
	{ 0x00, 0x00, 0x00, 0x00, 0xFE },
	{ 0xF2, 0x92, 0x92, 0x92, 0x9E },
	{ 0x92, 0x92, 0x92, 0x92, 0xFE },
	{ 0x1E, 0x10, 0x10, 0x10, 0xFE },
	{ 0x9E, 0x92, 0x92, 0x92, 0xF2 },
	{ 0xFE, 0x92, 0x92, 0x92, 0xF2 },
	{ 0x02, 0x02, 0x02, 0x02, 0xFE },
	{ 0xFE, 0x92, 0x92, 0x92, 0xFE },
	{ 0x9E, 0x92, 0x92, 0x92, 0xFE },
	{ 0x00, 0x00, 0x28, 0x00, 0x00 },
	{ 0x80, 0x40, 0x28, 0x00, 0x00 },
	{ 0x10, 0x28, 0x44, 0x82, 0x00 },
	{ 0x28, 0x28, 0x28, 0x28, 0x28 },
	{ 0x00, 0x82, 0x44, 0x28, 0x10 },
	{ 0x04, 0x02, 0xB2, 0x0A, 0x04 },
	{ 0x7C, 0xB2, 0xCA, 0xCA, 0xBC },
	{ 0xF8, 0x24, 0x22, 0x24, 0xF8 },
	{ 0xFE, 0x92, 0x92, 0x92, 0x6C },
	{ 0xFE, 0x82, 0x82, 0x82, 0x82 },
	{ 0xFE, 0x82, 0x82, 0x82, 0x7C },
	{ 0xFE, 0x92, 0x92, 0x92, 0x92 },
	{ 0xFE, 0x12, 0x12, 0x12, 0x12 },
	{ 0xFE, 0x82, 0x92, 0x92, 0xF2 },
	{ 0xFE, 0x10, 0x10, 0x10, 0xFE },
	{ 0x00, 0x82, 0xFE, 0x82, 0x00 },
	{ 0xE0, 0x80, 0x80, 0xFE, 0x00 },
	{ 0xFE, 0x10, 0x10, 0x28, 0xC6 },
	{ 0xFE, 0x80, 0x80, 0x80, 0x80 },
	{ 0xFE, 0x02, 0xFE, 0x02, 0xFE },
	{ 0xFE, 0x02, 0x02, 0x02, 0xFE },
	{ 0x7C, 0x82, 0x82, 0x82, 0x7C },
	{ 0xFE, 0x12, 0x12, 0x12, 0x1E },
	{ 0xFE, 0x82, 0xA2, 0x42, 0xBE },
	{ 0xFE, 0x12, 0x12, 0x32, 0xCC },
	{ 0x8C, 0x92, 0x92, 0x92, 0x62 },
	{ 0x02, 0x02, 0xFE, 0x02, 0x02 },
	{ 0x7E, 0x80, 0x80, 0x80, 0x7E },
	{ 0x3E, 0x40, 0x80, 0x40, 0x3E },
	{ 0x7E, 0x80, 0x7E, 0x80, 0x7E },
	{ 0xC6, 0x28, 0x10, 0x28, 0xC6 },
	{ 0x06, 0x08, 0xF0, 0x08, 0x06 },
	{ 0xC2, 0xA2, 0x92, 0x8A, 0x86 },
};

static const uint16_t FontBigDigits[][10] = {
	{ 0x0FFC, 0x1FFE, 0x3FFF, 0x3103, 0x2081, 0x2041, 0x3023, 0x3FFF, 0x1FFE, 0x0FFC },
	{ 0x0000, 0x0000, 0x0000, 0x0004, 0x0004, 0x3FFE, 0x3FFF, 0x3FFF, 0x0000, 0x0000 },
	{ 0x3804, 0x3C06, 0x3E07, 0x2703, 0x2381, 0x21C1, 0x20E3, 0x207F, 0x203E, 0x203C },
	{ 0x0804, 0x1806, 0x3807, 0x3003, 0x2001, 0x20C1, 0x31E3, 0x3FFF, 0x1F3E, 0x0E1C },
	{ 0x0380, 0x03C0, 0x0260, 0x0230, 0x0218, 0x020C, 0x3FFE, 0x3FFF, 0x3FFF, 0x0200 },
	{ 0x08FF, 0x18FF, 0x38FF, 0x3061, 0x2021, 0x2021, 0x3061, 0x3FE1, 0x1FC1, 0x0F81 },
	{ 0x0FFC, 0x1FFE, 0x3FFF, 0x30C3, 0x2061, 0x2061, 0x30E3, 0x3FC7, 0x1F86, 0x0F04 },
	{ 0x0001, 0x0001, 0x0001, 0x3FC1, 0x3FE1, 0x3FF1, 0x0039, 0x001F, 0x000F, 0x0007 },
	{ 0x0E1C, 0x1F3E, 0x3FFF, 0x31E3, 0x20C1, 0x20C1, 0x31E3, 0x3FFF, 0x1F3E, 0x0E1C },
	{ 0x083C, 0x187E, 0x38FF, 0x31C3, 0x2181, 0x2181, 0x30C3, 0x3FFF, 0x1FFE, 0x0FFC },
	{ 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0, 0x00C0 },
};

static const uint16_t Icons[] = {
	// 0x00 Lock
	0x03F0, 0x03FE, 0x03F1, 0x03F1, 0x03F1, 0x03F1, 0x03FE, 0x03F0,
	// 0x08 Bell
	0x0080, 0x00C0, 0x00FC, 0x01FE, 0x02FF, 0x02FF, 0x01FE, 0x00FC,
	// 0x10 ???
	0x00C0, 0x0080, 0x0040, 0x0080, 0x01FC, 0x0282, 0x0241, 0x0201, 0x0201, 0x0201, 0x0209, 0x0104, 0x00FE, 0x0004, 0x0008,
	// 0x1F Dual Watch
	0x03FF, 0x0201, 0x0201, 0x0201, 0x0201, 0x0102, 0x00FC, 0x0000, 0x00FF, 0x0100, 0x0200, 0x0100, 0x00FF, 0x0100, 0x0200, 0x0100, 0x00FF,
	// 0x30 Battery
	0x00FC, 0x00FC, 0x03FF, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x03FF,
	// 0x43 Vox
	0x007F, 0x0080, 0x0100, 0x0200, 0x0100, 0x0080, 0x007F, 0x0000, 0x01FE, 0x0201, 0x0201, 0x0201, 0x0201, 0x0201, 0x01FE, 0x0000, 0x0303, 0x0084, 0x0048, 0x0030, 0x0030, 0x0048, 0x0084, 0x0303,
	// 0x5B NOAA
	0x0038, 0x037C, 0x007C, 0x037E, 0x007F, 0x037F, 0x007F, 0x037E, 0x007C, 0x037C, 0x0038,
	// 0x66 TR
	0x0001, 0x0001, 0x0001, 0x03FF, 0x0001, 0x0001, 0x0001, 0x0000, 0x03FF, 0x0021, 0x0021, 0x0061, 0x00A1, 0x0112, 0x020C,
	// 0x75 RR
	0x03FF, 0x0021, 0x0021, 0x0061, 0x00A1, 0x0112, 0x020C, 0x0000, 0x03FF, 0x0021, 0x0021, 0x0061, 0x00A1, 0x0112, 0x020C,
};

static const uint16_t IconAntenna[][10] = {
	{ 0x0244, 0x0244, 0x0C84, 0x0088, 0x0308, 0x0C10, 0x0010, 0x0060, 0x0180, 0x0E00 },
	{ 0x001C, 0x0060, 0x0180, 0x0200, 0x020C, 0x0430, 0x0440, 0x084C, 0x0890, 0x0890 },
};

static const uint32_t IconRadio[] = {
	0x7FFC00, 0x800300, 0xAA7B00, 0xAA7A00, 0xAA7A00, 0xAA7A00, 0xAA7A00, 0x8003FF, 0x7FFC00,
};

static const uint8_t BitmapFM[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x08, 0x10, 0x1F, 0x20,
	0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7F, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x20, 0x20,
	0x20, 0x1F, 0x10, 0x08, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x06, 0x0E, 0x22, 0x42, 0x82, 0x02, 0x02, 0x02, 0x02, 0x02, 0x06, 0xFF, 0x06,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFF, 0xFF, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x06, 0xFF, 0x06, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x42, 0x22, 0x1E, 0x06, 0x01, 0x00, 0x00,
	0x00, 0x3F, 0xC1, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xC1, 0x3F, 0x00,
	0x00, 0xF8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF8, 0x00,
	0x00, 0x00, 0x00, 0xE0, 0x70, 0x4C, 0x46, 0x41, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0xFF, 0x40,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0xFF, 0xFF, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
	0x40, 0xFF, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x41, 0x42, 0x4C, 0x70, 0xE0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x60, 0x20, 0x10, 0x10, 0xF8, 0x08,
	0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0xFE, 0xFE, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04,
	0x08, 0xF8, 0x10, 0x10, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t BitmapSKY[] = {
	0x00, 0x01, 0x07, 0x0F, 0x1F, 0x3F, 0x3F, 0x3F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
	0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3F, 0x3F, 0x3F, 0x1F, 0x0F, 0x07, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0,
	0xF0, 0xF0, 0xF8, 0xF8, 0xF8, 0xFC, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xF8,
	0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0,
	0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFE, 0xFE, 0xFC, 0xF8, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF1,
	0xF1, 0xE3, 0x87, 0x0F, 0x1F, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x07, 0x00, 0x20, 0x38,
	0x3C, 0x78, 0x70, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x03, 0x07, 0x01, 0x00, 0xE0, 0xFE, 0xFF, 0x7F, 0x7F,
	0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x03, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
	0x40, 0x00, 0x00, 0x00, 0xC0, 0xE3, 0xE1, 0xF1, 0xF8, 0xF8, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
	0xF8, 0xF8, 0xF8, 0xF1, 0xE1, 0xE1, 0xC0, 0x00, 0x00, 0x00, 0x40, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF0, 0xE0, 0xC0, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF8, 0xFC, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint16_t BitmapMAIN[] = {
	0x00FC, 0x01FE, 0x0303, 0x03F7, 0x030F, 0x03F7, 0x0303, 0x03FF,
	0x030F, 0x03D7, 0x03DB, 0x03D7, 0x030F, 0x03FF, 0x037B, 0x0303,
	0x037B, 0x03FF, 0x0303, 0x03F7, 0x03EF, 0x03DF, 0x0303, 0x01FE,
	0x00FC,
};

static const uint8_t BitmapRadar[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03,
	0x07, 0x07, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x7F, 0x78, 0x78, 0x78, 0x78,
	0x78, 0x78, 0x78, 0x78, 0x7C, 0x3C, 0x3C, 0x3C, 0x3E, 0x1E, 0x1E, 0x1F, 0x0F, 0x0F, 0x07, 0x07,
	0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x3F, 0x7E, 0xFC, 0xF8, 0xF0, 0xE3,
	0xF2, 0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0,
	0xE0, 0xF0, 0xF8, 0xFC, 0x7E, 0x3F, 0x1F, 0x0F, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x07, 0x1F, 0x7F, 0xFF, 0xFC, 0xF0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x06, 0x0C, 0x18, 0xA0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x03, 0x01, 0xC1, 0x00, 0x80, 0x80, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x07, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xF0, 0xFC, 0xFF, 0x7F, 0x1F, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x7E,
	0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1C, 0x60, 0xC0, 0x80, 0xC0, 0xE0, 0xF0, 0x0C, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x60, 0x1F, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, 0xC0,
	0x7F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00,
	0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFC,
	0x0F, 0x0D, 0x00, 0x00, 0x04, 0x04, 0x80, 0x70, 0x1C, 0x06, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x01, 0x03, 0x02, 0x0E, 0x18, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0F,
	0xFC, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xC0, 0xF8, 0xFC, 0xFF, 0x7F, 0x1F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x06, 0x03, 0x03, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x03, 0x03, 0x06, 0x04, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x1F, 0xFF, 0xFF, 0xFC, 0xF0, 0xC0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0x7E, 0x3F, 0x1F, 0x0F,
	0x0F, 0x07, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x08, 0x18, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07,
	0x0F, 0x1F, 0x3F, 0x7E, 0xFC, 0xF8, 0xF0, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	0xC0, 0xC0, 0xE0, 0xE0, 0xF0, 0xF0, 0xF8, 0xF8, 0x78, 0x78, 0x7C, 0x7C, 0x7C, 0x3C, 0x3C, 0x3C,
	0x3C, 0x3C, 0x3C, 0x7C, 0x7C, 0x7C, 0x78, 0x78, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0,
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void UI_DrawString(uint8_t X, uint8_t Y, const char *String, uint8_t Size)
{
	FONT_Draw(X, Y, SF_AnotherOffsetTable, FONT_GetOffsets(String, Size, true));
}

void UI_DrawSmallCharacter(uint8_t X, uint8_t Y, char Digit)
{
	uint8_t Base;
	uint8_t i;

	Base = 0;
	if (Digit >= '-' && Digit <= 'Z') {
		Base = (Digit - '-') + 1;
	}
	for (i = 0; i < 5; i++) {
		uint8_t Pixel = FontSmall[Base][i];
		uint8_t j;

		ST7735S_SetPosition(X + i, Y);
		for (j = 0; j < 8; j++) {
			if (Pixel & 0x80U) {
				ST7735S_SendU16(gColorForeground);
			} else {
				ST7735S_SendU16(gColorBackground);
			}
			Pixel <<= 1;
		}
	}
}

void UI_DrawDigits(const char *pDigits, uint8_t Vfo)
{
	uint8_t X;
	uint8_t Y;
	uint8_t i;

	X = 136;
	Y = 73 + (Vfo * 215);
	gColorForeground = COLOR_WHITE;
	for (i = 0; i < 3; i++) {
		if (pDigits[i] == '-') {
			UI_DrawSmallCharacter(X, Y, '-');
		} else {
			UI_DrawSmallCharacter(X, Y, pDigits[i] + '0');
		}
		X += 6;
	}
}

void UI_DrawSmallString(uint8_t X, uint8_t Y, const char *String, uint8_t Size)
{
	uint8_t i;

	for (i = 0; i < Size; i++) {
		UI_DrawSmallCharacter(X, Y, String[i]);
		X += 6;
	}
}

void UI_DrawStatusIcon(uint8_t X, UI_Icon_t Icon, bool bDraw, uint16_t Color)
{
	const uint8_t Size = (Icon >> 0) & 0xFFU;
	const uint8_t Index = (Icon >> 8) & 0xFFU;
	uint8_t i, j;

	for (i = 0; i < Size; i++) {
		uint16_t Pixel = Icons[Index + i];

		ST7735S_SetPosition(X + i, 85);
		for (j = 0; j < 10; j++) {
			if (bDraw) {
				if (Pixel & 0x0200U) {
					ST7735S_SendU16(Color);
				} else {
					ST7735S_SendU16(gColorBackground);
				}
				Pixel <<= 1;
			} else {
				ST7735S_SendU16(gColorBackground);
			}
		}
	}
}

void UI_DrawRoger(void)
{
	gColorForeground = COLOR_WHITE;

	switch (gSettings.RogerBeep) {
	case 0:
		UI_DrawStatusIcon(20, ICON_BELL, false, COLOR_WHITE);
		UI_DrawSmallString(32, 85, "  ", 2);
		break;
	case 1:
		UI_DrawStatusIcon(20, ICON_BELL, true, COLOR_WHITE);
		UI_DrawSmallString(32, 86, " 1", 2);
		break;
	case 2:
		UI_DrawStatusIcon(20, ICON_BELL, true, COLOR_WHITE);
		UI_DrawSmallString(32, 86, " 2", 2);
		break;
	case 3:
		UI_DrawStatusIcon(20, ICON_BELL, true, COLOR_WHITE);
		UI_DrawSmallString(32, 86, "ID", 2);
		break;
	}
}

void UI_DrawVoltage(uint8_t Vfo)
{
	if (gSettings.DualDisplay == 0) {
		const uint8_t Y = 72 - (Vfo * 41);
		gColorForeground = COLOR_BLUE;
		Int2Ascii(gBatteryVoltage, 2);
		gShortString[2] = gShortString[1];
		gShortString[1] = '.';
		gShortString[3] = 'V';
		UI_DrawString(16, Y, "Voltage :", 9);
		UI_DrawString(96, Y, gShortString, 4);
	}
}

void UI_DrawVfoFrame(uint8_t Y)
{
	Y = 43 - (Y * 41);
	DISPLAY_DrawRectangle0( 20, Y, 100, 1, COLOR_WHITE);
	DISPLAY_DrawRectangle1( 20, Y,   6, 1, COLOR_WHITE);
	DISPLAY_DrawRectangle1(120, Y,   6, 1, COLOR_WHITE);
	DISPLAY_DrawRectangle1( 45, Y,   5, 1, COLOR_WHITE);
	DISPLAY_DrawRectangle1( 70, Y,   5, 1, COLOR_WHITE);
	DISPLAY_DrawRectangle1( 95, Y,   5, 1, COLOR_WHITE);
}

void UI_DrawName(uint8_t Vfo, const char *pName)
{
	gColorForeground = COLOR_GREY;
	UI_DrawString(34, 81 - (Vfo * 41), pName, 10);
}

void UI_DrawExtra(uint8_t Mode, bool bIsAM, uint8_t Vfo)
{
	const uint8_t Y = 43 - (Vfo * 41);

	switch (Mode) {
	case 0:
		gColorForeground = COLOR_BLUE;
		UI_DrawSmallString(4, Y, bIsAM ? "AM" : "FM", 2);
		break;

	case 1: // TX mode
		gColorForeground = COLOR_RED;
		UI_DrawSmallString(4, Y, "VO", 2);
		break;

	case 2: // RX mode
		gColorForeground = COLOR_BLUE;
		UI_DrawSmallString(4, Y, "SQ", 2);
		break;
	}
}

void UI_DrawFrequency(uint32_t Frequency, uint8_t Vfo, uint16_t Color)
{
	uint8_t X = 20;
	uint8_t Y = 50 - (Vfo * 41);
	uint32_t Divider = 10000000U;
	uint8_t i;

	gColorForeground = Color;
	DISPLAY_Fill(56, 57, Y, Y + 1, Color);

	for (i = 0; i < 8; i++) {
		UI_DrawBigDigit(X, Y, (Frequency / Divider) % 10U);
		Divider /= 10;
		if (i == 2) {
			X += 16;
		} else {
			X += 12;
		}
	}
}

void UI_DrawBigDigit(uint8_t X, uint8_t Y, uint8_t Digit)
{
	uint8_t i;
	uint8_t j;

	for (i = 0; i < 10; i++) {
		uint16_t Pixel = FontBigDigits[Digit][i];

		ST7735S_SetPosition(X + i, Y);
		for (j = 0; j < 14; j++) {
			if (Pixel & 0x2000U) {
				ST7735S_SendU16(gColorForeground);
			} else {
				ST7735S_SendU16(gColorBackground);
			}
			Pixel <<= 1;
		}
	}
}

void UI_DrawCss(uint8_t CodeType, uint16_t Code, uint8_t Encrypt, bool bMute, uint8_t Vfo)
{
	const uint8_t Y = 58 - (Vfo * 41);

	gColorForeground = COLOR_WHITE;

	if (bMute) {
		UI_DrawSmallString(124, Y, "MUTE ", 5);
		return;
	}
	if (Encrypt) {
		UI_DrawSmallString(124, Y, "ENC  ", 5);
		return;
	}
	switch (CodeType) {
	case 0:
		Int2Ascii(Code, 4);
		gShortString[4] = gShortString[3];
		gShortString[3] = '.';
		UI_DrawSmallString(124, Y, gShortString, 5);
		break;
	case 1:
	case 2:
		UI_DrawSmallString(124, Y, (CodeType == CODE_TYPE_DCS_N) ? "D   N" : "D   I", 5);
		Int2Ascii(CSS_ConvertCode(Code), 3);
		UI_DrawSmallString(130, Y, gShortString, 3);
		break;
	case 3:
		UI_DrawSmallString(124, Y, "OFF  ", 5);
		break;
	}
}

void UI_DrawTxPower(bool bIsLow, uint8_t Vfo)
{
	uint8_t Y = 43 - (Vfo * 41);

	if (bIsLow) {
		gColorForeground = COLOR_RED;
		UI_DrawSmallString(124, Y, "L", 1);
	} else {
		gColorForeground = COLOR_GREEN;
		UI_DrawSmallString(124, Y, "H", 1);
	}
}

void UI_DrawChannel(uint16_t Channel, uint8_t Vfo)
{
	uint8_t Y = 73 - (Vfo * 41);

	gColorForeground = COLOR_WHITE;
	if (Channel > 998) {
		UI_DrawSmallString(124, Y, "VFO  ", 5);
	} else {
		UI_DrawSmallString(124, Y, "CH", 2);
		Int2Ascii(Channel + 1, 3);
		UI_DrawSmallString(136, Y, gShortString, 3);
	}
}

static void DrawRadio(uint8_t Vfo)
{
	uint8_t i;

	for (i = 0; i < 9; i++) {
		uint32_t Pixel = IconRadio[i];
		uint8_t j;

		ST7735S_SetPosition(4 + i, 56 - (Vfo * 41));
		for (j = 0; j < 24; j++) {
			if (Pixel & 0x800000U) {
				ST7735S_SendU16(gColorForeground);
			} else {
				ST7735S_SendU16(gColorBackground);
			}
			Pixel <<= 1;
		}
	}
}

void UI_DrawRX(uint8_t Vfo)
{
	uint8_t i;
	uint8_t j;

	for (i = 0; i < 10; i++) {
		uint16_t Pixel;

		if (gRadioMode == RADIO_MODE_TX) {
			gColorForeground = COLOR_RED;
			Pixel = IconAntenna[0][i];
		} else if (gRadioMode == RADIO_MODE_RX) {
			gColorForeground = COLOR_BLUE;
			Pixel = IconAntenna[1][i];
		} else {
			gColorForeground = gColorBackground;
			Pixel = 0x0FFF;
		}
		ST7735S_SetPosition(14 + i, 70 - (Vfo * 41));
		for (j = 0; j < 12; j++) {
			if (Pixel & 0x800U) {
				ST7735S_SendU16(gColorForeground);
			} else {
				ST7735S_SendU16(gColorBackground);
			}
			Pixel <<= 1;
		}
	}
	DrawRadio(Vfo);
}

void UI_DrawDTMF(void)
{
	gColorForeground = COLOR_RED;
	UI_DrawString(10, 54, "DTMF Input :", 12);
	UI_DrawString(10, 38, gDTMF_Input.String, 14);
}

void UI_DrawFMFrequency(uint16_t Frequency)
{
	Int2Ascii(gSettings.FmFrequency, 4);
	gShortString[4] = gShortString[3];
	gShortString[3] = '.';
	gColorForeground = COLOR_BLUE;
	UI_DrawString(84, 58, gShortString, 5);
	UI_DrawString(124, 58, "M", 1);
}

void UI_DrawFM(void)
{
	gColorForeground = COLOR_GREY;
	DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
	DISPLAY_DrawRectangle0(0, 81, 160, 1, gSettings.BorderColor);
	UI_DrawFrame(12, 150, 6, 74, 2, gColorForeground);
	UI_DrawFrame(72, 144, 36, 64, 2, gColorForeground);
	DISPLAY_Fill( 72,  88, 16, 22, gColorForeground);
	DISPLAY_Fill(100, 116, 16, 22, gColorForeground);
	DISPLAY_Fill(128, 144, 16, 22, gColorForeground);
	UI_DrawBitmap(16,  16,  6, 48, BitmapFM);
}

void UI_DrawBitmap(uint8_t X, uint8_t Y, uint8_t H, uint8_t W, const uint8_t *pBitmap)
{
	uint8_t x, y, i;

	for (y = 0; y < H; y++) {
		for (x = 0; x < W; x++) {
			uint8_t Pixel = pBitmap[x + (y * W)];

			for (i = 0; i < 8; i++) {
				if (Pixel & 0x80U) {
					ST7735S_SetPixel(X + x, Y + i, gColorForeground);
				} else {
					ST7735S_SetPixel(X + x, Y + i, gColorBackground);
				}
				Pixel <<= 1;
			}
		}
		Y += 8;
	}
}

void UI_DrawFrame(uint8_t X0, uint8_t X1, uint8_t Y0, uint8_t Y1, uint8_t Thickness, uint16_t Color)
{
	DISPLAY_Fill(X0, X0 + Thickness - 1, Y0, Y1, Color);
	DISPLAY_Fill(X1 - Thickness + 1, X1, Y0, Y1, Color);
	DISPLAY_Fill(X0, X1, Y0, Y0 + Thickness - 1, Color);
	DISPLAY_Fill(X0, X1, Y1 - Thickness + 1, Y1, Color);
}

void UI_DrawDialog(void)
{
	DISPLAY_Fill(4, 156, 19, 61, COLOR_BLACK);
	UI_DrawFrame(4, 156, 19, 61, 2, gSettings.BorderColor);
}

void UI_DrawVox(void)
{
	if (gVoxRssiUpdateTimer == 0) {
		uint16_t Vox;

		gVoxRssiUpdateTimer = 200;
		Vox = BK4819_ReadRegister(0x64);
		if (Vox > 5000) {
			Vox = 5000;
		}
		UI_DrawBar(Vox / 50, gSettings.CurrentVfo);
	}
}

void UI_DrawBar(uint8_t Level, uint8_t Vfo)
{
	uint8_t Y = 44 - (Vfo * 41);
	uint8_t i;

	if (Level < 25) {
		gColorForeground = COLOR_RED;
	} else if (Level < 50) {
		gColorForeground = COLOR_RGB(31, 41, 0);
	} else {
		gColorForeground = COLOR_GREEN;
	}

	for (i = 0; i < Level; i++) {
		if (i != 0 && i != 25 && i != 50 && i != 75) {
			DISPLAY_DrawRectangle1(20 + i, Y, 4, 1, gColorForeground);
		}
	}

	for (; Level < 100; Level++) {
		if (Level != 0 && Level != 25 && Level != 50 && Level != 75) {
			DISPLAY_DrawRectangle1(20 + Level, Y, 4, 1, gColorBackground);
		}
	}
}

void UI_DrawSomething(void)
{
	const uint8_t Y = gCurrentVfo * 41;

	if (gSettings.DualDisplay == 0 && gSettings.CurrentVfo != gCurrentVfo) {
		DISPLAY_Fill(1, 158, 1 + Y, 40 + Y, COLOR_BLACK);
		DISPLAY_Fill(1, 158, 1 + ((!gCurrentVfo) * 41), 40 + ((!gCurrentVfo) * 41), COLOR_BLACK);
		UI_DrawVoltage(!gSettings.CurrentVfo);
		UI_DrawVfo(gSettings.CurrentVfo);
	} else {
		UI_DrawVfo(gCurrentVfo);
		if (gSettings.CurrentVfo == gCurrentVfo && gInputBoxWriteIndex) {
			if (gSettings.WorkMode) {
				UI_DrawDigits(gInputBox, gSettings.CurrentVfo);
			} else {
				UI_DrawFrequencyEx(gInputBox, gSettings.CurrentVfo, gFrequencyReverse);
			}
		}
		UI_DrawRX(gCurrentVfo);
		UI_DrawBar(0, gCurrentVfo);
	}
	UI_DrawMainBitmap(1, gSettings.CurrentVfo);
}

void UI_DrawMainBitmap(bool bOverride, uint8_t Vfo)
{
	uint8_t i, j;

	if (gSettings.bFLock) {
		gColorForeground = COLOR_RED;
	} else {
		gColorForeground = COLOR_BLUE;
	}

	for (i = 0; i < 25; i++) {
		uint16_t Pixel = BitmapMAIN[i];

		ST7735S_SetPosition(i + 4, 70 - (Vfo * 41));
		for (j = 0; j < 10; j++) {
			if (bOverride) {
				if (Pixel & 0x0200U) {
					ST7735S_SendU16(gColorForeground);
				} else {
					ST7735S_SendU16(gColorBackground);
				}
				Pixel <<= 1;
			} else {
				ST7735S_SendU16(gColorBackground);
			}
		}
	}
}

void UI_DrawSky(void)
{
	gColorForeground = COLOR_RGB(31, 63, 31);
	DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
	DISPLAY_DrawRectangle0(0, 81, 160, 1, gSettings.BorderColor);
	UI_DrawBitmap(90, 16, 7, 70, BitmapSKY);
}

void UI_DrawFrequencyEx(const char *String, uint8_t Vfo, bool bReverse)
{
	uint8_t X = 20;
	const uint8_t Y = 50 - (Vfo * 41);
	uint8_t i;

	if (!bReverse) {
		gColorForeground = COLOR_WHITE;
	} else {
		gColorForeground = COLOR_RED;
	}

	DISPLAY_Fill(56, 57, Y, Y + 1, gColorForeground);
	for (i = 0; i < 8; i++) {
		UI_DrawBigDigit(X, Y, String[i]);
		if (i == 2) {
			X += 16;
		} else {
			X += 12;
		}
	}
}

void UI_DrawBootVoltage(uint8_t X, uint8_t Y)
{
	gColorForeground = COLOR_RED;
	Int2Ascii(gBatteryVoltage, 2);
	gShortString[2] = gShortString[1];
	gShortString[1] = '.';
	gShortString[3] = 'V';
	UI_DrawString(X, Y, "Voltage:", 8);
	UI_DrawString(X + 72, Y, gShortString, 4);
}

void UI_DrawDecimal(const char *pInput)
{
	char String[5];
	uint8_t i;

	gColorForeground = COLOR_BLUE;
	for (i = 0; i < 4; i++) {
		String[i] = (pInput[i] == 10) ? '-' : ('0' + pInput[i]);
	}
	String[4] = String[3];
	String[3] = '.';
	UI_DrawString(84, 58, String, 5);
}

void UI_DrawMenuPosition(const char *pString)
{
	uint8_t i;

	for (i = 0; i < 2; i++) {
		if (pString[i] == '-') {
			gString[i] = '-';
		} else {
			gString[i] = '0' + pString[i];
		}
	}
	UI_DrawString(136, 76, gString, 2);
}

void UI_DrawStringMenuSettings(void)
{
	UI_DrawString(24, 76, "Menu Settings   ", 16);
}

void UI_DrawStringSwitchType(void)
{
	DISPLAY_DrawRectangle0(1, 20, 159, 1, COLOR_RGB(31, 53, 0));
	gColorForeground = COLOR_RED;
	UI_DrawString(4, 18, "Switch type by [*]", 18);
	gColorForeground = COLOR_WHITE;
}

void UI_DrawRadar(void)
{
	DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
	DISPLAY_DrawRectangle0(0, 81, 160, 1, gSettings.BorderColor);
	gColorForeground = COLOR_BLUE;
	UI_DrawBitmap(4, 12, 8, 64, BitmapRadar);
}

void UI_DrawGolay(void)
{
	uint8_t i;

	gSettingGolay = 0;
	for (i = 0; i < 8; i++) {
		gSettingGolay = (gSettingGolay * 10) + (gInputBox[i] - '0');
	}
	if (gSettingGolay > 0xFFFFFF) {
		gSettingGolay &= 0xFFFFFF;
		Int2Ascii(gSettingGolay, 8);
		UI_DrawString(24, 24, gShortString, 8);
	}
}

void UI_DrawChannelNumber(const char *pString)
{
	uint8_t i;

	for (i = 0; i < 3; i++) {
		if (pString[i] == '-') {
			gString[i] = '-';
		} else {
			gString[i] = '0' + pString[i];
		}
	}
	UI_DrawString(128, 76, gString, 3);
}

void UI_DrawBand(void)
{
	gColorForeground = COLOR_RED;
	if (gSettings.bUseVHF) {
		UI_DrawString(80, 72, "VHF      ", 9);
	} else {
		UI_DrawString(80, 72, "UHF      ", 9);
	}
}

void UI_DrawScanFrequency(uint32_t Frequency)
{
	uint8_t i;

	gColorForeground = COLOR_RED;
	Int2Ascii(Frequency, 8);
	for (i = 8; i > 3; i--) {
		gShortString[i] = gShortString[i - 1];
	}
	gShortString[3] = '.';
	UI_DrawString(80, 56, gShortString, 9);
}

void UI_DrawCtdcScan(void)
{
	gColorForeground = COLOR_RED;
	UI_DrawString(80, 72, "CTDC Scan", 9);
}

void UI_DrawCtcssCode(uint16_t Code)
{
	gColorForeground = COLOR_RED;
	Int2Ascii(Code, 4);
	gShortString[4] = gShortString[3];
	gShortString[3] = '.';
	UI_DrawString(80, 40, gShortString, 5);
}

void UI_DrawDcsCodeN(uint16_t Code)
{
	gColorForeground = COLOR_RED;
	Int2Ascii(CSS_ConvertCode(Code), 3);
	gShortString[3] = gShortString[2];
	gShortString[2] = gShortString[1];
	gShortString[1] = gShortString[0];
	gShortString[0] = 'D';
	gShortString[4] = 'N';
	UI_DrawString(80, 40, gShortString, 5);
}

void UI_DrawDTMFString(void)
{
	if (gDTMF_Settings.Display) {
		if (!gDataDisplay) {
			UI_DrawDialog();
			gColorForeground = COLOR_RED;
			UI_DrawString(10, 54, "DTMF:", 5);
		}
		gColorForeground = COLOR_RED;
		UI_DrawString(10, 38, gDTMF_String, 14);
	}
}

void UI_DrawMuteInfo(bool bIs24Bit, uint32_t Golay)
{
	gColorForeground = COLOR_RED;
	Int2Ascii(Golay, 8);
	if (bIs24Bit) {
		UI_DrawString(80, 40, "24bits", 6);
	} else {
		UI_DrawString(80, 40, "23bits", 6);
	}
	UI_DrawString(80, 24, gShortString, 8);
}

void UI_DrawNone(void)
{
	gColorForeground = COLOR_RED;
	UI_DrawString(80, 40, "None", 4);
}
