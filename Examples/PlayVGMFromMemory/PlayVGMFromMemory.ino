 /*
    Title:          PlayVGMFromMemory.ino
    Author:         René Richard
    Description:
        
    Target Hardware:
        Teensy 3.2 + Audio Adapter
    Arduino IDE settings:
        Board Type  - Teensy 3.2
        USB Type    - Serial
 LICENSE
 
    This file is part of TDSN76489.
    TDSN76489 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with TDSN76489.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <TDSN76489.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SonicTitleScreen.h"

#define ONESAMPLE   (( 1 / AUDIO_SAMPLE_RATE ) * 1000000)   // microseconds per audio sample
#define ONE60TH     16667
#define ONE50TH     20000

#define VIEWPSGWRITES

elapsedMicros vgmTimer;
uint16_t vgmWait;
//hard code to start at 0x40 since I know this is a vgm 1.10 file
uint8_t *vgmptr = &TitleScreen[0x40];

AudioTDSN76489           psgChip;  //xy=189,110
AudioOutputI2S           i2s1;           //xy=366,111
AudioConnection          patchCord1(psgChip, 0, i2s1, 0);
AudioConnection          patchCord2(psgChip, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=354,176

void setup() {

  Serial.begin(9600); // USB is always 12 Mbit/sec

  AudioMemory(10);

  //there are several variations of LFSR for the noise channel, see TDSN76489.h 
  psgChip.reset(NOISE_BITS_SMS, NOISE_TAPPED_SMS);

  //parameters for the audio codec
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);

  //wait for thjings to settle
  delay(4000);
  Serial.println("Play VGM From Memory starting...");
  vgmWait = 0;

  //mute the psg, and enable VGM playback
  psgChip.write(0x9F);  //channel 1 volume off
  psgChip.write(0xBF);  //channel 2 volume off
  psgChip.write(0xDF);  //channel 3 volume off
  psgChip.write(0xFF);  //channel 4 volume off
  psgChip.play(true);

  delay(1000);

}

void loop() {

  bool doneFrame = false;

  if( vgmTimer >= vgmWait )
  {
    vgmTimer = 0;
    
    while(doneFrame == false)
    {
      switch(*vgmptr)
      {
        case 0x50: // 0x50 dd : PSG (SN76489/SN76496) write value dd
          vgmptr++;
          psgChip.write(*vgmptr);
#ifdef VIEWPSGWRITES
          Serial.print(" psg write: ");
          Serial.print(*vgmptr, HEX);
#endif
          vgmptr++;
          break;
        case 0x61: // 0x61 nn nn : Wait n samples, n can range from 0 to 65535
          vgmptr++;
          vgmWait = (uint16_t)( *vgmptr & 0x00FF );
          vgmptr++;
          vgmWait |= (uint16_t)((*vgmptr << 8) & 0xFF00 );
          vgmptr++;
#ifdef VIEWPSGWRITES
          Serial.print(" wait: ");
          Serial.print(vgmWait, DEC);
#endif
          doneFrame = true;
          break;
          
        case 0x62: // wait 735 samples (60th of a second)
          vgmWait = ONE60TH;
          vgmptr++;
#ifdef VIEWPSGWRITES
          Serial.print(" wait: 1/60");
#endif
          doneFrame = true;
          break;
          
        case 0x63: // wait 882 samples (50th of a second)
          vgmWait = ONE50TH;
          vgmptr++;
#ifdef VIEWPSGWRITES
          Serial.print(" wait: 1/50");
#endif
          doneFrame = true;
          break;
          
        case 0x70: // 0x7n : wait n+1 samples, n can range from 0 to 15
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
          vgmWait = (ONESAMPLE * (*vgmptr & 0x0f));
          vgmptr++;
#ifdef VIEWPSGWRITES
          Serial.print(" wait: ");
          Serial.print(vgmWait, DEC);
#endif
          doneFrame = true;
          break;
          
        case 0x66: // 0x66 : end of sound data
          vgmptr = &TitleScreen[0x40];
#ifdef VIEWPSGWRITES
          Serial.print("song over");
#endif

          doneFrame = true;
          break;
          
        default:
          break;
      } //end switch
    } //end while
    Serial.println(" frame end");
  }
}



