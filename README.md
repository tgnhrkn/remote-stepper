# remote-stepper

The motivation behind this project is my inability to wake up. My bedroom has blackout shades which make sleeping awesome and waking up hard. I was bad at waking up before installing the blackout shades, I am even worse now. 

So, I wanted to be able to control the shade position remotely to enable two modes: alarm mode and weekend mode. In alarm mode, the shade would raise at a predetermined time, ideally set via home automation voice commands or similar. In weekend mode, the shade would respond to a remote to raise or lower.

The shade in my bedroom is similar to [this one](https://www.homedepot.com/p/StyleWell-Cut-to-Size-Linen-Cordless-Blackout-Fabric-Roller-Shade-73-25-in-W-x-72-in-L-RSSLPNLBOSC7372/309076211), note that it is cordless. The solution I have come up with is to mount a motor at the top of the window sill that would simulate my hand pushing the shade up (by pulling up on the bottom of the shade). The motor would have a wheel on the shaft on which fishing line would be wound. The fishing line would reach down to the bottom of the shade and loop over a hook. Rotating the motor would tighten the line, pulling the shade bottom up. The internal mechanism of the shade would reel it in as it is pulled up, mission accomplished. However, there is the small issue of lowering the shade. I have thought about a few ways to do this. The simplest would be to increase the weight of the shade bottom so that it is naturally inclined to lower, then the holding torque of the motor could hold it in place and unravelling the fishing line would let the shade fall. Another solution would involve a pulley system that, instead of relying on the mechanism of the shade, just attaches to the shade bottom at a fixed point in the pulley cord and raises and lowers by moving the fixed point of the pulley. I will go with the first approach because the second sounds more complicated and probably would mean ordering some more visible timing belts and gears.

# Project Materials and Plan

## Requirements

I need a motor to mount to the top of my window that can hold in position, can make precise movements, and isn't really large, loud, hot or expensive. I also need a remote that has a couple of buttons and can talk to the motor.

## Plan

- Code a basic motor controller 'OS' that can react to button presses and remote commands. Build a remote 'OS' that sends commands to the motor controller.

- 3d model an enclosure for the motor controller circuitry and motor and 3d print it.

- Solder up the motor controller circuitry and install the motor and circuitry in the housing.

- Mount the motor and test

- Iterate as necessary on the above

- Print a final 'pretty' enclosure and use the product!

## Core Materials

I chose NEMA 17 stepper motors for the motor, specifically one of [these](https://www.amazon.com/dp/B094CQ4DBQ?ref=ppx_yo2_dt_b_product_details&th=1). They are relatively cheap, the holding torque (42Ncm) is enough for my use case, they are common so I expected good hardware and software support, and they are used in the 3d printing world and I could repurpose the extras for a DIY 3d printer (I bought 5).

Stepper motors need drivers, and after some googling I (maybe naively?) ordered some [TMC2208 stepper motor drivers](https://www.amazon.com/dp/B094CQ4DBQ?ref=ppx_yo2_dt_b_product_details&th=1) because TMC (Trinamic Motion Control) seemed like a reputable company and reviews for the drivers were positive. It turns out that these drivers (and the TMC22XX series in general) are excellent drivers that support super-quiet operation and some advanced configurability. The TMC2208 dev boards that I bought were desiged (and the TMC2208 in general) to be drop-in replacements for non-quiet A4988 drivers. If I were ordering drivers now I would probably get A4988 drivers because they are way simpler and I don't really care about the quietness (I am trying to wake up after all). I am certainly happy with the TCM2208 because they are super quiet and can work as simply as A4988 but at a higher price point. The basic operation mode involves sending a STEP and DIR signal (rising edge on STEP = make one step (or microstep if enabled), DIR logic level controls motor direction) which makes the control code easy to think about.

To run the motor and remote logic I chose ESP32 modules on [these boards](https://www.amazon.com/dp/B09J95SMG7?psc=1&ref=ppx_yo2_dt_b_product_details) that were pretty cheap. ESP32 is the successor of the ESP8266 wifi microcontroller and are pretty sweet. The ones I bought did not have bluetooth support (probably why they are cheap) but WiFi is enough for this project, although if the power went out it would be cool to keep using bluetooth.

If the power went out? That's right, this whole project will be battery powered. The remote will run off of a [3.7v 1200mAh lipo](https://www.adafruit.com/product/258) with [a charging board attached](https://www.adafruit.com/product/259) for easy recharging/continuous use. The motor driver needs (omg I just read the datasheet and I totally didn't need to buy such a big battery)... nevermind. I though it needed >12v DC, turns out 5V would suffice. BUT I bought a 14.8v lipo (even though I already have several 7.4v) so that's cool. Anyway, to regulate the 14.8v battery to the 5V needed to power the ESP32 I bought a [cheap (and tiny) voltage regulator](https://www.amazon.com/dp/B08SHQHNNR?ref=ppx_yo2_dt_b_product_details&th=1). I was a little bit apprehensive about these because they were so small and cheap but so far they are working fine. They are a little hard to adjust with a multimeter because the pads are so small but directly fiddling with the potentiometer while measuring the output with a multimeter works fine. Why the different battery voltages? These ESP32 dev boards (and maybe ESP32 in general?) support 5v input on the 5v rail or 3.3v input on the 3.3v rail as power. It turns out that the 3.7v lipo voltage range works fine for this, although it may not be the totally correct way to run the ESP32. So I will regulate 14.8v -> 5.1v for the motor board and just go straight from battery -> 3.3v rail on the remote. [Here is a website that says to not do what I am doing, YMMV](https://techexplorations.com/guides/esp32/begin/power/). I bought a lipo charger for the motor batteries, [this one from Dynamite](https://www.amazon.com/dp/B07751LCYB?psc=1&ref=ppx_yo2_dt_b_product_details).

Back to the TMC2208, it has onboard current regulation that can be adjusted via potentiometer. I set the value very low for testing (~0.3A even though my motors go up to 1.5A) and plan to keep it as low as possible in practice to keep the single-battery runtime as long as possible. At higher currents the board gets HOT and gets hot FAST. You could maybe run the motor for 30 seconds at >1A before the TMC2208 shuts itself down due to heat. This makes a fan (and included heatsinks) important if you want to run the stepper for any extended amount of time (>30s). I bought some 50x50x10mm 12V DC fans that I will turn on when the motor is being driven, mouted to blow on the heatsink of the TMC2208. The ESP32 can also run hot but I haven't seen this as much of an issue yet.

## Programming

I will be writing/wrote the code for this project using the Arduino platform. I considered PlatformIO and may switch to it over time but Arduino library availability is too easy to not prototype in. Installing ESP32 support was moderately challenging following instructions [here](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html). There are some steps in there about installing the `Git GUI` that I would avoid if you are an experience git user. That being said, the most surefire way to get it done is by following the instructions to the letter, which I did.

## Equipment & Skillz

I already had some equipment and skillz before starting this project including:

- multimeter
- bench power supply
- breadboards/perfboard
- 3d printer/CAD skillz
- embedded systems programming experience
- jumper wires,cutters,strippers
- soldering iron

This project inspired me to buy a [Dupont/JST crimper](https://www.amazon.com/dp/B07R1H3Z8X?psc=1&ref=ppx_yo2_dt_b_product_details). So far it has been great. Some of the reviews about it not working correctly or that is the wrong size differ from my experience thus far.

## Off the Shelf

As far as I can tell a wifi enabled, battery powered, homebrewable stepper motor setup doesn't exist as an off the shelf component. This is certainly not the best we can do in that area, integrating electronics with the motor housing would be far more ideal. Baby steps?

Remote controlled window shades do exist, but that wasn't an option for this project.

## What the heck are you actually making?

Good point, pretty much a box, that has a motor in it, that has some fishing line coming out of it, that connects to the window shade and a remote that can tell it to go up and down.


