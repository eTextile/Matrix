// Matrix shield V1.1 //

#define outLatchPin    10      //  pin connected to latch pin (ST_CP) of HC595 8-BIT shift register
#define outClockPin    A0      //  pin connected to clock pin (SH_CP) of HC595
#define outDataPin     13      //  pin connected to Data in (DS) of 74HC595

// Multiplexer 74HC4067 control pins
// four bits to adress 16 lignes 
#define s0_pin         A4      //
#define s1_pin         A3      //
#define s2_pin         A1      //
#define s3_pin         A2      //

const int controlPin[] = {
    s0_pin,
    s1_pin,
    s2_pin,
    s3_pin
};

#define sig_pin        A5      // multiplexer output "SIG" pin is connected to Arduino Analog pin 5

int analogValue = 0;           // variable use to store analog inputs values
byte value[16][16];            // 256 bytes to hold 256 analog valeus
unsigned int columnBitPos = 0; // variable to store column bit position

boolean DEBUG = false;          // true for arduino debuging, false for processing.

////////////////////////////////////// SETUP
void setup(){
  // set up Serial library at 115200 bauds
  Serial.begin(19200);
  // Set output shift registers pins
  pinMode(outLatchPin, OUTPUT);
  pinMode(outClockPin, OUTPUT);
  pinMode(outDataPin, OUTPUT);
  // set analog mux output pins control 
  pinMode(s0_pin, OUTPUT);
  pinMode(s1_pin, OUTPUT);
  pinMode(s2_pin, OUTPUT);
  pinMode(s3_pin, OUTPUT);
  // init analog mux output pins
  digitalWrite(s0_pin, LOW);
  digitalWrite(s1_pin, LOW);
  digitalWrite(s2_pin, LOW);
  digitalWrite(s3_pin, LOW);
}

////////////////////////////////////// LOOP

void loop(){
  scanMatrix(); // scan all 256 textile matrix intersections
  transmit(); // serial to Processing
}

///////////// Scan the 16x16 matrix sensors
void scanMatrix() {

  for(int row=0; row<16; row++){
    registerWrite(row, outDataPin, outClockPin, outLatchPin);
    readMux(row); //
  }
}

///////////// write HIGH to an output column 
void registerWrite(unsigned int whichColumn, int whichDataPin, int whichClockPin, int whichLatchPin){
  
  unsigned int outputVal = 0;
  // array to set matrix pins order
  unsigned int bitPos[16]={
    0x100,  // 8
    0x200,  // 7 
    0x400,  // 6
    0x800,  // 5
    0x1000, // 4
    0x2000, // 3
    0x4000, // 2
    0x80,   // 9
    0x1,    // 16
    0x2,    // 15
    0x4,    // 14
    0x8,    // 13
    0x10,   // 12 
    0x20,   // 11
    0x40,   // 10
    0x8000  // 1
  };

  outputVal = bitPos[whichColumn];

  // set latchPin low
  digitalWrite(whichLatchPin, LOW);
  //shift out highbyte
  shiftOut(whichDataPin, whichClockPin, LSBFIRST, outputVal);
  // shift out lowbyte
  shiftOut(whichDataPin, whichClockPin, LSBFIRST, (outputVal >> 8));
  // set latchPin high
  digitalWrite(whichLatchPin, HIGH);
}

///////////// fonction to read 16 matrix columns
int readMux(int whichRow){

  const int muxChannels[16][4]={
   { 1, 1, 1, 0 }, //channel 7
   { 0, 1, 1, 0 }, //channel 6
   { 1, 0, 1, 0 }, //channel 5
   { 0, 0, 1, 0 }, //channel 4
   { 1, 1, 0, 0 }, //channel 3
   { 0, 1, 0, 0 }, //channel 2
   { 1, 0, 0, 0 }, //channel 1
   { 0, 0, 0, 0 }, //channel 0
   { 1, 1, 1, 1 }, //channel 15
   { 0, 1, 1, 1 }, //channel 14
   { 1, 0, 1, 1 }, //channel 13
   { 0, 0, 1, 1 }, //channel 12
   { 1, 1, 0, 1 }, //channel 11
   { 0, 1, 0, 1 }, //channel 10
   { 1, 0, 0, 1 }, //channel 9
   { 0, 0, 0, 1 }  //channel 8
   };

  for(int column=0; column<16; column++){ // two byte by ligne
    // configure which mux analog pin to read
    for(int i=0; i<4; i++){
      digitalWrite(controlPin[i], muxChannels[column][i]);
    }
    
    delayMicroseconds(100);                            // delay to slow down analog readings
    analogValue = analogRead(sig_pin);                 // read an analog input row
    value[whichRow][column] = (byte) (analogValue/4);  // 256 byte to hold 256 analogs valeus
  }
}

///////////// send all values to Processing 
void transmit() {

  ////////// header (16 x 255)
  for(int i=0; i<16; i++){
    if( !DEBUG ) Serial.write(255); // 0XFF header A
  };
  ////////// data
  for (int row=0; row<16; row++){
    for (int column=0; column<16; column++) {
      if( !DEBUG ) Serial.write(value[row][column]);
      if( DEBUG ) Serial.print(value[row][column]), Serial.print("\t");
    }
    if( DEBUG ) Serial.println();
  }
  if( DEBUG ) Serial.println();
}

