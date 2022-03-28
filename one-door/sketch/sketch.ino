// fridge-door-alarm with one door

// ++++++++++ config ++++++++++
// +++++ pin config +++++
const byte switchPin = 2;
const byte greenLedPin = 3;
const byte yellowLedPin = 4;
const byte redLedPin = 5;

const byte buzzerPin = 6;

const byte buttonPin = 7;
// ----- pin config -----

//FEATURE add alarm interval

const int alarmThreshold = 120000;// [ms]
const int muteTimeout = 300000;// [ms]

const int intervalNormal = 120000;// [ms]
const int intervalDebug = 1000;// interval for debug mode and open door [ms]
const bool debugDefault = true;

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
unsigned long openSince = CLOSED;
bool mute = false;
unsigned long muteSince = 0;

void setup() {
  pinMode(switchPin, INPUT);

  pinMode(greenLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

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

  byte switchReading = digitalRead(switchPin);
  handleSwitchReading(switchReading);

  if(muteSince > 0 && millis() - muteSince >= muteTimeout) {
    mute = false;
  }

  delay();
}


void handleButtonPress() {
  Serial.println("handleButtonPress");

  if(openSince == CLOSED) {
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


void handleSwitchReading(byte switchReading) {
  if(switchReading == HIGH) {
    Serial.println("door open");// or switch disconnected
    handleOpenDoor();
  } else {
    Serial.println("door closed");
    handleClosedDoor();
  }
}


void handleClosedDoor() {
  digitalWrite(greenLedPin, debug ? HIGH : LOW);
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, LOW);

  openSince = CLOSED;
  mute = false;
  muteSince = 0;
}


void handleOpenDoor() {
  Serial.print("openSince: ");Serial.println(openSince);
  unsigned long time = millis();
  Serial.println("time: " + String(time));
  digitalWrite(greenLedPin, LOW);

  if(openSince == CLOSED) {
    openSince = time;
  }

  unsigned long openDuration = time - openSince;
  Serial.println("door open for " + String(openDuration) + " (alarm threshold: " + alarmThreshold + ")");

  if(openDuration >= alarmThreshold) {
    handleAlarm();
  } else {
    digitalWrite(yellowLedPin, HIGH);
    digitalWrite(redLedPin, LOW);
  }
}


void handleAlarm() {
  Serial.println("alarm");
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

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
  long d = debug ? intervalDebug : openSince != CLOSED ? intervalDebug : intervalNormal;
  Serial.println("delay: " + String(d));
  delay(d);
}
