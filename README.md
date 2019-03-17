# esp8266-saeco-intelia-mqtt
Control a Philips Saeco Intelia coffee maker with an ESP8266 and MQTT.

## Objective & inspiration

Integrate my Saeco coffee maker into Openhab to control it with MQTT and Google Assistant voice commands.

I wanted to be able to:
* Switch coffee maker on / off
* Read current status (on / off / ready / warning)
* Trigger espresso button
* Trigger large coffee button
* Trigger tea water button
* Communicate via MQTT for easy integration with Openhab

I was inspired by [OpHaCo's smart coffee maker project](https://github.com/OpHaCo/smart_coffee_machine) and [bntdumas's "Add WiFi to your coffee machine" tutorial](http://www.bntdumas.com/2015/07/15/how-to-add-wifi-to-your-coffee-machine-part-1/) but have a slightly different model and wanted to implement it in Arduino code, not LUA.

## What you need

* Philips Saeco Intelia coffee maker
* ESP8266 (I use model E12 on the Wemos D1 Mini Lite breakout board)
* 2 x 1 kΩ resistor to read background led color of display
* 4 x 2N2222A transistor (1 for every button we want to trigger)
* 4 x 1 kΩ resistor (1 for every button we want to trigger)
* A prototype board, basic wire etc.

## What the code does

* Read current machine status (off / red / yellow / green)
* Press power, small / large coffee, tea water buttons
* Communicate status info to MQTT server
* Listen to MQTT server for commands

## Openhab integration

### Channels & items

```
Label: Coffee maker status
MQTT state topic: stat/coffee_maker/STATUS
Incoming value transformation: JSONPATH:$.STATUS
Item type: String
Item id: CoffeeMaker_CoffeeMakerStatus

Label: On / Off
MQTT state topic: stat/coffee_maker/POWER
MQTT command topic: cmnd/coffee_maker/PB4
Incoming value transformation: JSONPATH:$.POWER
Item type: Switch
Item id: CoffeeMaker_OnOff

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
Default item=CoffeeMaker_LastLogEntry label="Letztes Ereignis" icon="text"
Switch item=CoffeeMaker_SmallCoffee label="Kaffee klein" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]
Switch item=CoffeeMaker_LargeCoffee label="Kaffee groß" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]      	
Switch item=CoffeeMaker_TeaWater label="Teewasser" icon="kitchen" mappings=[ON="Make"] visibility=[CoffeeMaker_CoffeeMakerStatus=="GREEN"]
```

## IFTTT applet & Google Assistant integration

* Expose commands to be linked in Openhab cloud service settings
* Create IFTTT applet
* If: Google Assistant / say a simple phrase
* Then: Openhab / Send a command

## What the code does not do / possible extensions

- [ ] Read specific display status messages (e.g. out of water, beans etc.)
- [ ] Skip decalcification warning
- [ ] Push cup under coffee outlet (and only after heat up flushing ;-))
- [ ] Coffee consumption statistics
