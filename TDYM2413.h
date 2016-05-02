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

#ifndef TDYM2413_h_
#define TDYM2413_h_

#include <AudioStream.h>
#include <Audio.h>
#include "midinotes.h"
#include <inttypes.h>

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

#ifndef PI
#define PI 3.14159265358979323846
#endif

#if (SAMPLE_BITS==16)
typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef INT8 SAMP;
#endif

class AudioTDYM2413: public AudioStream
{
	public:

		AudioTDYM2413(void) : AudioStream(0, NULL) { YM2413Init(0,0); }

		int32_t YM2413Init(uint32_t clock, uint32_t rate);
		
		void play(bool val) { playing = val; } 
		inline bool isPlaying(void) { return playing; }
		virtual void update(void);

	private:
		
		volatile bool playing;
		
		void OPLL_initalize(YM2413 *chip);
		
		typedef struct{
			UINT32	ar;			/* attack rate: AR<<2			*/
			UINT32	dr;			/* decay rate:  DR<<2			*/
			UINT32	rr;			/* release rate:RR<<2			*/
			UINT8	KSR;		/* key scale rate				*/
			UINT8	ksl;		/* keyscale level				*/
			UINT8	ksr;		/* key scale rate: kcode>>KSR	*/
			UINT8	mul;		/* multiple: mul_tab[ML]		*/

			/* Phase Generator */
			UINT32	phase;		/* frequency counter			*/
			UINT32	freq;		/* frequency counter step		*/
			UINT8   fb_shift;	/* feedback shift value			*/
			INT32   op1_out[2];	/* slot1 output for feedback	*/

			/* Envelope Generator */
			UINT8	eg_type;	/* percussive/nonpercussive mode*/
			UINT8	state;		/* phase type					*/
			UINT32	TL;			/* total level: TL << 2			*/
			INT32	TLL;		/* adjusted now TL				*/
			INT32	volume;		/* envelope counter				*/
			UINT32	sl;			/* sustain level: sl_tab[SL]	*/

			UINT8	eg_sh_dp;	/* (dump state)					*/
			UINT8	eg_sel_dp;	/* (dump state)					*/
			UINT8	eg_sh_ar;	/* (attack state)				*/
			UINT8	eg_sel_ar;	/* (attack state)				*/
			UINT8	eg_sh_dr;	/* (decay state)				*/
			UINT8	eg_sel_dr;	/* (decay state)				*/
			UINT8	eg_sh_rr;	/* (release state for non-perc.)*/
			UINT8	eg_sel_rr;	/* (release state for non-perc.)*/
			UINT8	eg_sh_rs;	/* (release state for perc.mode)*/
			UINT8	eg_sel_rs;	/* (release state for perc.mode)*/

			UINT32	key;		/* 0 = KEY OFF, >0 = KEY ON		*/

			/* LFO */
			UINT32	AMmask;		/* LFO Amplitude Modulation enable mask */
			UINT8	vib;		/* LFO Phase Modulation enable flag (active high)*/

			/* waveform select */
			unsigned int wavetable;
		} YM2413_OPLL_SLOT;
		
		typedef struct{
			YM2413_OPLL_SLOT SLOT[2];
			/* phase generator state */
			UINT32  block_fnum;	/* block+fnum					*/
			UINT32  fc;			/* Freq. freqement base			*/
			UINT32  ksl_base;	/* KeyScaleLevel Base step		*/
			UINT8   kcode;		/* key code (for key scaling)	*/
			UINT8   sus;		/* sus on/off (release speed in percussive mode)*/
		} YM2413_OPLL_CH;
		

		/* chip state */
		typedef struct {
			YM2413_OPLL_CH P_CH[9];                /* OPLL chips have 9 channels*/
			UINT8	instvol_r[9];			/* instrument/volume (or volume/volume in percussive mode)*/

			UINT32	eg_cnt;					/* global envelope generator counter	*/
			UINT32	eg_timer;				/* global envelope generator counter works at frequency = chipclock/72 */
			UINT32	eg_timer_add;			/* step of eg_timer						*/
			UINT32	eg_timer_overflow;		/* envelope generator timer overlfows every 1 sample (on real chip) */

			UINT8	rhythm;					/* Rhythm mode					*/

			/* LFO */
			UINT32	lfo_am_cnt;
			UINT32	lfo_am_inc;
			UINT32	lfo_pm_cnt;
			UINT32	lfo_pm_inc;

			UINT32	noise_rng;				/* 23 bit noise shift register	*/
			UINT32	noise_p;				/* current noise 'phase'		*/
			UINT32	noise_f;				/* current noise period			*/


		/* instrument settings */
		/*
			0-user instrument
			1-15 - fixed instruments
			16 -bass drum settings
			17,18 - other percussion instruments
		*/
			UINT8 inst_tab[19][8];

			/* external event callback handlers */
			OPLL_UPDATEHANDLER UpdateHandler; /* stream update handler		*/
			int UpdateParam;				/* stream update parameter		*/

			UINT32	fn_tab[1024];			/* fnumber->increment counter	*/

			UINT8 address;					/* address register				*/
			UINT8 status;					/* status flag					*/

			int clock;						/* master clock  (Hz)			*/
			int rate;						/* sampling rate (Hz)			*/
			double freqbase;				/* frequency base				*/
		} YM2413;
		
		YM2413 YM2413regs;
};

#endif
