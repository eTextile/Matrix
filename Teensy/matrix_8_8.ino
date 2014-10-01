/////////////////////////////// MATRIX
#define ROW            8
#define COLUMN         8
#define BAUDRATE       115200

// Dig pins
const int rowPins[ROW] = {
  0, 1, 2 , 3, 4, 5, 6, 7
};
// Analog pins
const int columnPins[COLUMN] = {
  A0, A1, A2, A3, A4, A5, A6, A7
};

void setup() {
  Serial.begin(BAUDRATE);
  // Set row pin in high-impedance state
  for (int i=0; i<ROW; i++) {
    pinMode(rowPins[ROW], INPUT);
  }
}

/////////////////////// the loop
void loop() {
  for (int row=0; row<ROW; row++) {
    // Set row pin as output +3V
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);
    
    for ( int column=0; column<COLUMN; column++ ) {
      int value = analogRead( columnPins[column] );
      Serial.write( value & B01111111 );          // lowByte
      Serial.write( (value >> 7) & B00000111 );   // highByte
    }
    // Set row pin in high-impedance state
    pinMode(rowPins[row], INPUT);
  }
  Serial.write( 255 ); // footer
}
