/*
 * Copyright (C) 2026 nukeykt
 *
 * This file is part of Nuked OPL2 Lite.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  Nuked OPL2 Lite
 *  Thanks:
 *      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *          Feedback and Rhythm part calculation information.
 *      forums.submarine.org.uk(carbon14, opl3):
 *          Tremolo and phase generator calculation information.
 *      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *          OPL2 ROMs.
 *      siliconpr0n.org(John McMaster, digshadow):
 *          YMF262 and VRC VII decaps and die shots.
 *      Travis Goodspeed:
 *          YM3812 decap and die shot
 *
 * version: 0.9 beta
 */

#include <string.h>
#include "opl2.h"
#include "wf_rom.h"

#define RSM_FRAC    10

/* Channel types */

enum {
    ch_normal = 0,
    ch_drum = 1
};

/* Envelope key types */

enum {
    egk_norm = 0x01,
    egk_drum = 0x02
};

/* OPL2_HOT_IN_RAM places hot functions and tables in RAM; no-ops otherwise. */
#ifdef OPL2_HOT_IN_RAM
#define OPL2_RAM_TABLE __attribute__((section(".data.opl2_table")))
#define OPL2_HOT       __attribute__((section(".time_critical"))) __attribute__((hot))
#define OPL2_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define OPL2_RAM_TABLE
#define OPL2_HOT
#define OPL2_FORCE_INLINE inline
#endif

/*
    logsin table


static const uint16_t logsinrom[256] = {
    0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471,
    0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
    0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd,
    0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
    0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f,
    0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
    0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195,
    0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
    0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c,
    0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
    0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8,
    0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
    0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1,
    0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
    0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094,
    0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
    0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070,
    0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
    0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052,
    0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
    0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039,
    0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
    0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026,
    0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
    0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017,
    0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
    0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c,
    0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
    0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004,
    0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
    0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000
};
*/
/*
    exp table
*/
OPL2_RAM_TABLE
static const uint16_t exprom[256] = {
    0xff4, 0xfea, 0xfde, 0xfd4, 0xfc8, 0xfbe, 0xfb4, 0xfa8,
    0xf9e, 0xf92, 0xf88, 0xf7e, 0xf72, 0xf68, 0xf5c, 0xf52,
    0xf48, 0xf3e, 0xf32, 0xf28, 0xf1e, 0xf14, 0xf08, 0xefe,
    0xef4, 0xeea, 0xee0, 0xed4, 0xeca, 0xec0, 0xeb6, 0xeac,
    0xea2, 0xe98, 0xe8e, 0xe84, 0xe7a, 0xe70, 0xe66, 0xe5c,
    0xe52, 0xe48, 0xe3e, 0xe34, 0xe2a, 0xe20, 0xe16, 0xe0c,
    0xe04, 0xdfa, 0xdf0, 0xde6, 0xddc, 0xdd2, 0xdca, 0xdc0,
    0xdb6, 0xdac, 0xda4, 0xd9a, 0xd90, 0xd88, 0xd7e, 0xd74,
    0xd6a, 0xd62, 0xd58, 0xd50, 0xd46, 0xd3c, 0xd34, 0xd2a,
    0xd22, 0xd18, 0xd10, 0xd06, 0xcfe, 0xcf4, 0xcec, 0xce2,
    0xcda, 0xcd0, 0xcc8, 0xcbe, 0xcb6, 0xcae, 0xca4, 0xc9c,
    0xc92, 0xc8a, 0xc82, 0xc78, 0xc70, 0xc68, 0xc60, 0xc56,
    0xc4e, 0xc46, 0xc3c, 0xc34, 0xc2c, 0xc24, 0xc1c, 0xc12,
    0xc0a, 0xc02, 0xbfa, 0xbf2, 0xbea, 0xbe0, 0xbd8, 0xbd0,
    0xbc8, 0xbc0, 0xbb8, 0xbb0, 0xba8, 0xba0, 0xb98, 0xb90,
    0xb88, 0xb80, 0xb78, 0xb70, 0xb68, 0xb60, 0xb58, 0xb50,
    0xb48, 0xb40, 0xb38, 0xb32, 0xb2a, 0xb22, 0xb1a, 0xb12,
    0xb0a, 0xb02, 0xafc, 0xaf4, 0xaec, 0xae4, 0xade, 0xad6,
    0xace, 0xac6, 0xac0, 0xab8, 0xab0, 0xaa8, 0xaa2, 0xa9a,
    0xa92, 0xa8c, 0xa84, 0xa7c, 0xa76, 0xa6e, 0xa68, 0xa60,
    0xa58, 0xa52, 0xa4a, 0xa44, 0xa3c, 0xa36, 0xa2e, 0xa28,
    0xa20, 0xa18, 0xa12, 0xa0c, 0xa04, 0x9fe, 0x9f6, 0x9f0,
    0x9e8, 0x9e2, 0x9da, 0x9d4, 0x9ce, 0x9c6, 0x9c0, 0x9b8,
    0x9b2, 0x9ac, 0x9a4, 0x99e, 0x998, 0x990, 0x98a, 0x984,
    0x97c, 0x976, 0x970, 0x96a, 0x962, 0x95c, 0x956, 0x950,
    0x948, 0x942, 0x93c, 0x936, 0x930, 0x928, 0x922, 0x91c,
    0x916, 0x910, 0x90a, 0x904, 0x8fc, 0x8f6, 0x8f0, 0x8ea,
    0x8e4, 0x8de, 0x8d8, 0x8d2, 0x8cc, 0x8c6, 0x8c0, 0x8ba,
    0x8b4, 0x8ae, 0x8a8, 0x8a2, 0x89c, 0x896, 0x890, 0x88a,
    0x884, 0x87e, 0x878, 0x872, 0x86c, 0x866, 0x860, 0x85a,
    0x854, 0x850, 0x84a, 0x844, 0x83e, 0x838, 0x832, 0x82c,
    0x828, 0x822, 0x81c, 0x816, 0x810, 0x80c, 0x806, 0x800,
};

/*
    freq mult table multiplied by 2

    1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15
*/

static const uint8_t mt[16] = {
    1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
};

/*
    ksl table
*/

static const uint8_t kslrom[16] = {
    0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
};

static const uint8_t kslshift[4] = {
    8, 1, 2, 0
};

/*
    envelope generator constants
*/

static const uint8_t eg_incstep[4][4] = {
    { 0, 0, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 0, 1, 0 },
    { 1, 1, 1, 0 }
};

/*
    address decoding
*/

static const int8_t ad_slot[0x20] = {
    0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1,
    12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const int8_t ad_ch[0x10] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, -1, -1, 0, 1
};

static const int8_t ad_ch2[0x10] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1, -1, 0, 1
};

static const uint8_t ch_slot[9] = {
    0, 1, 2, 6, 7, 8, 12, 13, 14
};

/*
    Envelope generator
*/

enum envelope_gen_num
{
    envelope_gen_num_attack = 0,
    envelope_gen_num_decay = 1,
    envelope_gen_num_sustain = 2,
    envelope_gen_num_release = 3
};

OPL2_HOT static void OPL2_EnvelopeUpdateKSL(opl2_slot *slot)
{
    int16_t ksl = (kslrom[slot->channel->f_num >> 6u] << 2)
               - ((0x08 - slot->channel->block) << 5);
    if (ksl < 0)
    {
        ksl = 0;
    }
    slot->eg_ksl = (uint8_t)ksl;
	/* Refresh the cached (reg_tl << 2) + (eg_ksl >> kslshift[reg_ksl])
     * sum used by OPL3_EnvelopeCalc. Both reg_tl/reg_ksl-driven changes
     * (via SlotWrite40, which calls this function) and eg_ksl-driven
     * changes (f_num/block updates via Channel{A0,B0}) flow through
     * here, so this covers all dirty cases. */
    slot->eg_tl_ksl = (uint16_t)((slot->reg_tl << 2)
                              + (slot->eg_ksl >> kslshift[slot->reg_ksl]));
}

OPL2_HOT static void OPL2_EnvelopeUpdateRate(opl2_slot *slot)
{
    uint8_t ii;

    slot->eg_ks = slot->channel->ksv >> ((slot->reg_ksr ^ 1) << 1);
    for (ii = 0; ii < 4; ii++)
    {
        uint8_t rate = slot->eg_ks + (slot->eg_rates[ii] << 2);
        uint8_t rate_hi = rate >> 2;
        if (rate_hi & 0x10)
        {
            rate_hi = 0x0f;
        }
        slot->eg_rate_hi[ii] = rate_hi;
        slot->eg_rate_lo[ii] = rate & 0x03;
    }
}

OPL2_HOT static void OPL2_EnvelopeCalc(opl2_slot *slot)
{
    uint8_t nonzero;
    uint8_t rate_hi;
    uint8_t rate_lo;
    uint8_t reg_rate = 0;
    uint8_t eg_shift, shift;
    uint16_t eg_rout;
    int16_t eg_inc;
    uint8_t eg_off;
    uint8_t reset = 0;
    uint8_t key;
    slot->eg_out = slot->eg_rout + slot->eg_tl_ksl + *slot->trem;
    key = slot->key | slot->chip->csm_kon;
    if (key && slot->eg_gen == envelope_gen_num_release)
    {
        reset = 1;
        reg_rate = slot->eg_rates[0];
    }
    else
    {
        reg_rate = slot->eg_rates[slot->eg_gen];
    }
    slot->pg_reset = reset;
    nonzero = (reg_rate != 0);
    if (reset)
    {
        rate_hi = slot->eg_rate_hi[0];
        rate_lo = slot->eg_rate_lo[0];
    }
    else
    {
        rate_hi = slot->eg_rate_hi[slot->eg_gen];
        rate_lo = slot->eg_rate_lo[slot->eg_gen];
    }
    eg_shift = rate_hi + slot->chip->eg_add;
    shift = 0;
    if (nonzero)
    {
        if (rate_hi < 12)
        {
            if (slot->chip->eg_state)
            {
                switch (eg_shift)
                {
                case 12:
                    shift = 1;
                    break;
                case 13:
                    shift = (rate_lo >> 1) & 0x01;
                    break;
                case 14:
                    shift = rate_lo & 0x01;
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            shift = (rate_hi & 0x03) + eg_incstep[rate_lo][slot->chip->eg_timer_lo];
            if (shift & 0x04)
            {
                shift = 0x03;
            }
            if (!shift)
            {
                shift = slot->chip->eg_state;
            }
        }
    }
    eg_rout = slot->eg_rout;
    eg_inc = 0;
    eg_off = 0;
    /* Instant attack */
    if (reset && rate_hi == 0x0f)
    {
        eg_rout = 0x00;
    }
    /* Envelope off */
    if ((slot->eg_rout & 0x1f8) == 0x1f8)
    {
        eg_off = 1;
    }
    slot->eg_mute = slot->eg_gen != envelope_gen_num_attack && !reset && eg_off;
    if (slot->eg_mute)
    {
        eg_rout = 0x1ff;
    }
    switch (slot->eg_gen)
    {
    case envelope_gen_num_attack:
        if (!slot->eg_rout)
        {
            slot->eg_gen = envelope_gen_num_decay;
        }
        else if (key && shift > 0 && rate_hi != 0x0f)
        {
            eg_inc = ~slot->eg_rout >> (4 - shift);
        }
        break;
    case envelope_gen_num_decay:
        if ((slot->eg_rout >> 4) == slot->reg_sl)
        {
            slot->eg_gen = envelope_gen_num_sustain;
        }
        else if (!eg_off && !reset && shift > 0)
        {
            eg_inc = 1 << (shift - 1);
        }
        break;
    case envelope_gen_num_sustain:
    case envelope_gen_num_release:
        if (!eg_off && !reset && shift > 0)
        {
            eg_inc = 1 << (shift - 1);
        }
        break;
    }
    slot->eg_rout = (eg_rout + eg_inc) & 0x1ff;
    /* Key off */
    if (reset)
    {
        slot->eg_gen = envelope_gen_num_attack;
    }
    if (!key)
    {
        slot->eg_gen = envelope_gen_num_release;
    }
}

static void OPL2_EnvelopeKeyOn(opl2_slot *slot, uint8_t type)
{
    slot->key |= type;
}

static void OPL2_EnvelopeKeyOff(opl2_slot *slot, uint8_t type)
{
    slot->key &= ~type;
}

/*
    Phase Generator
*/

static void OPL2_PhaseUpdateInc(opl2_slot *slot)
{
    uint32_t basefreq = ((uint32_t)slot->channel->f_num << slot->channel->block) >> 1;
    slot->pg_inc = (basefreq * mt[slot->reg_mult]) >> 1;
}

OPL2_HOT static void OPL2_PhaseGenerate(opl2_slot *slot)
{
    opl2_chip *chip;
    uint16_t f_num;
    uint32_t basefreq;
	uint32_t phaseinc;
    uint8_t rm_xor, n_bit;
    uint32_t noise;
    uint16_t phase;

    chip = slot->chip;
    if (slot->reg_vib)
    {
        int8_t range;
        uint8_t vibpos;
		
		f_num = slot->channel->f_num;

        range = (f_num >> 7) & 7;
        vibpos = slot->chip->vibpos;

        if (!(vibpos & 3))
        {
            range = 0;
        }
        else if (vibpos & 1)
        {
            range >>= 1;
        }
        range >>= slot->chip->vibshift;

        if (vibpos & 4)
        {
            range = -range;
        }
        f_num += range;
		basefreq = (f_num << slot->channel->block) >> 1;
        phaseinc = (basefreq * mt[slot->reg_mult]) >> 1;
    }
	else
    {
        phaseinc = slot->pg_inc;
    }
    phase = (uint16_t)(slot->pg_phase >> 9);
    if (slot->pg_reset)
    {
        slot->pg_phase = 0;
    }
	slot->pg_phase += phaseinc;
    /* Rhythm mode */
    noise = chip->noise;
    slot->pg_phase_out = phase;
    switch (slot->slot_num)
    {
    case 13: /* hh */
        chip->rm_hh_bit2 = (phase >> 2) & 1;
        chip->rm_hh_bit3 = (phase >> 3) & 1;
        chip->rm_hh_bit7 = (phase >> 7) & 1;
        chip->rm_hh_bit8 = (phase >> 8) & 1;
        if (chip->rhy & 0x20)
        {
            rm_xor = (chip->rm_hh_bit2 ^ chip->rm_hh_bit7)
                   | (chip->rm_hh_bit3 ^ chip->rm_tc_bit5)
                   | (chip->rm_tc_bit3 ^ chip->rm_tc_bit5);
            slot->pg_phase_out = rm_xor << 9;
            if (rm_xor ^ (noise & 1))
            {
                slot->pg_phase_out |= 0xd0;
            }
            else
            {
                slot->pg_phase_out |= 0x34;
            }
        }
        break;
    case 16: /* sd */
        if (chip->rhy & 0x20)
        {
            slot->pg_phase_out = (chip->rm_hh_bit8 << 9)
                               | ((chip->rm_hh_bit8 ^ (noise & 1)) << 8);
        }
        break;
    case 17: /* tc */
        if (chip->rhy & 0x20)
        {
            chip->rm_tc_bit3 = (phase >> 3) & 1;
            chip->rm_tc_bit5 = (phase >> 5) & 1;
            rm_xor = (chip->rm_hh_bit2 ^ chip->rm_hh_bit7)
                   | (chip->rm_hh_bit3 ^ chip->rm_tc_bit5)
                   | (chip->rm_tc_bit3 ^ chip->rm_tc_bit5);
            slot->pg_phase_out = (rm_xor << 9) | 0x100;
        }
        break;
    default:
        break;
    }
    n_bit = ((noise >> 14) ^ noise) & 0x01;
    chip->noise = (noise >> 1) | (n_bit << 22);
}

/*
    Slot
*/

static void OPL2_SlotWrite20(opl2_slot *slot, uint8_t data)
{
    if ((data >> 7) & 0x01)
    {
        slot->trem = &slot->chip->tremolo;
    }
    else
    {
        slot->trem = (uint8_t*)&slot->chip->zeromod;
    }
    slot->reg_vib = (data >> 6) & 0x01;
    slot->reg_type = (data >> 5) & 0x01;
	slot->eg_rates[2] = slot->reg_type ? 0 : slot->reg_rr;
    slot->reg_ksr = (data >> 4) & 0x01;
    slot->reg_mult = data & 0x0f;
	OPL2_EnvelopeUpdateRate(slot);
    OPL2_PhaseUpdateInc(slot);
}

static void OPL2_SlotWrite40(opl2_slot *slot, uint8_t data)
{
    slot->reg_ksl = (data >> 6) & 0x03;
    slot->reg_tl = data & 0x3f;
    OPL2_EnvelopeUpdateKSL(slot);
}

static void OPL2_SlotWrite60(opl2_slot *slot, uint8_t data)
{
    slot->reg_ar = (data >> 4) & 0x0f;
    slot->reg_dr = data & 0x0f;
	slot->eg_rates[0] = slot->reg_ar;
    slot->eg_rates[1] = slot->reg_dr;
    OPL2_EnvelopeUpdateRate(slot);
}

static void OPL2_SlotWrite80(opl2_slot *slot, uint8_t data)
{
    slot->reg_sl = (data >> 4) & 0x0f;
    if (slot->reg_sl == 0x0f)
    {
        slot->reg_sl = 0x1f;
    }
    slot->reg_rr = data & 0x0f;
    slot->eg_rates[2] = slot->reg_type ? 0 : slot->reg_rr;
    slot->eg_rates[3] = slot->reg_rr;
    OPL2_EnvelopeUpdateRate(slot);
}

static void OPL2_SlotWriteE0(opl2_slot *slot, uint8_t data)
{
    if (slot->chip->wfe)
        slot->reg_wf = data & 0x03;
}

static OPL2_FORCE_INLINE void OPL2_SlotGenerate(opl2_slot *slot)
{
    uint16_t phase = slot->pg_phase_out + *slot->mod;
    uint16_t envelope = slot->eg_out;
    uint16_t wf_data = logsin_wf[slot->reg_wf][phase & 0x3ff];
    uint16_t neg = (uint16_t)(((int16_t)wf_data) >> 15);
    uint32_t level = (wf_data & 0x7fff) + (envelope << 3);
    if (level > 0x1fff)
    {
        level = 0x1fff;
    }
    slot->out = ((exprom[level & 0xffu] >> (level >> 8)) ^ neg);
}

/* Silent-regime variant: when the caller has proven eg_out >= 0x180, the
 * exprom lookup always reads through to zero (max exprom value 0xff4 >> 12
 * = 0), so the final out reduces to just the sign bit of wf_data. Skips a
 * load, an add, a clamp, a shift, and a xor. */
static OPL2_FORCE_INLINE void OPL2_SlotGenerateSilent(opl2_slot *slot)
{
    uint16_t phase = slot->pg_phase_out + *slot->mod;
    uint16_t wf_data = logsin_wf[slot->reg_wf][phase & 0x3ff];
    slot->out = (int16_t)wf_data >> 15;
}

static OPL2_FORCE_INLINE void OPL2_SlotCalcFB(opl2_slot *slot)
{
    if (slot->channel->fb != 0x00)
    {
        slot->fbmod = (slot->prout + slot->out) >> (0x09 - slot->channel->fb);
    }
    else
    {
        slot->fbmod = 0;
    }
    slot->prout = slot->out;
}

/*
    Channel
*/

static void OPL2_ChannelSetupCon(opl2_channel *channel);

static void OPL2_ChannelUpdateRhythm(opl2_chip *chip, uint8_t data)
{
    opl2_channel *channel6;
    opl2_channel *channel7;
    opl2_channel *channel8;
    uint8_t chnum;

    chip->rhy = data & 0x3f;
    if (chip->rhy & 0x20)
    {
        channel6 = &chip->channel[6];
        channel7 = &chip->channel[7];
        channel8 = &chip->channel[8];
        for (chnum = 6; chnum < 9; chnum++)
        {
            chip->channel[chnum].chtype = ch_drum;
        }
        OPL2_ChannelSetupCon(channel6);
        OPL2_ChannelSetupCon(channel7);
        OPL2_ChannelSetupCon(channel8);
        /* hh */
        if (chip->rhy & 0x01)
        {
            OPL2_EnvelopeKeyOn(channel7->slotz[0], egk_drum);
        }
        else
        {
            OPL2_EnvelopeKeyOff(channel7->slotz[0], egk_drum);
        }
        /* tc */
        if (chip->rhy & 0x02)
        {
            OPL2_EnvelopeKeyOn(channel8->slotz[1], egk_drum);
        }
        else
        {
            OPL2_EnvelopeKeyOff(channel8->slotz[1], egk_drum);
        }
        /* tom */
        if (chip->rhy & 0x04)
        {
            OPL2_EnvelopeKeyOn(channel8->slotz[0], egk_drum);
        }
        else
        {
            OPL2_EnvelopeKeyOff(channel8->slotz[0], egk_drum);
        }
        /* sd */
        if (chip->rhy & 0x08)
        {
            OPL2_EnvelopeKeyOn(channel7->slotz[1], egk_drum);
        }
        else
        {
            OPL2_EnvelopeKeyOff(channel7->slotz[1], egk_drum);
        }
        /* bd */
        if (chip->rhy & 0x10)
        {
            OPL2_EnvelopeKeyOn(channel6->slotz[0], egk_drum);
            OPL2_EnvelopeKeyOn(channel6->slotz[1], egk_drum);
        }
        else
        {
            OPL2_EnvelopeKeyOff(channel6->slotz[0], egk_drum);
            OPL2_EnvelopeKeyOff(channel6->slotz[1], egk_drum);
        }
    }
    else
    {
        for (chnum = 6; chnum < 9; chnum++)
        {
            chip->channel[chnum].chtype = ch_normal;
            OPL2_ChannelSetupCon(&chip->channel[chnum]);
            OPL2_EnvelopeKeyOff(chip->channel[chnum].slotz[0], egk_drum);
            OPL2_EnvelopeKeyOff(chip->channel[chnum].slotz[1], egk_drum);
        }
    }
}

static void OPL2_ChannelWriteA0(opl2_channel *channel, uint8_t data)
{
    channel->f_num = (channel->f_num & 0x300) | data;
    channel->ksv = (channel->block << 1)
                 | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    OPL2_EnvelopeUpdateKSL(channel->slotz[0]);
    OPL2_EnvelopeUpdateKSL(channel->slotz[1]);
	OPL2_EnvelopeUpdateRate(channel->slotz[0]);
    OPL2_EnvelopeUpdateRate(channel->slotz[1]);
    OPL2_PhaseUpdateInc(channel->slotz[0]);
    OPL2_PhaseUpdateInc(channel->slotz[1]);
}

static void OPL2_ChannelKeyOn(opl2_channel* channel)
{
    OPL2_EnvelopeKeyOn(channel->slotz[0], egk_norm);
    OPL2_EnvelopeKeyOn(channel->slotz[1], egk_norm);
}

static void OPL2_ChannelKeyOff(opl2_channel* channel)
{
    OPL2_EnvelopeKeyOff(channel->slotz[0], egk_norm);
    OPL2_EnvelopeKeyOff(channel->slotz[1], egk_norm);
}

static void OPL2_ChannelWriteB0(opl2_channel *channel, uint8_t data)
{
    channel->f_num = (channel->f_num & 0xff) | ((data & 0x03) << 8);
    channel->block = (data >> 2) & 0x07;
    channel->ksv = (channel->block << 1)
                 | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    OPL2_EnvelopeUpdateKSL(channel->slotz[0]);
    OPL2_EnvelopeUpdateKSL(channel->slotz[1]);
	OPL2_EnvelopeUpdateRate(channel->slotz[0]);
    OPL2_EnvelopeUpdateRate(channel->slotz[1]);
    OPL2_PhaseUpdateInc(channel->slotz[0]);
    OPL2_PhaseUpdateInc(channel->slotz[1]);
    if (data & 0x20)
        OPL2_ChannelKeyOn(channel);
    else
        OPL2_ChannelKeyOff(channel);
}

static void OPL2_ChannelSetupCon(opl2_channel *channel)
{
    if (channel->chtype == ch_drum)
    {
        if (channel->ch_num == 7 || channel->ch_num == 8)
        {
            channel->slotz[0]->mod = &channel->chip->zeromod;
            channel->slotz[1]->mod = &channel->chip->zeromod;
            return;
        }
        switch (channel->con)
        {
        case 0x00:
            channel->slotz[0]->mod = &channel->slotz[0]->fbmod;
            channel->slotz[1]->mod = &channel->slotz[0]->out;
            break;
        case 0x01:
            channel->slotz[0]->mod = &channel->slotz[0]->fbmod;
            channel->slotz[1]->mod = &channel->chip->zeromod;
            break;
        }
        return;
    }
    switch (channel->con)
    {
    case 0x00:
        channel->slotz[0]->mod = &channel->slotz[0]->fbmod;
        channel->slotz[1]->mod = &channel->slotz[0]->out;
        break;
    case 0x01:
        channel->slotz[0]->mod = &channel->slotz[0]->fbmod;
        channel->slotz[1]->mod = &channel->chip->zeromod;
        break;
    }
}

static void OPL2_ChannelWriteC0(opl2_channel *channel, uint8_t data)
{
    channel->fb = (data & 0x0e) >> 1;
    channel->con = data & 0x01;
    OPL2_ChannelSetupCon(channel);
}

static void OPL2_UpdateChannelParams(opl2_chip *chip, uint8_t slot)
{
    if (slot < 12)
    {
        uint8_t ch = slot;
        if (ch >= 9)
            ch -= 9;
        if (chip->ch_upd_a0 & (1u << slot))
            OPL2_ChannelWriteA0(&chip->channel[ch], chip->ch_upd_a0_value[slot]);
        if (chip->ch_upd_b0 & (1u << slot))
            OPL2_ChannelWriteB0(&chip->channel[ch], chip->ch_upd_b0_value[slot]);
    }
}

static OPL2_FORCE_INLINE void OPL2_ProcessSlot(opl2_slot *slot)
{
    OPL2_SlotCalcFB(slot);
    OPL2_EnvelopeCalc(slot);
    OPL2_PhaseGenerate(slot);
    OPL2_SlotGenerate(slot);
}

static void OPL2_ProcessTimers(opl2_chip *chip)
{
    chip->csm_kon = 0;
    if (chip->t1_start && (chip->timer & 0x3) == 0x3)
    {
        chip->t1_value++;
        if (chip->t1_value == 0u)
        {
            if (chip->csm_enable)
                chip->csm_kon = 1;
            chip->t1_value = chip->t1_reg;
            if (!chip->t1_mask)
                chip->t1_status = 1;
        }
    }
    if (chip->t2_start && (chip->timer & 0xf) == 0xf)
    {
        chip->t2_value++;
        if (chip->t2_value == 0u)
        {
            chip->t2_value = chip->t2_reg;
            if (!chip->t2_mask)
                chip->t2_status = 1;
        }
    }
}

static int16_t OPL2_OutputCrush(int32_t sample)
{
    uint8_t shift;
    int32_t top;
    if (sample > 32767)
        sample = 32767;
    else if (sample < -32768)
        sample = -32768;

    top = sample >> 9;
    if (top < 0)
        top = (~top) & 63;
    else
        top = top & 63;
    shift = 0;
    if (top & 32)
        shift = 6;
    else if ((top & 48) == 16)
        shift = 5;
    else if ((top & 56) == 8)
        shift = 4;
    else if ((top & 60) == 4)
        shift = 3;
    else if ((top & 62) == 2)
        shift = 2;
    else if (top == 1)
        shift = 1;
    else if (top == 0)
        shift = 0;

    sample >>= shift;
    sample <<= shift;

    return (int16_t)sample;
}

OPL2_HOT void OPL2_Generate(opl2_chip *chip, int16_t * sample)
{
    opl2_writebuf *writebuf;
    int32_t mix;
    uint8_t ii;
    uint8_t shift;
    uint8_t update_tremolo;

    *sample = OPL2_OutputCrush(chip->mixbuff);

    for (ii = 0; ii < 15; ii++)
    {
        OPL2_ProcessSlot(&chip->slot[ii]);
        OPL2_UpdateChannelParams(chip, ii);
    }

    mix = 0;
    if (chip->channel[0].con && !chip->slot[0].eg_mute)
        mix += chip->slot[0].out;
    if (chip->channel[1].con && !chip->slot[1].eg_mute)
        mix += chip->slot[1].out;
    if (chip->channel[2].con && !chip->slot[2].eg_mute)
        mix += chip->slot[2].out;
    if (!chip->slot[3].eg_mute)
        mix += chip->slot[3].out;
    if (!chip->slot[4].eg_mute)
        mix += chip->slot[4].out;
    if (!chip->slot[5].eg_mute)
        mix += chip->slot[5].out;
    if (chip->channel[3].con && !chip->slot[6].eg_mute)
        mix += chip->slot[6].out;
    if (chip->channel[4].con && !chip->slot[7].eg_mute)
        mix += chip->slot[7].out;
    if (chip->channel[5].con && !chip->slot[8].eg_mute)
        mix += chip->slot[8].out;
    if (!chip->slot[9].eg_mute)
        mix += chip->slot[9].out;
    if (!chip->slot[10].eg_mute)
        mix += chip->slot[10].out;
    if (!chip->slot[11].eg_mute)
        mix += chip->slot[11].out;
    if (chip->rhy & 0x20)
    {
        if (!chip->slot[13].eg_mute)
            mix += chip->slot[13].out << 1;
        if (!chip->slot[14].eg_mute)
            mix += chip->slot[14].out << 1;
        if (!chip->slot[15].eg_mute)
            mix += chip->slot[15].out << 1;
        if (!chip->slot[16].eg_mute)
            mix += chip->slot[16].out << 1;
        if (!chip->slot[17].eg_mute)
            mix += chip->slot[17].out << 1;
    }
    else
    {
        if (chip->channel[6].con && !chip->slot[12].eg_mute)
            mix += chip->slot[12].out;
        if (chip->channel[7].con && !chip->slot[13].eg_mute)
            mix += chip->slot[13].out;
        if (chip->channel[8].con && !chip->slot[14].eg_mute)
            mix += chip->slot[14].out;
        if (!chip->slot[15].eg_mute)
            mix += chip->slot[15].out;
        if (!chip->slot[16].eg_mute)
            mix += chip->slot[16].out;
        if (!chip->slot[17].eg_mute)
            mix += chip->slot[17].out;
    }
    chip->mixbuff = mix;

    for (ii = 15; ii < 18; ii++)
    {
        OPL2_ProcessSlot(&chip->slot[ii]);
        OPL2_UpdateChannelParams(chip, ii);
    }

    chip->ch_upd_a0 = 0;
    chip->ch_upd_b0 = 0;

    OPL2_ProcessTimers(chip);

    update_tremolo = chip->tremolo_dirty;
    if ((chip->timer & 0x3f) == 0x3f)
    {
        chip->tremolopos++;
        if (chip->tremolopos == 210)
        {
            chip->tremolopos = 0;
        }
        update_tremolo = 1;
    }
    if (update_tremolo)
    {
        if (chip->tremolopos < 105)
        {
            chip->tremolo = chip->tremolopos >> chip->tremoloshift;
        }
        else
        {
            chip->tremolo = (210 - chip->tremolopos) >> chip->tremoloshift;
        }
        chip->tremolo_dirty = 0;
    }

    if ((chip->timer & 0x3ff) == 0x3ff)
    {
        chip->vibpos = (chip->vibpos + 1) & 7;
    }

    chip->timer++;

    if (chip->eg_state)
    {
        shift = 0;
        while (shift < 13 && ((chip->eg_timer >> shift) & 1) == 0)
        {
            shift++;
        }
        if (shift > 12)
        {
            chip->eg_add = 0;
        }
        else
        {
            chip->eg_add = shift + 1;
        }
        chip->eg_timer_lo = (uint8_t)(chip->eg_timer & 0x3u);

        if (chip->eg_timer == 0x3ffff)
        {
            chip->eg_timer = 0;
            chip->eg_timerrem = 1;
        }
        else
        {
            chip->eg_timer++;
            chip->eg_timer += chip->eg_timerrem;
            chip->eg_timerrem = 0;
        }
    }

    chip->eg_state ^= 1;

    while ((writebuf = &chip->writebuf[chip->writebuf_cur]), writebuf->time <= chip->writebuf_samplecnt)
    {
        if (!(writebuf->reg & 0x100))
        {
            break;
        }
        writebuf->reg &= 0xff;
        OPL2_WriteReg(chip, writebuf->reg, writebuf->data);
        chip->writebuf_cur = (chip->writebuf_cur + 1) % OPL2_WRITEBUF_SIZE;
    }
    chip->writebuf_samplecnt++;
}

void OPL2_GenerateResampled(opl2_chip *chip, int16_t *sample)
{
    while (chip->samplecnt >= chip->rateratio)
    {
        chip->oldsample = chip->sample;
        OPL2_Generate(chip, &chip->sample);
        chip->samplecnt -= chip->rateratio;
    }
    *sample = (int16_t)((chip->oldsample * (chip->rateratio - chip->samplecnt)
                        + chip->sample * chip->samplecnt) / chip->rateratio);
    chip->samplecnt += 1 << RSM_FRAC;
}

void OPL2_Reset(opl2_chip *chip, uint32_t samplerate)
{
    opl2_slot *slot;
    opl2_channel *channel;
    uint8_t slotnum;
    uint8_t channum;
    uint8_t local_ch_slot;

    memset(chip, 0, sizeof(opl2_chip));
    for (slotnum = 0; slotnum < 18; slotnum++)
    {
        slot = &chip->slot[slotnum];
        slot->chip = chip;
        slot->mod = &chip->zeromod;
        slot->eg_rout = 0x1ff;
        slot->eg_out = 0x1ff;
        slot->eg_gen = envelope_gen_num_release;
        slot->trem = (uint8_t*)&chip->zeromod;
        slot->slot_num = slotnum;
    }
    for (channum = 0; channum < 9; channum++)
    {
        channel = &chip->channel[channum];
        local_ch_slot = ch_slot[channum];
        channel->slotz[0] = &chip->slot[local_ch_slot];
        channel->slotz[1] = &chip->slot[local_ch_slot + 3u];
        chip->slot[local_ch_slot].channel = channel;
        chip->slot[local_ch_slot + 3u].channel = channel;
        channel->chip = chip;
        channel->chtype = ch_normal;
        channel->ch_num = channum;
        OPL2_ChannelSetupCon(channel);
    }
    chip->noise = 1;
    chip->rateratio = (samplerate << RSM_FRAC) / 49716;
    chip->tremoloshift = 4;
    chip->vibshift = 1;
}

OPL2_HOT void OPL2_WriteReg(opl2_chip *chip, uint8_t reg, uint8_t v)
{
    uint8_t regm = reg & 0xff;
    switch (regm & 0xf0)
    {
    case 0x00:
        switch (regm & 0x0f)
        {
        case 0x01:
            chip->wfe = (v >> 5) & 1;
            break;
        case 0x02:
            chip->t1_reg = v;
            break;
        case 0x03:
            chip->t2_reg = v;
            break;
        case 0x04:
            if (v & 0x80)
            {
                chip->t1_status = 0;
                chip->t2_status = 0;
            }
            else
            {
                if (!chip->t1_start && (v & 1))
                    chip->t1_value = chip->t1_reg;
                if (!chip->t2_start && (v & 2))
                    chip->t2_value = chip->t2_reg;
                chip->t1_mask = (v >> 6) & 1;
                chip->t2_mask = (v >> 5) & 1;
                chip->t1_start = (v >> 0) & 1;
                chip->t2_start = (v >> 1) & 1;
                if (!chip->t1_mask)
                    chip->t1_status = 0;
                if (!chip->t2_mask)
                    chip->t2_status = 0;
            }
            break;
        case 0x08:
            chip->nts = (v >> 6) & 0x01;
            chip->csm_enable = (v >> 7) & 1;
            break;
        }
        break;
    case 0x20:
    case 0x30:
        if (ad_slot[regm & 0x1fu] >= 0)
        {
            OPL2_SlotWrite20(&chip->slot[ad_slot[regm & 0x1fu]], v);
        }
        break;
    case 0x40:
    case 0x50:
        if (ad_slot[regm & 0x1fu] >= 0)
        {
            OPL2_SlotWrite40(&chip->slot[ad_slot[regm & 0x1fu]], v);
        }
        break;
    case 0x60:
    case 0x70:
        if (ad_slot[regm & 0x1fu] >= 0)
        {
            OPL2_SlotWrite60(&chip->slot[ad_slot[regm & 0x1fu]], v);
        }
        break;
    case 0x80:
    case 0x90:
        if (ad_slot[regm & 0x1fu] >= 0)
        {
            OPL2_SlotWrite80(&chip->slot[ad_slot[regm & 0x1fu]], v);
        }
        break;
    case 0xe0:
    case 0xf0:
        if (ad_slot[regm & 0x1fu] >= 0)
        {
            OPL2_SlotWriteE0(&chip->slot[ad_slot[regm & 0x1fu]], v);
        }
        break;
    case 0xa0:
        if (ad_ch2[regm & 0xfu] >= 0)
        {
            int8_t ch = ad_ch2[regm & 0xfu];
            chip->ch_upd_a0 |= 1 << ch;
            chip->ch_upd_a0_value[ch] = v;
        }
        break;
    case 0xb0:
        if (regm == 0xbd)
        {
            uint8_t tremoloshift = (((v >> 7) ^ 1) << 1) + 2;
            if (chip->tremoloshift != tremoloshift)
            {
                chip->tremolo_dirty = 1;
            }
            chip->tremoloshift = tremoloshift;
            chip->vibshift = ((v >> 6) & 0x01) ^ 1;
            OPL2_ChannelUpdateRhythm(chip, v);
        }
        if (ad_ch2[regm & 0xfu] >= 0)
        {
            int8_t ch = ad_ch2[regm & 0xfu];
            chip->ch_upd_b0 |= 1 << ch;
            chip->ch_upd_b0_value[ch] = v;
        }
        break;
    case 0xc0:
        if (ad_ch[regm & 0xfu] >= 0)
        {
            OPL2_ChannelWriteC0(&chip->channel[ad_ch[regm & 0xfu]], v);
        }
        break;
    }
}

OPL2_HOT void OPL2_WriteRegBuffered(opl2_chip *chip, uint8_t reg, uint8_t v)
{
    uint64_t time1, time2;
    opl2_writebuf *writebuf;
    uint32_t writebuf_last;

    writebuf_last = chip->writebuf_last;
    writebuf = &chip->writebuf[writebuf_last];

    if (writebuf->reg & 0x100)
    {
        OPL2_WriteReg(chip, writebuf->reg & 0xff, writebuf->data);

        chip->writebuf_cur = (writebuf_last + 1) % OPL2_WRITEBUF_SIZE;
        chip->writebuf_samplecnt = writebuf->time;
    }

    writebuf->reg = (uint16_t)reg | 0x100;
    writebuf->data = v;
    time1 = chip->writebuf_lasttime + OPL2_WRITEBUF_DELAY;
    time2 = chip->writebuf_samplecnt;

    if (time1 < time2)
    {
        time1 = time2;
    }

    writebuf->time = time1;
    chip->writebuf_lasttime = time1;
    chip->writebuf_last = (writebuf_last + 1) % OPL2_WRITEBUF_SIZE;
}

void OPL2_GenerateStream(opl2_chip *chip, int16_t *sndptr, uint32_t numsamples)
{
    uint_fast32_t i;

    for(i = 0; i < numsamples; i++)
    {
        OPL2_GenerateResampled(chip, sndptr);
        sndptr++;
    }
}

uint8_t OPL2_ReadStatus(opl2_chip *chip)
{
    uint8_t status = 0x6u;
    if (chip->t1_status)
        status |= 0xc0u;
    if (chip->t2_status)
        status |= 0xa0u;

    return status;
}

