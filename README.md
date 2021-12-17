# esp8266-saeco-intelia-mqtt
Control this [Philips Saeco Intelia coffee maker](https://www.philips.de/c-p/HD8751_11/saeco-intelia-kaffeevollautomat) with an ESP8266 and MQTT (via openHAB and Google Assistant).

![Coffee machine](https://images.philips.com/is/image/PhilipsConsumer/HD8751_11-IMS-de_DE?wid=494&hei=435&$pnglarge$)

## Objective & inspiration

Integrate my Saeco coffee maker into [openHAB](https://openhab.org) to control it with MQTT and Google Assistant voice commands.

I wanted to be able to:
* Switch coffee maker on / off
* Read current status (on / off / ready / warning)
* Trigger espresso button
* Trigger large coffee button
* Trigger tea water button
* Communicate via MQTT for easy integration with openHAB

I was inspired by [OpHaCo's smart coffee maker project](https://github.com/OpHaCo/smart_coffee_machine) and [bntdumas's "Add WiFi to your coffee machine" tutorial](http://www.bntdumas.com/2015/07/15/how-to-add-wifi-to-your-coffee-machine-part-1/) but have a slightly different model and wanted to implement it in Arduino code, not LUA.

## What you need

* Philips Saeco Intelia coffee maker
* ESP8266 (I use model E12 on the Wemos D1 Mini Lite breakout board)
* 2 x 1 kΩ resistor to read background led color of display
* 4 x 2N2222A transistor (1 for every button we want to trigger)
* 4 x 1 kΩ resistor (1 for every button we want to trigger)
* A prototype board, basic wire etc.

## Building the board

![Coffee machine control board](https://github.com/nicolaus-hee/esp8266-saeco-intelia-mqtt/blob/master/images/coffee_machine_control_board.jpg)
Coffee machine control board (before wiring it up)

![Prototype board (without ESP8266)](https://github.com/nicolaus-hee/esp8266-saeco-intelia-mqtt/blob/master/images/prototype_board_without_esp8266.jpg)
Prototype board (without ESP8266)

![Prototype board inside coffee machine](https://github.com/nicolaus-hee/esp8266-saeco-intelia-mqtt/blob/master/images/prototype_board_with_esp8266_in_coffee_machine.jpg)
Prototype board inside coffee machine

## What the code does

* Read current machine status (off / red / yellow / green)
* Press power, small / large coffee, tea water buttons
* Communicate status info to MQTT server
* Listen to MQTT server for commands

## MQTT integration

Topic | Payload | Comment
----- | ------- | --------
stat/coffee_maker/STATUS | {"POWER":"ON", "STATUS":"GREEN"} | Published upon changes
stat/coffee_maker/LOG | String, e.g. "switching machine on" | Published upon events
cmnd/coffee_maker/PB4 | ON, OFF | Power on/off
cmnd/coffee_maker/PB1 | (none) | Make small coffee
cmnd/coffee_maker/PB2 | (none) | Make large coffee
cmnd/coffee_maker/PB3 | (none) | Make tea water

## openHAB integration

I'm using the [MQTT binding for openHAB](https://www.openhab.org/addons/bindings/mqtt/).

### Channels & items

When all channels are added and items created, your list of channels should [look like this](https://github.com/nicolaus-hee/esp8266-saeco-intelia-mqtt/blob/master/images/mqtt_channels.JPG).

```
Label: Coffee maker status
MQTT state topic: stat/coffee_maker/STATUS
Incoming value transformation: JSONPATH:$.STATUS
Item type: String
Item id: CoffeeMaker_CoffeeMakerStatus

Label: On / Off
MQTT state topic: stat/coffee_maker/STATUS
MQTT command topic: cmnd/coffee_maker/PB4
Incoming value transformation: JSONPATH:$.POWER
Item type: Switch
Item id: CoffeeMaker_OnOff
Item meta data: Google Assistant > Coffee Maker

Label: Small coffee
MQTT state topic: stat/coffee_maker/STATUS
MQTT command topic: cmnd/coffee_maker/PB1
Incoming value transformation: JSONPATH:$.POWER
Item type: Switch
Item id: CoffeeMaker_SmallCoffee

Label: Large coffee
MQTT state topic: stat/coffee_maker/STATUS
MQTT command topic: cmnd/coffee_maker/PB2
Incoming value transformation: JSONPATH:$.POWER
Item type: Switch
Item id: CoffeeMaker_LargeCoffee

Label: Tea water
MQTT state topic: stat/coffee_maker/STATUS
MQTT command topic: cmnd/coffee_maker/PB3
Incoming value transformation: JSONPATH:$.POWER
Item type: Switch
Item id: CoffeeMaker_TeaWater

Label: Last log entry
MQTT state topic: stat/coffee_maker/LOG
Item type: String
Item id: CoffeeMaker_LastLogEntry
```

### Sitemap
```
Switch item=CoffeeMaker_OnOff label="On / off" icon="switch"
Default item=CoffeeMaker_CoffeeMakerStatus label="Status" icon="rgb"
Default item=CoffeeMaker_LastLogEntry label="Last event" icon="text"
Switch item=CoffeeMaker_SmallCoffee label="Small coffee" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]
Switch item=CoffeeMaker_LargeCoffee label="Large coffee" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]      	
Switch item=CoffeeMaker_TeaWater label="Tea water" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]
```

## Google Assistant integration

* Expose items to [openHAB cloud connector](https://www.openhab.org/docs/ecosystem/google-assistant/) (Settings > openHAB cloud > Items to expose)

### Option 1: Via IFTTT
* Create IFTTT applet
* If: Google Assistant / Say a simple phrase
* Then: openHAB / Send a command to item (Power [ON, OFF], Small coffee...)
* Your Google Assistant devices will now respond to the defined 'simple phrase'
* Advantage of IFTTT: You can simply add commands other than power on/off.

### Option 2: Via OpenHAB
* Make sure the `CoffeeMaker_Power` item has Google Assistant `Coffee_Maker` tag assigned in openHAB
* In Google Home app, add openHAB account (say "talk to openHAB")
* (If you had already done the previous step: say "refresh devices")
* You will see a coffee maker icon popup
* Your Google Assistant devices will now understand "turn on/off coffee maker" commands (but nothing else)

## What the code does not do / possible extensions

- [ ] Read specific display status messages (e.g. out of water, beans etc.)
- [ ] Skip decalcification warning
- [ ] Push cup under coffee outlet (and only after heat up flushing ;-))
- [ ] Coffee consumption statistics
 