# starbie ⭐ (3 button edition)

so basically starbie is a tiny digital pet that lives on your desk, kinda like a tamagotchi lol. it walks around a little oled screen, reacts when you tilt or shake it, shows the temperature and humidity, and now it also gets happy when you press its 3rd button.

this is my version of the  starbie made for **half life by hack club**

<!-- TODO: add pics here, like the schematic, the pcb layout, the 3d render. half life wants photos in the readme -->

## why i made this 
to learn hardware .

## what it does
control what happens ?
button 1 (first press)  opens the menu 
tilt while menu is open  moves the ball to NAP, PLAY, FEED or PET
button 1 (second press) picks the one the ball is on
button 2  shows or hides the stats (joy, energy, fullness, temp, humidity)
button 3 makes starbie happy, it hops and hearts float up and sparkles twinkle
shake  starbie does a shake reaction

## how it works
 **XIAO ESP32-C3** is the brain
 **0.96" oled** shows the pet and the menu
**MPU6050** feels the tilt and shake
**DHT11** reads temperature and humidity (with a 10k resistor on its data pin)
**3 mx switches** are the buttons

pins:
| part | xiao pin | gpio |
| --- | --- | --- |
| dht11 data | D1 | 3 |
| button 1 | D2 | 4 |
| button 2 | D3 | 5 |
| scl (oled + mpu6050) | D4 | 6 |
| sda (oled + mpu6050) | D5 | 7 |
| button 3 | D6 | 21 |

## what i changed from the guide

- **added a 3rd button.** the guide only has 2. this was my twist for the project
- **happy animation** for the 3rd button. press it and starbie hops, hearts float up and sparkles twinkle for a few seconds, and its joy goes up
- **swapped scl and sda.** mine are on D4 and D5 the other way around than the guide's table, so the numbers in the code are swapped too

## the pcb

made in kicad. i wanted a cloud shaped pcb with teddy ears so i used the curve lines to make the edge cut, which was harder than i thought lol. the pcb size also had to be under 100x100mm so i kept moving the parts and the edge cuts closer together to make it smaller. there is a gnd pour on both copper layers. i also added some art on the silkscreen and 3d models for the parts so the render looks nice.

stuff that went wrong (a lot):
- some parts like the esp32 weren't in kicad search so i had to import them from the care package
- my labels in the schematic weren't touching the parts so nothing was actually connected, someone in the slack helped me figure that out
- one trace was unconnected in the drc and i had to fix it

<!-- TODO: say what the final board shape is if it changed from the cloud, and the board size -->

## firmware
it's one arduino sketch based on the guide's code, with my pins and the happy button added.
1. install the arduino ide
2. installed parts
3. wrote code
4. took 200% help from the guide

# status
- schematic and pcb layout are done
- the firmware compiles
- i haven't tested it on the real board yet because it's not built { update in week 5 }

#credits
- made for [half life](https://halflife.hackclub.com) by hack club
- thanks to the people in the half life slack who helped when i got stuck
