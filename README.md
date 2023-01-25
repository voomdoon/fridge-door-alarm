# fridge-door-alarm

Fridge door alarm using Arduino and reed switch.


## Remarks

* signaling of door state using LEDs
  * **green:** door is closed
  * **yellow:** door is open, but threshold for alarm not hit
  * **red:** door is open for long time
* two modes
  * **normal**
    * huge interval to save energy
    * green LED disabled to save energy
  * **debug**
    * short interval to debug the behavior
    * green LED enabled
* single button
  * **green:** toggle between normal and debug mode
  * **red** (and yellow): mute alarm
  * hint: for button presses you need to hold down the button until next interval
    * there is a tone if the button press is accepted, then you should release the button to avoid further press handing
* mute times out after configured time elapsed


## Roadmap

* add wiring diagram
* reduce power consumption
  * disable power LED
  * use deep sleep
* use switch for debug mode
* add blink of green LED in normal mode if door is closed
