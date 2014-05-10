////////// Customiz the menu
void customize(DropdownList ddl) {
  ddl.setBackgroundColor(255);
  ddl.setItemHeight(25);
  ddl.setBarHeight(20);
  ddl.captionLabel().set("Serial Port");
  ddl.captionLabel().style().marginTop = 3;
  ddl.captionLabel().style().marginLeft = 3;
  ddl.valueLabel().style().marginTop = 3;
  for (int i=0; i<Serial.list().length; i++) {
    String portName = Serial.list()[i];
    ddl.addItem(portName, i);
  }
  ddl.setColorBackground(50);
  ddl.setColorActive(color(255, 0, 0));
}

////////// Get the port number 
void controlEvent(ControlEvent theEvent) {
  int BAUDERATE = 9600;
  int portValue = 0;

  if (theEvent.isGroup()) {
    portValue = (int) theEvent.group().value();
    String portName = Serial.list()[portValue];
    myPort = new Serial(this, portName, BAUDERATE);
    // myPort.clear();
    displayKeyboard = true;
  }
}
