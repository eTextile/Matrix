// '$' to twitt
// '^' to Delate
// '=' to set GE tag
// '>' to display a space
// '<' to convert letter to upper case
 
 char[][]lettersLocation = new char [][] {
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { '^', '6', '7', '8', '9', '0', '-', '+', '5', ' ', ' ', ' ', '1', '2', '3', '4' },  // 0, 1, 2, ...
 { '/', 'y', 'u', 'i', 'o', 'p', '%', '*', 't', ' ', ' ', ' ', 'q', 'w', 'e', 'r' },  // 0, 1, 2, ...
 { ' ', 'h', 'j', 'k', 'l', ':', '"', '#', 'g', ' ', ' ', ' ', 'a', 's', 'd', 'f' },  // 0, 1, 2, ...
 { '=', '>', '>', '>', ' ', '$', '$', '$', '>', ' ', ' ', ' ', ' ', ' ', ' ', '>' },  // 0, 1, 2, ...
 { ' ', 'b', 'n', 'm', ',', '.', '?', '@', 'v', ' ', '=', ' ', '<', 'z', 'x', 'c' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },  // 0, 1, 2, ...
 { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }  // 0, 1, 2, ...
};

char[] letterTag = new char[] { 
  '#', 'G', 'E', 'R', 'S', 'N', 'A', ' ', '#', 'R', 'S', 'N', 'A', '1', '1', ' '
};

/////////////////// LocalKeyboard
int STARTPOS = 0;
int MAXLETTERS = 140;
int MAXLETTERS_TAG = 16;
int letterPos = 0;

int menuXsize = 256;

int textTwitterSize = 25;
int textXyinteractionSize = 14;

int textePosX = 180;
int textePosY = 165;
int texteWidth = 500;
int texteHeight = 220;
int texteBorder = 30;

char letter;
char[] letters = new char[MAXLETTERS];
String words = new String(letters);


void keyPressed() {

  ///////////////// Local keyboard letters
  if ( (key >= '0' && key <= '9') || (key >= 'A' && key <= 'z') || (key == ' ') && letterPos < (MAXLETTERS - 1) && (key != 'G') ) {
    letters[letterPos] = key;
    letterPos++;
    refreshTexte();                 // refresh screen
  }

  ///////////////// Display Twitter GE tag
  if (key == 'G' && letterPos > 1 && letterPos < MAXLETTERS - MAXLETTERS_TAG) {
    letterPos--;
    letters[letterPos] = ' '; // space (clear)
    letterPos++;
    letters[letterPos] = ' '; // space (clear)
    for (int i=0; i<MAXLETTERS_TAG; i++) {
      letters[letterPos] = letterTag[i]; // space (clear)
      letterPos++;
    }
    refreshTexte();           // refresh screen
  }

  ///////////////// Display keyboaard frame to debug
  if ( key == 'K') {
    letterPos--;                      // decrease letterPos
    letters[letterPos] = ' ';         // clear K letter
    refreshTexte();                   // refresh screen
    refreshMenu();                    // refresh menu
    displayKeyboard = !displayKeyboard;
  }

  ///////////////// Delate
  if ( key == BACKSPACE && letterPos > STARTPOS && letterPos < (MAXLETTERS - 1)) {
    letters[letterPos] = ' ';         // space (clear)
    letterPos--;
    letters[letterPos] = ' ';         // space (clear)
    refreshTexte();                   // refresh screen
  }

  ///////////////// Send words to Twitter
  if (key == ENTER && letterPos > STARTPOS) {
    if (TWITTER) sendTweet(words);    // Send Tweet
    letterPos = STARTPOS;             // init counter letterPos
    clearKeyboardBuffer();            // clear buffer
    refreshTexte();                   // refresh screen
  }
}

///////////////// Clear Keyboard Buffer
void clearKeyboardBuffer() {
  for (int letterPos=STARTPOS; letterPos<MAXLETTERS; letterPos++) {
    letters[letterPos] = ' ';
  }
}

///////////////// Refresh texte zone
void refreshTexte() {
  textFont(myFont, textTwitterSize);  
  words = new String(letters);      // display result
  stroke(255);                      // contour bleu GE
  fill(255);                        // blank
  rect(textePosX, textePosY, texteWidth, texteHeight);
  fill(0);                          // noir
  text(words, textePosX, textePosY, texteWidth, texteHeight);
}

///////////////// Refresh menu zone
void refreshMenu() {
  stroke(58, 85, 165); // contour bleu GE
  fill(58, 85, 165);   // contour bleu GE
  rect(width - menuXsize, 0, menuXsize + 1, height - menuXsize);
}

///////////////// Blue texte frame
void GEframe() {
  stroke(58, 85, 165); // contour bleu GE
  fill(255);
  rect( textePosX - texteBorder, textePosY - texteBorder, texteWidth + (2 * texteBorder), texteHeight + (2 * texteBorder) );
}

void XYinteraction() {
  textFont(xyinteractionFont, textXyinteractionSize);
  fill(0);                          // noir
  text("Courtesy of XYinteraction", (width / 2) - 3, height - 69);
}
