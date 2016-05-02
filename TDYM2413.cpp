 /*
    Title:          TDYM2413.h
    Author:         René Richard
    Description:
        
    Target Hardware:
        Teensy 3.2 + Audio Adapter
    Arduino IDE settings:
        Board Type  - Teensy 3.2
        USB Type    - Serial
 LICENSE
 
    This file is part of TDYM2413.
    TDYM2413 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    TDYM2413 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with TDYM2413.  If not, see <http://www.gnu.org/licenses/>.
*/

//reference http://www.smspower.org/Development/SN76489 throughout

#include "TDYM2413.h"

//Update function is called by the Teensy audio libray, this is what fills the audio output buffer
void AudioTDYM2413::update(void)
{
	audio_block_t *block;
    uint32_t sampleCount;

	// only update if we're playing
	if (!playing) return;

	// allocate the audio blocks to transmit
	block = allocate();
	if (block == NULL) return;

    for(sampleCount = 0; sampleCount < AUDIO_BLOCK_SAMPLES; sampleCount++) 
	{

        // block->data[sampleCount] = ( channels[0] + channels[1] + channels[2] + channels[3] );
    }	

	transmit(block);
	release(block);
}

int32_t AudioTDYM2413::YM2413Init(uint32_t clock, uint32_t rate)
{
	uint32_t state_size;
	
	if (OPLL_LockTable() ==-1) return -1;
	
	/* calculate chip state size */
	state_size  = sizeof(YM2413);
	memset(YM2413regs,0,state_size);
	
	YM2413regs.clock = clock;
	YM2413regs.rate = rate;

	OPLL_initalize(YM2413regs);
	OPLLResetChip(YM2413regs);
	
	return 0;
}

void AudioTDYM2413::OPLL_initalize(YM2413 *chip)
{
	int i;

	/* frequency base */
	chip->freqbase  = (chip->rate) ? ((double)chip->clock / 72.0) / chip->rate  : 0;

	/* make fnumber -> increment counter table */
	for( i = 0 ; i < 1024; i++ )
	{
		/* OPLL (YM2413) phase increment counter = 18bit */
		chip->fn_tab[i] = (uint32_t)( (double)i * 64 * chip->freqbase * (1<<(FREQ_SH-10)) ); /* -10 because chip works with 10.10 fixed point, while we use 16.16 */
	}

	/* Amplitude modulation: 27 output levels (triangle waveform); 1 level takes one of: 192, 256 or 448 samples */
	/* One entry from LFO_AM_TABLE lasts for 64 samples */
	chip->lfo_am_inc = (1.0 / 64.0 ) * (1<<LFO_SH) * chip->freqbase;

	/* Vibrato: 8 output levels (triangle waveform); 1 level takes 1024 samples */
	chip->lfo_pm_inc = (1.0 / 1024.0) * (1<<LFO_SH) * chip->freqbase;

	/* Noise generator: a step takes 1 sample */
	chip->noise_f = (1.0 / 1.0) * (1<<FREQ_SH) * chip->freqbase;

	chip->eg_timer_add  = (1<<EG_SH)  * chip->freqbase;
	chip->eg_timer_overflow = ( 1 ) * (1<<EG_SH);
}
