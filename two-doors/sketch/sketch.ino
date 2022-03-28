// fridge-door-alarm with two doors

// ++++++++++ config ++++++++++
// +++++ pin config +++++
const byte switch1Pin = 2;
const byte greenLed1Pin = 3;
const byte yellowLed1Pin = 4;
const byte redLed1Pin = 5;

const byte switch2Pin = 8;
const byte greenLed2Pin = 9;
const byte yellowLed2Pin = 10;
const byte redLed2Pin = 11;

const byte buzzerPin = 6;

const byte buttonPin = 7;
// ----- pin config -----

//FEATURE add alarm interval

const long alarmThreshold = 120000;// known duration a door is open before firing the alarm [ms]
const long muteTimeout = 300000;// [ms]

const long intervalNormal = 120000;// [ms]
const long intervalDebug = 1000;// interval for debug mode and open door [ms]
const bool debugDefault = false;

const unsigned long buttonDebounceDelay = 50;// [ms]
// ---------- config ----------

class Button {
  private:
    int pin;
    String name;

    boolean pressed = false;
    int lastButtonState = LOW;
    unsigned long lastDebounceTime = 0;
  public:
    Button(byte pin, String name) {
      this->pin = pin;
      this->name = name;
    }

    boolean isPressed() {
      boolean result = pressed;
      pressed = false;
      lastDebounceTime = 0;

      return result;
    }

    void read() {
      int reading = digitalRead(pin);
      Serial.println("button '" + name + "' reading: " + reading + " (lastButtonState: " + lastButtonState + ")");

      if (reading != lastButtonState) {
        if ((millis() - lastDebounceTime) > buttonDebounceDelay) {
          Serial.println("new press");
          pressed = true;
        }

        lastDebounceTime = millis();
      }
    }
};

Button button = Button(buttonPin, "button");
bool debug = debugDefault;

const unsigned long CLOSED = 0;
unsigned long open1Since = CLOSED;
unsigned long open2Since = CLOSED;
bool mute = false;
unsigned long muteSince = 0;

void setup() {
  pinMode(switch1Pin, INPUT);

  pinMode(greenLed1Pin, OUTPUT);
  pinMode(yellowLed1Pin, OUTPUT);
  pinMode(redLed1Pin, OUTPUT);

  pinMode(switch2Pin, INPUT);

  pinMode(greenLed2Pin, OUTPUT);
  pinMode(yellowLed2Pin, OUTPUT);
  pinMode(redLed2Pin, OUTPUT);

  pinMode(buzzerPin, OUTPUT);

  pinMode(buttonPin, INPUT);

  Serial.begin(9600);
}


void loop() {
  Serial.println("mode: " + String(debug ? "debug" : "standard")
    + ", mute: " + (mute ? "ON" : "OFF")
    + mute ? " (for " +  String(millis() - muteSince) + "/" + String(muteTimeout)+ ")" : "");

  button.read();

  if(button.isPressed()) {
    handleButtonPress();
  }

  byte switch1Reading = digitalRead(switch1Pin);
  byte switch2Reading = digitalRead(switch2Pin);
  handleSwitchReadings(switch1Reading, switch2Reading);

  if(muteSince > 0 && millis() - muteSince >= muteTimeout) {
    mute = false;
  }

  long d = debug ? intervalDebug : (open1Since != CLOSED || open2Since != CLOSED)? intervalDebug : intervalNormal;
  Serial.println("delay: " + String(d));
  delay(d);
}


void handleButtonPress() {
  Serial.println("handleButtonPress");

  if(open1Since == CLOSED && open2Since == CLOSED) {
    toggleDebug();
  } else {
    Serial.print("toggle alarm mute: ");
    mute = !mute;
    Serial.println(mute ? "ON" : "OFF");

    if(mute) {
      muteSince = millis();
    } else {
      muteSince = 0;
    }
  }

  toneAccept(buzzerPin);
}    


void handleSwitchReadings(byte switch1Reading, byte switch2Reading) {
  handleSwitch1Reading(switch1Reading);
  handleSwitch2Reading(switch2Reading);

  if(switch1Reading == LOW && switch2Reading == LOW) {
    mute = false;
    muteSince = 0;
  }
}


void handleSwitch1Reading(byte switchReading) {
  if(switchReading == HIGH) {
    Serial.println("door 1 open");// or switch disconnected
    handleOpenDoor1();
  } else {
    Serial.println("door 1 closed");
    handleClosedDoor1();
  }
}


void handleSwitch2Reading(byte switchReading) {
  if(switchReading == HIGH) {
    Serial.println("door 2 open");// or switch disconnected
    handleOpenDoor2();
  } else {
    Serial.println("door 2 closed");
    handleClosedDoor2();
  }
}


void handleClosedDoor1() {
  Serial.println("handleClosedDoor1");
  digitalWrite(greenLed1Pin, debug ? HIGH : LOW);
  digitalWrite(yellowLed1Pin, LOW);
  digitalWrite(redLed1Pin, LOW);

  open1Since = CLOSED;
}


void handleClosedDoor2() {
  Serial.println("handleClosedDoor2");
  digitalWrite(greenLed2Pin, debug ? HIGH : LOW);
  digitalWrite(yellowLed2Pin, LOW);
  digitalWrite(redLed2Pin, LOW);

  open2Since = CLOSED;
}


void handleOpenDoor1() {
  Serial.print("open1Since: ");Serial.println(open1Since);
  unsigned long time = millis();
  Serial.println("time: " + String(time));
  digitalWrite(greenLed1Pin, LOW);

  if(open1Since == CLOSED) {
    open1Since = time;
  }

  unsigned long openDuration = time - open1Since;
  Serial.println("door 1 open for " + String(openDuration) + " (alarm threshold: " + alarmThreshold + ")");

  if(openDuration >= alarmThreshold) {
    handleAlarm1();
  } else {
    digitalWrite(yellowLed1Pin, HIGH);
    digitalWrite(redLed1Pin, LOW);
  }
}


void handleOpenDoor2() {
  Serial.print("open2Since: ");Serial.println(open1Since);
  unsigned long time = millis();
  Serial.println("time: " + String(time));
  digitalWrite(greenLed2Pin, LOW);

  if(open2Since == CLOSED) {
    open2Since = time;
  }

  unsigned long openDuration = time - open2Since;
  Serial.println("door 2 open for " + String(openDuration) + " (alarm threshold: " + alarmThreshold + ")");

  if(openDuration >= alarmThreshold) {
    handleAlarm2();
  } else {
    digitalWrite(yellowLed2Pin, HIGH);
    digitalWrite(redLed2Pin, LOW);
  }
}


void handleAlarm1() {
  Serial.println("alarm1");
  digitalWrite(yellowLed1Pin, LOW);
  digitalWrite(redLed1Pin, HIGH);

  if(!mute) {
    toneAlarm();
  }
}


void handleAlarm2() {
  Serial.println("alarm2");
  digitalWrite(yellowLed2Pin, LOW);
  digitalWrite(redLed2Pin, HIGH);

  if(!mute) {
    toneAlarm();
  }
}


void toggleDebug() {
  debug = !debug;
  Serial.println("mode switch to: " + debug ? "debug" : "standard");
}


void toneAlarm() {
  tone(buzzerPin, 1000, 100);
}

void toneAccept(byte buzzerPin) {
    tone(buzzerPin, 2000);
    delay(50);
    noTone(buzzerPin);
    delay(50);
    tone(buzzerPin, 2000);
    delay(50);
    noTone(buzzerPin);
}


void delay() {
  long d = debug ? intervalDebug : (open1Since != CLOSED || open2Since != CLOSED) ? intervalDebug : intervalNormal;
  Serial.println("delay: " + String(d));
  delay(d);
}
