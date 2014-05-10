/////////////////// TextilKeyboard
int state = 0;                  // start with header chek mode
int SOF = 0XFF;                 // header tag (start of frame) is 255
int posX = 0;                   // init counter posX
int posY = 0;                   // init counter posY

int incomingByte = 0;           // input value
int incomingByteIndex = 0;      // index of incomingByte Buffer
int[][]incomingByteBuffer = new int[2][16]; // 16 row of two byte

int byteToSplit = 0;            // incomming byte  
boolean bitFrame = false;       // splited incomming byte

int screenBitIndex = 0;         // 
int[]screenBitBuffer = new int[256]; // 16 * 16 = 256 boolean value

int letterPosX = 0;
int letterPosY = 0;

boolean upperCase = true;
boolean hTag = false;

/////////////////// Get serial data
void serialEvent(Serial myPort) {

  while ( myPort.available () > 0 ) {
    incomingByte = myPort.read();

    switch( state ) {

    case 0: /////////////////// Header Detection mode
      {
        if ( DEBUG_state ) print(state + ", ");
        if ( incomingByte == SOF ) {
          incomingByteIndex = 0;
          state = 1; // exit mode 1
        }
        break;
      }

    case 1: /////////////////// enregistrement des données serie sur 32 bytes (256 value)
      {
        if ( DEBUG_state ) print(state + ", ");

        posX = incomingByteIndex % 2;  // posX = 0, 1, 0, 1, 0, 1...
        if ( posX == 0 ) posY = (incomingByteIndex % 32) / 2; // posY = 0, 0, 1, 1, 2, 2, 3, 3... 15

        if ( DEBUG_bytePos ) print("posX = " + posX + ", "); // print index
        if ( DEBUG_bytePos ) println("posY = " + posY + ", "); // print index

        incomingByteBuffer[posX][posY] = incomingByte; // incomingByteBuff[2][16]

        if ( DEBUG_serial ) print("incomingByteIndex = " + incomingByteIndex + ", "); // print index
        if ( DEBUG_serial ) println("incomingByte = " + incomingByteBuffer[posX][posY] + " "); // print incomingByte

        incomingByteIndex++; // byte 0 to 31

        if ( incomingByteIndex == 31 ) { // 0 to 31
          state = 2;                     // set state 
          break;                         // exit mode 2
        } 
        break;
      }

    case 2: /////////////////// Exit serial input mode
      {
        if ( DEBUG_state ) print(state + ", ");
        state = 0; // exit mode 3 apres avoir graphé les données ouvrir l'ecoute
        incomingByteIndex = 0;
        posX = 0;
        posY = 0;
        screenBitIndex = 0;
        letterPosX = 0;
        letterPosY = 0;
        doDraw = true;
        break;
      }
    }
  }
}

void displayKeyboard() {

  if ( doDraw == true ) {

    for ( int screenPosY=0; screenPosY<16; screenPosY++ ) { // [2][16]; // 16 row of two byte
      for ( int screenPosX=0; screenPosX<2; screenPosX++ ) {

        byteToSplit = incomingByteBuffer[screenPosX][screenPosY]; // incomingByteBuff[2][16]

          if ( DEBUG_screenPos ) print("screenPosX = " + screenPosX + ", "); // print index
        if ( DEBUG_screenPos ) println("screenPosY = " + screenPosY + ", "); // print index

        for (int bitPos=7; bitPos>=0; bitPos--) {

          bitFrame = getBit(byteToSplit, bitPos);  // get bit from byteToSplit

          if ( DEBUG_bitFrame ) print("bitPos = " + bitPos + ", "); // print index
          if ( DEBUG_bitFrame ) println("bitFrame = " + bitFrame + ", "); // print index

          letterPosX = screenBitIndex % 16;        // j = 0, 1, 2, 3, 4, 5... 15
          letterPosY = screenBitIndex / 16;        // i = 0, 16, 32... 255

          drawAllKey(letterPosX, letterPosY, bitFrame); // display keyboard on the right

          if ( DEBUG_letterPos ) print("letterPosX = " + letterPosX + ", "); // print index
          if ( DEBUG_letterPos ) println("letterPosY = " + letterPosY + ", "); // print index

          displayTextileKeyboardLetter(letterPosX, letterPosY, bitFrame); /////////////// TO DO

          screenBitIndex++;
        }
      }
    }
    doDraw = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////// Display textile keyboard letters
void displayTextileKeyboardLetter( int keyXid, int keyYid, boolean statusKey ) {

  ///////////////// look if a key is pressed
  ///////////////// look if letterPos limit
  ///////////////// look if the key is ok
  if ( statusKey == true && lettersLocation[keyXid][keyYid] != ' ' && letterPos <= (MAXLETTERS - 1) ) {

    ///////////////// Delate (recive '^' letter)
    if ( lettersLocation[keyXid][keyYid] == '^' && letterPos > STARTPOS && letterPos <= (MAXLETTERS - MAXLETTERS_TAG - 1) ) {
      letters[letterPos] = ' ';         // space (clear)
      letterPos--;                      // decrease letterPos
      letters[letterPos] = ' ';         // space (clear)
      refreshTexte();                   // refresh screen
    }

    ///////////////// Space (recive '>' letter)
    if ( lettersLocation[keyXid][keyYid] == '>' && letterPos > (STARTPOS + 1) && letterPos <= (MAXLETTERS - MAXLETTERS_TAG - 1) ) {
      letters[letterPos] = ' ';         // space (clear)
      letterPos++;
      letters[letterPos] = ' ';         // space (clear)
      refreshTexte();                   // refresh screen
    }

    ///////////////// Send words to Twitter (recive '$' letter)
    if ( lettersLocation[keyXid][keyYid] == '$' && letterPos >= STARTPOS + 1 && hTag == true) {
      if (TWITTER) sendTweet(words);    // Send Tweet
      letterPos = STARTPOS;             // init counter letterPos
      clearKeyboardBuffer();            // clear buffer
      refreshTexte();                   // refresh screen
      hTag = false;
    }

    ///////////////// Display Twitter GE tag
    if ( lettersLocation[keyXid][keyYid] == '='  && letterPos <= (MAXLETTERS - MAXLETTERS_TAG - 1) ) {
      letterPos++;
      letters[letterPos] = ' '; // space (clear)
      for (int i=0; i<MAXLETTERS_TAG; i++) {
        letters[letterPos] = letterTag[i]; // space (clear)
        letterPos++;
      }
      letterPos--;              // decrease letterPos
      letters[letterPos] = ' '; // space (clear)
      refreshTexte();           // refresh screen
      hTag = true;
    }

    ///////////////// Convert next letter to upper case
    if ( lettersLocation[keyXid][keyYid] == '<' ) {
      upperCase = true;
    }

    ///////////////// All others keys
    // '$' to twitt
    // '^' to Delate
    // '=' to set GE tag
    // '>' to display a space
    // '<' to convert letter to upper case

      if (
      letterPos < (MAXLETTERS - MAXLETTERS_TAG - 1) &&
      lettersLocation[keyXid][keyYid] != '$' && 
      lettersLocation[keyXid][keyYid] != '^' && 
      lettersLocation[keyXid][keyYid] != '=' && 
      lettersLocation[keyXid][keyYid] != '>' && 
      lettersLocation[keyXid][keyYid] != '<' 
      ) {

      if ( upperCase == false ) {
        letters[letterPos] = lettersLocation[keyXid][keyYid]; // add corresponding letter to letters[] array
        letterPos++;
        refreshTexte();                   // refresh screen
      }
      if ( upperCase == true ) {
        letters[letterPos] = toupperCaseLetter(lettersLocation[keyXid][keyYid]);
        letterPos++;
        refreshTexte();                   // refresh screen
        upperCase = false;
      }
    }
  }
}

/////////////////// Get bit from Byte
boolean getBit( int myVarIn, int whatBit ) {
  boolean bitState;
  bitState = boolean( myVarIn & ( 1 << whatBit ) );
  return bitState;
}

/////////////////// Display all keys
void drawAllKey( int keyXid, int keyYid, boolean statusKey ) {
  int keyPosX = 0; // init variable keyPosX
  int keyPosY = 0; // init variable keyPosY
  int keySize = menuXsize / 16;
  int borderX = width - ( 16 * keySize );
  int borderY = height - ( 16 * keySize );

  keyPosX = ( keyXid * keySize ) + borderX;
  keyPosY = ( keyYid * keySize ) + borderY;

  if ( statusKey == false ) {
    fill( 255 ); // white
  }
  else {
    fill( 255, 0, 0 ); // red
  }
  stroke( 0 ); // contour noir
  rect( keyPosX, keyPosY, keySize, keySize );
  fill( 0 ); // typo noir
  textSize(13);

  // Display all coresponding letter 
  text( lettersLocation[keyXid][keyYid], keyPosX + 5, keyPosY + ( keySize - 3 ) );
}

/////////////////// Convert letter to upper case
char toupperCaseLetter(char inputCharLetter) {
  String upperCaseLetter = "";
  char car;

  upperCaseLetter = new String(Character.toString(inputCharLetter));
  upperCaseLetter = upperCaseLetter.toUpperCase();
  car = upperCaseLetter.charAt(0); // string to car
  return car;
}

