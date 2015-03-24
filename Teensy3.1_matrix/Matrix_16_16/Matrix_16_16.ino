// Maurin Donneaud
// 16x16 e-textile matrix for Teensy 3.1
// USB and STANDALONE mode
// licence GPL
// Audio lib from pjrc : http://www.pjrc.com/teensy/td_libs_Audio.html
// USB MIDI (Musical Instrument Digital Interface) device : https://www.pjrc.com/teensy/td_midi.html
// USB is always 12 Mbit/sec

#define USB
// #define STANDALONE
// #define MIDI_USB

#if defined( STANDALONE )
 #include <Audio.h>
 #include <Wire.h>
 #include <SPI.h>
 #include <SD.h>
#endif

#define  ROW              16
#define  COLUMN           16
#define  THRESHOLD        330
#define  DEBOUNCE_TIME    100
#define  LED_PIN          13
#define  BUTTON_PIN       32
#define  FRAME_RATE       10
#define  FOOTER           255
#define  HEADER           65

// Dig pins array
// BUG FIX : strap to 33
const int rowPins[ROW] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};

// Analog pins array 
const int columnPins[COLUMN] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

int values[ROW][COLUMN];
long lastSensTime[ROW][COLUMN];
boolean toggel[ROW][COLUMN];

boolean DEBUG_MATRIX = false;
boolean DEBUG_SWITCHS = false;
boolean DEBUG_SYNTH = false;
boolean DEBUG_TRANSMIT_USB = false;
boolean DEBUG_MIDI_USB = false;

#if defined( STANDALONE )

#include "Freq.h"

#define  SYNTH    8         // Synthesizers

typedef struct wavedef {
  AudioSynthWaveform *waveform;
  float pulseW;
  float vol;
  float freq;
  short int waveType;
};

typedef struct envdef {
  AudioEffectEnvelope *envelope;
  float att;
  float hol;
  float dec;
  float sus;
  float rel;
};

typedef struct synth {
  boolean state;              // true:free - false:busy
  boolean toggelPlay;         // true:free - false:busy
  int sensorID;               // 0 to 255 (16_16 matrix)
  unsigned int sensorValue;   // 1 to 1024
  int metro;                  // time betwin two notes in reapeat mode
  int timeON;                 // tim of a mote
  unsigned long lastPlayTime; // 
  struct wavedef wave;
  struct envdef env;
};

AudioSynthWaveform waveform1, waveform2, waveform3, waveform4;
AudioSynthWaveform waveform5, waveform6, waveform7, waveform8;
AudioEffectEnvelope envelope1, envelope2, envelope3, envelope4;
AudioEffectEnvelope envelope5, envelope6, envelope7, envelope8;

synth allSynth[SYNTH] = {
 { true, true, -1, 0, 2000, 500, 0, { &waveform1, 0.5, 0.25, 440, 0 }, { &envelope1, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform2, 0.5, 0.25, 440, 1 }, { &envelope2, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform3, 0.5, 0.25, 440, 2 }, { &envelope3, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform4, 0.5, 0.25, 440, 3 }, { &envelope4, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform5, 0.5, 0.25, 440, 4 }, { &envelope5, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform6, 0.5, 0.25, 440, 5 }, { &envelope6, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform7, 0.5, 0.25, 440, 0 }, { &envelope7, 10, 2.1, 15, 6, 500 } },
 { true, true, -1, 0, 2000, 500, 0, { &waveform8, 0.5, 0.25, 440, 1 }, { &envelope8, 10, 2.1, 15, 6, 500 } }
};

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioMixer4  mixer1;
AudioMixer4  mixer2;
AudioMixer4  mixer3;
AudioOutputAnalog  DAC1;

AudioConnection  patchCord1( waveform1, envelope1 );
AudioConnection  patchCord2( waveform2, envelope2 );
AudioConnection  patchCord3( waveform3, envelope3 );
AudioConnection  patchCord4( waveform4, envelope4 );
AudioConnection  patchCord5( waveform5, envelope5 );
AudioConnection  patchCord6( waveform6, envelope6 );
AudioConnection  patchCord7( waveform7, envelope7 );
AudioConnection  patchCord8( waveform8, envelope8 );

AudioConnection  patchCord9( envelope1, 0, mixer1, 0 );
AudioConnection  patchCord10( envelope2, 0, mixer1, 1 );
AudioConnection  patchCord11( envelope3, 0, mixer1, 2 ); 
AudioConnection  patchCord12( envelope4, 0, mixer1, 3 );
AudioConnection  patchCord13( envelope5, 0, mixer2, 0 );
AudioConnection  patchCord14( envelope6, 0, mixer2, 1 );
AudioConnection  patchCord15( envelope7, 0, mixer2, 2 );
AudioConnection  patchCord16( envelope8, 0, mixer2, 3 );

AudioConnection  patchCord17( mixer1, 0, mixer3, 0 );
AudioConnection  patchCord18( mixer2, 0, mixer3, 1 );
AudioConnection  patchCord19( mixer3, DAC1 );

int AudioMemoryVal = 0;

#endif // STANDALONE

boolean communicationState = false;
boolean ledState = false;
unsigned long lastFrameTime = 0;
byte incomingByte = 0xFF; // 255

/////////////////////// INITIALISATION
void setup(){
  
  analogReadRes( 10 );

  pinMode(LED_PIN, OUTPUT);              // Set rows pins in high-impedance state
  pinMode(BUTTON_PIN, INPUT_PULLUP);     // Set rows pins in high-impedance state

  for( int i=0; i<ROW; i++ ) {
    pinMode(rowPins[ROW], INPUT);        // Set rows pins in high-impedance state
  }

#if defined(STANDALONE)
  // Allocate audio memory samples blocks
  AudioMemory(10); // Use emoryUsage to set the value

  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 0.25);

  mixer2.gain(0, 0.25);
  mixer2.gain(1, 0.25);
  mixer2.gain(2, 0.25);
  mixer2.gain(3, 0.25);

  mixer3.gain(0, 0.50);
  mixer3.gain(1, 0.50);
  mixer3.gain(2, 0);
  mixer3.gain(3, 0);

  // Init all synthetizers
  for( int i=0; i<SYNTH; i++ ) {
    allSynth[i].timeON = allSynth[i].env.att + allSynth[i].env.hol + allSynth[i].env.dec + allSynth[i].env.sus;
    allSynth[i].wave.waveform -> amplitude( allSynth[i].wave.vol );
    allSynth[i].wave.waveform -> pulseWidth( allSynth[i].wave.pulseW );
    allSynth[i].env.envelope -> attack( allSynth[i].env.att );
    allSynth[i].env.envelope -> hold( allSynth[i].env.hol );
    allSynth[i].env.envelope -> decay( allSynth[i].env.dec );
    allSynth[i].env.envelope -> sustain( allSynth[i].env.sus );
    allSynth[i].env.envelope -> release( allSynth[i].env.rel ); 
  }
#endif

  blinkState();

}

//////////////////////////////////////////////////////////// BOUCLE PRINCIPALE
void loop(){
  
  resetButton();
  
#if defined( USB )
  UsbTransmit();
#endif

#if defined( STANDALONE )
  scanStandaloneMatrix();
  updateStandaloneSynth();
  // memoryUsage();
#endif

#if defined( MIDI_USB )
  UsbMidiTransmit();
#endif

}

/////////////////////// USB TRANSMIT
#if defined( USB )
void UsbTransmit(){
  
   if( Serial.available() ) {
    incomingByte = Serial.read();  // will not be -1
    if( incomingByte == HEADER ){
      communicationState = true;
      digitalWrite(LED_PIN, HIGH);
    }
  }
  
  if( ( millis() - lastFrameTime ) >= FRAME_RATE && communicationState == true){
    lastFrameTime = millis();
    
    for( int row=0; row<ROW; row++ ){
      // Set row pin as output +3V
      pinMode( rowPins[row], OUTPUT );
      digitalWrite( rowPins[row], HIGH );

      for( int column=0; column<COLUMN; column++ ){
        int value = analogRead( columnPins[column] );
        if( !DEBUG_TRANSMIT_USB ) Serial.write( value & B01111111 );          // lowByte
        if( !DEBUG_TRANSMIT_USB ) Serial.write( (value >> 7) & B00000111 );   // highByte
        if( DEBUG_TRANSMIT_USB ) Serial.print("*"), Serial.print(" "); 
      }
      // Set row pin in high-impedance state
      pinMode(rowPins[row], INPUT);
      if( DEBUG_TRANSMIT_USB ) Serial.println();
    }
    if( !DEBUG_TRANSMIT_USB ) Serial.write( FOOTER ); // footer
    if( DEBUG_TRANSMIT_USB ) Serial.println();
  }
}
#endif


/////////////////////// USB TRANSMIT
#if defined( MIDI_USB )
void UsbMidiTransmit(){
  
  const int channel = 1;

  for( int row=0; row<ROW; row++ ){

    // Set row pin as output HIGH (+3V)
    pinMode( rowPins[row], OUTPUT );
    digitalWrite( rowPins[row], HIGH );

    for( int column=0; column<COLUMN; column++ ){
      int sensorID = row * COLUMN + column + 1;

      values[row][column] = analogRead( columnPins[column] );

      // If a sensor is touched 
      if( values[row][column] > THRESHOLD && toggel[row][column] == true && ( lastSensTime[row][column] - millis() ) > DEBOUNCE_TIME ){
        lastSensTime[row][column] = millis();
        toggel[row][column] = false; // toggel is false : KEY IN BUSY
        usbMIDI.sendNoteOn(61, 99, channel);  // 61 = C#4
        if( DEBUG_MIDI_USB  ) Serial.print( sensorID ), Serial.print(" "), Serial.println("NOTE_ON");
      }

      // Release the MIDI note
      if( values[row][column] <= THRESHOLD && toggel[row][column] == false ){
        toggel[row][column] = true; // toggel is true : KEY IN FREE
        usbMIDI.sendNoteOff(61, 0, channel);  // 60 = C4
        if( DEBUG_MIDI_USB ) Serial.print( sensorID ), Serial.print(" "), Serial.println("NOTE_OFF");
      }
    }
    // Set row pin in high-impedance state
    pinMode( rowPins[row], INPUT );
  }
}
#endif


/////////////////////// SEQ
#if defined( STANDALONE )
void updateStandaloneSynth(){

  for( int synthIndex=0; synthIndex<SYNTH; synthIndex++ ){

    // volume control
    byte column = ( allSynth[synthIndex].sensorID - 1 ) % 16;
    byte row = (int) ( ( allSynth[synthIndex].sensorID - 1 ) / 16 );
    float velocity = values[row][column] / 1024;

    // allSynth[synthIndex].wave.waveform -> amplitude( velocity ); // DO NOT WORK !?

    // PLAY
    // if( ( millis() - allSynth[synthIndex].lastPlayTime ) >= allSynth[synthIndex].metro && allSynth[synthIndex].state == false && allSynth[synthIndex].toggelPlay == false ){
    if( allSynth[synthIndex].state == false && allSynth[synthIndex].toggelPlay == false ){  // state is false == BUSY_SYNTH
      allSynth[synthIndex].toggelPlay = true;
      allSynth[synthIndex].wave.waveform -> begin( allSynth[synthIndex].wave.vol, allSynth[synthIndex].wave.freq, allSynth[synthIndex].wave.waveType ); // WAVEFORM_SINE
      allSynth[synthIndex].env.envelope -> noteOn();
    }

    // STOP
    // if( ( millis() - allSynth[synthIndex].lastPlayTime ) >= ( allSynth[synthIndex].metro + allSynth[synthIndex].timeON ) &&
    // allSynth[synthIndex].state == true && allSynth[synthIndex].toggelPlay == true ){
    if( allSynth[synthIndex].state == true && allSynth[synthIndex].toggelPlay == true ){ // state is true == FREE_SYNTH
      allSynth[synthIndex].toggelPlay = false;
      allSynth[synthIndex].env.envelope -> noteOff();
    }
  }
}
#endif

/////////////////////// SCAN MATRIX
#if defined( STANDALONE )
void scanStandaloneMatrix() {

  for( int row=0; row<ROW; row++ ) {

    // Set row pin as output HIGH (+3V)
    pinMode( rowPins[row], OUTPUT );
    digitalWrite( rowPins[row], HIGH ); // External PULL_DOWN

    for( int column=0; column<COLUMN; column++ ){
      int sensorID = row * COLUMN + column + 1;

      values[row][column] = analogRead( columnPins[column] );

      if( DEBUG_MATRIX ) Serial.print( values[row][column] ), Serial.print("\t");

      // If a sensor is touched
      if( values[row][column] > THRESHOLD && toggel[row][column] == true && ( lastSensTime[row][column] - millis() ) > DEBOUNCE_TIME ){
        lastSensTime[row][column] = millis();
        toggel[row][column] = false; // toggel is false : KEY IN BUSY

        if( DEBUG_SWITCHS ) Serial.print( sensorID ), Serial.print(" "), Serial.println( "ON" );

        // Select a free synthesizer
        for( int i=0; i<SYNTH; i++ ){
          if( allSynth[i].state == true ){                           // State is true : synth is FREE
            allSynth[i].state = false;                               // State is false : synth is BUSY
            allSynth[i].sensorID = sensorID;                         // Remind the sensor id
            allSynth[i].wave.freq = freq[row][column];               // Set the synthesizer frequancy
            // allSynth[i].wave.waveType = wave[row][column];        // Set the synthesizer wave type
            allSynth[i].wave.waveType = 1;                           // Set the synthesizer wave type
            if( DEBUG_SYNTH ) Serial.print( i ), Serial.print(" "), Serial.println( "BUSY" );
            break;
          }
        }
      }

      // Release the synthesizer
      if( values[row][column] <= THRESHOLD && toggel[row][column] == false ){
        toggel[row][column] = true; // toggel is true : KEY IN FREE

        // Clear the synth
        for( int i=0; i<SYNTH; i++){
          if( allSynth[i].sensorID == sensorID ){
            if( DEBUG_SYNTH ) Serial.print( i ), Serial.print(" "), Serial.println( "FREE" );
            allSynth[i].sensorID = -1;
            allSynth[i].state = true; // state is true : SYNTH IS FREE
            break;
          }
        }
        if( DEBUG_SWITCHS ) Serial.print( sensorID ), Serial.print(" "), Serial.println( "OFF" );
      }
    }
    // Set row pin in high-impedance state
    pinMode( rowPins[row], INPUT );
    if( DEBUG_MATRIX ) Serial.println();
  }
}

//////////////////////////////////////////////// test audio memory
void  memoryUsage(){

  AudioMemoryVal = AudioMemoryUsageMax();
  Serial.println(AudioMemoryVal);
  delay(1000);
}
#endif

//////////////////////////////////////////////// test audio memory
void  resetButton(){
  
  if( digitalRead(BUTTON_PIN) == LOW ){
    ledState = !ledState;
    communicationState = !communicationState;
    digitalWrite(LED_PIN, ledState);
    delay(500);
  }
  // delayMicroseconds(1);
}


void blinkState(){

  for( int i=0; i<15; i++ ){
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }

}

