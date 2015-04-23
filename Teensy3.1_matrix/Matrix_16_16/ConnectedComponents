//////////////////// algorithm TwoPass
#define  LED_PIN          13
#define  COLS             18   // 16 + 2 for an array right and left ofset 
#define  ROWS             18   // 16 + 2 for an array right and left ofset
#define  THRESHOLD        150  // Threshold to 
#define  DEBOUNCE_TIME    80   // 

int rawSensorValue[ROWS][COLS];
boolean toggel[ROWS][COLS];

long lastSensTime[ROWS][COLS];
short int label[ROWS][COLS];
short int lastLabel[ROWS][COLS];

// 000000000 00000000
// 011111111 11111111
// 011111111 11111111
// 011111111 11111111
// 011111111 11111111
// ...

// Dig pins array
// BUG FIXED : strap to 33
const int rowPins[ROWS-1] = {
  27, 26, 25, 24, 12, 11, 10, 9, 8, 7, 6, 5, 33, 2, 1, 0
};
// Analog pins array
const int colPins[COLS-1] = {
  A17, A18, A19, A0, A20, A1, A2, A3, A4, A5, A6, A7, A11, A8, A10, A9
};

///////////////////////////////////////////////////// SETUP
void setup(){

  pinMode(LED_PIN, OUTPUT);              // Set rows pins in high-impedance state

  for( int row=0; row<ROWS; row++ ){
    for( int col=0; col<COLS; col++ ){
      rawSensorValue[row][col] = 0;
      lastSensTime[row][col] = 0;
      toggel[row][col] = false;
      label[row][col] = 0;
    }
  }
  blinkState();
}

///////////////////////////////////////////////////// LOOP
void loop(){

  scanMatrix();
  connectedComponentLabeling();
}

/////////////////////// Scan the e-textil matrix
void scanMatrix(){

  for( int row=1; row<ROWS-1; row++ ){

    // Set row pin as output HIGH (+3.3V)
    pinMode( rowPins[row-1], OUTPUT );
    digitalWrite( rowPins[row-1], HIGH );

    for( int col=1; col<COLS-1; col++ ){
      rawSensorValue[row][col] = analogRead( colPins[col-1] );

      // Is touched
      if( rawSensorValue[row][col] >= THRESHOLD && toggel[row][col] == false && ( lastSensTime[row][col] - millis() ) > DEBOUNCE_TIME ){
        toggel[row][col] = true;
        lastSensTime[row][col] = millis();
      }
      // Is released
      if( rawSensorValue[row][col] < THRESHOLD && toggel[row][col] == true ){
        toggel[row][col] = false;
      }
    }
    // Set row pin in high-impedance state
    pinMode( rowPins[row-1], INPUT );
  }
}


/////////////////////// Set label on connected components
void connectedComponentLabeling(){

  int curentLabel = 0;

  //////////////////// First pass
  for( int row=1; row<ROWS-1; row++ ){
    for( int col=1; col<COLS-1; col++ ){

      if( toggel[row][col] == true ){                                       // if the curent pixel is forground
        // if( toggel[row][col-1] == false ){
        if( toggel[row][col-1] == false && toggel[row-1][col] == false ){   // If the left and top neighbor pixels are background
          curentLabel++;                                                    // increse the curent label
        }
        
        lastLabel[row][col] = label[row][col];                              // Remember the last pixel label
        label[row][col] = curentLabel;                                      // Get the curent pixel label
        
        if( curentLabel != lastLabel[row][col] ){                           // 
          label[row][col] = min( lastLabel[row][col], curentLabel );        //
        }
        
        if( label[row-1][col] != 0 ){                                       // If the top neighbor pixel is smaler than the curent label
          label[row][col] = min( label[row-1][col], curentLabel );          // Take the right neighbor label pixiel value
        }
        if( label[row][col-1] != 0 ){                                       // If the right neighbor pixel is smaler than the curent label
          label[row][col] = min( label[row][col-1], curentLabel );          // Take the right neighbor label pixel value
        }
      }
      else {                                                                // If the curent pixel is background
        label[row][col] = 0;                                                // Raz pixel label value
      }
    }
  }
  Serial.println(" First pass ");
  printOut();

  //////////////////// Second pass
  for( int row=ROWS-1; row>0; row-- ){
    for( int col=COLS-1; col>0; col-- ){
      if( label[row][col] != 0 ){                                      // If the pixel label value in different than 0
        if( label[row][col+1] > label[row][col] ){                     // If the right neighbor pixel is smaler than the curent label
          label[row][col] = label[row][col+1];                         // Take the right neighbor pixel label value
        }
        if( label[row+1][col] > label[row][col] ){                     // If the bottom neighbor pixel label is smaler than the curent pixel label
          label[row][col] = label[row+1][col];                         // Take the bottom neighbor label pixel value
        }
      }
    }
  }
  Serial.println(" Second pass ");
  printOut();
}

/////////////////////// USB DEBUG
void printOut(){

  for( int row=1; row<ROWS-1; row++ ){
    for( int col=1; col<COLS-1; col++ ){
      if( toggel[row][col] == true ){
        // Serial.print( rawSensorValue[row][col] ), Serial.print( "\t" );
        // Serial.print( toggel[row][col] ), Serial.print( "\t" );
        Serial.print( label[row][col] ), Serial.print( "\t" );
      } 
      else {
        Serial.print( "\t" );
      }
    }
    Serial.println();
  }
  Serial.println();
}

/////////////////////// Blink LED
void blinkState(){

  for( int i=0; i<15; i++ ){
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
}
