# RECTANGLEBIE (3 button edition)
<img width="1920" height="893" alt="Starbie 3d view" src="https://github.com/user-attachments/assets/b5116f53-e8c3-4fcb-9228-94c47502168b" />


so basically starbie is a tiny digital pet that lives on your desk, kinda like a tamagotchi lol. it walks around a little oled screen, reacts when you tilt or shake it, shows the temperature and humidity, and now it also gets happy when you press its 3rd button.

# WHY I MADE THIS
to learn hardware simple . this is my first ever pcb so yeahh so excited to get my hands on it .

# WHAT IT DOES
button 1 opens the menu. when the menu is open you tilt it to move the ball over NAP, PLAY, FEED or PET, then press button 1 again to pick one. button 2 shows or hides the stats like joy, energy, fullness, temperature and humidity. button 3 makes starbie happy, it hops and hearts float up and sparkles twinkle. if you shake it, starbie does a shake reaction. it also saves its stats so it remembers how it was doing after you unplug it.

# HOW IT WORKS
the XIAO ESP32-C3 is the brain. the 0.96 inch oled shows the pet and the menu. the MPU6050 feels the tilt and shake. the DHT11 reads temperature and humidity, with a 10k resistor on its data pin. the 3 mx switches are the buttons.

for the pins, the dht11 is on D1 (gpio 3), button 1 is on D2 (gpio 4), button 2 is on D3 (gpio 5), scl is on D4 (gpio 6), sda is on D5 (gpio 7) and button 3 is on D6 (gpio 21). the names on my schematic wires (gpio2, gpio3, gpio4) are just labels i typed, the real gpio numbers are the ones i just said.

<img width="1920" height="1080" alt="Screenshot 2026-10-11 153941" src="https://github.com/user-attachments/assets/2fcd4cbf-931d-40e3-95b3-02f748e2c935" />


# WHAT I CHANGED FROM THE GUIDE
i added a 3rd button because the guide only has 2, that was my twist for the project. i also made a happy animation for it, so when you press it starbie hops, hearts float up and sparkles twinkle for a few seconds, and its joy goes up. the last thing is that my scl and sda are swapped compared to the guide's table, so the numbers in the code are swapped too.

# THE PCB
made in kicad. i wanted a cloud shaped pcb with teddy ears so i used the curve lines to make the edge cut, which was harder than i thought lol. the pcb size also had to be under 100x100mm so i kept moving the parts and the edge cuts closer together to make it smaller. there is a gnd pour on both copper layers. i also added some art on the silkscreen and 3d models for the parts so the render looks nice.
<img width="1920" height="1080" alt="Screenshot 2026-10-08 010850" src="https://github.com/user-attachments/assets/22290b10-3287-4043-96f2-8e4b1acd8897" />


a lot of stuff went wrong. some parts like the esp32 weren't in kicad search so i had to import them from the care package. my labels in the schematic weren't touching the parts so nothing was actually connected, and someone in the slack helped me figure that out. one trace was also unconnected in the drc and i had to fix it.

<img width="1920" height="1080" alt="Screenshot 2026-10-09 182456" src="https://github.com/user-attachments/assets/78181b9c-4785-47bb-8ca1-95afb7dec3de" />


# FLIMWARE
it's one arduino sketch based on the guide's code, with changes in the code , adding 3 button , changing the character with image app , specifically telling the reaction to the pet to nap rest happy jump excited , menu opening much  deadline is on my head bro .

## STATUS
the schematic and pcb layout are done and the firmware compiles. i haven't tested it on the real board yet because it's not built.(SOON)

## Bill of Materials

| Component    | Quantity | Description         | Source                       | Price |
| ------------ | -------: | ------------------- | ---------------------------- | ----: |
| ESP32-C3     |        1 | Microcontroller     | [link](https://robu.in/product/seeed-studio-xiao-esp32c3-tiny-mcu-board-with-wi-fi-and-ble-battery-charge-supported-power-efficiency-and-rich-interface/) | 749  |
| OLED Display |        1 | Small display       | [link](https://aax-eu-zaz.amazon.in/x/c/REao_15M95vKYntB18vcqsQAAAGhK2_ypgoAAAH_AQBvbm9fdHhuX2JpZDEgICBvbm9fdHhuX2ltcDEgICB6uu3k/clv1c_ek-4wgfWLdTWDdVvBA3o7V4T-Ux7v8IHNdnWY-5ctKp5R5xYsFtGp4DbV6nmeraRcxgdyI_Yi0XkI01zHLAAZ01CFojvgqOBUW2gQakDc1XffTD-nbD2y9RA8v0_aMLlEMCDL0AqpTYyosmahz1lYLmQOs3V8q3o4MreP3fqzoyLQ0clm1MsIVM2OGS2yjg0iJRTVis5a60EcgTmBGgZufhjlVhuUzz6VG7ma_Tew5wT9yn2kHN-jZ4KCZ9pSauUJF41hgDE3dXgJqMZzZvcv96AMqr-pFF7oeqJKqulMdXk7PXw7y0HXslkm3gakSolobKtAbrgz9Rr1lS_EtNeAYkSaQvFt0I6t2vZIiqfbr19nKpUEXebFU1G9yK4du3wJ7ERbrd7Aj92YpeghFsunlpa8ySxw_9Mgkzew1bkQiYYI0p57fk1A7ZdJTYpn0dnCsNrGruLhs53ChVy0V-LS7Zdqq_8IM1IQyEsi93FGfRMnaMoYPIJDPn_HHtxJ27pZTC661BqBCERqSj9oGzt88Co_fQMXnnjPtvunfcass6d35Nl-PZ_SMkU24y8yZI5uWbOYKtZwW1utgh07QRny2vLLaW0JDB94GHCujPd6bm_HxqZMadb0UgAVn0Fh07aTtOflNbZw_GCVD4tB-6xUwlIGM-KlHUGqlNwM5I1yvs5HI7npu9h7h7eAo9hRgEX9g7d8WmMmmS04WhaTE9a7OpZVoJjJFC8JdnTJgIvRAftmMOf2t9-MVsGJusO7PH9f3l3mjPpIXWaoyGdfeaTkHkIgBhck8AvOEnIKGbXHD84qbuRDB0TX9oNLDdOjk_9zGY9diTKQYPIhhHuT1AP5KzYQLfpcuo0-matkF89XGWitHs3422if_rWuqs7pUuk8Rf2UZzcV_6Nc8EYm0eVvnVl294ueOyp5Kn3PZJVMyzbJcLahMZawjXABO6eERntEaDsZg1lO0N_C6udHruySB1VRVvw64Otp_f-8y6N7IgsIbz-dc-z0C6tZm3Ev7MuxT6UThBx2ShPeVz-lUddKA-7CpZNm_txdFVu3bHLhLzYoBE8Z2uqM3sRbY-8Vt6a66b8qH93oa1AP2jorzBr4UJfLzprGsDJ4pr54sw-b0U2Ha_z9Xnv-LzcsLMx2rlDjoTjAbAEhrwBS8yNHhVvEK_G/http://www.amazon.in/dp/B0D9LH4RD8/ref=syn_sd_onsite_desktop_0?ie=UTF8&psc=1&pd_rd_plhdr=t&aref=9WetcRq8gb) |299  |
| 10K Resistor |        1 | Pull-up resistor    | [link](https://www.amazon.in/dp/B0HLGBWRG1?aref=V4mB1tbc8K&aaxitk=d37a7298a5aebdb1885e695ce4fad234&language=en_IN&pd_rd_plhdr=t&ref=dacx_dp_577493687766699741_589633223044007090) |128|
| switches  |        3 | User input buttons  | [link](https://amzn.in/d/0aKbDkSN) | 359 |
| sensor   |        1 | environment sensor  | [link](https://robu.in/product/serial-dht11-temperature-and-humidity-sensor/) | 289 |
| PCB          |        1 | Custom-designed PCB | [link](https://jlcpcb.com/)  | 1330|

all in rupees indian currency 

# CREDITS
 made for half life by hack club. thanks to the people in the half life slack who helped ME when i got stuck.

and all the Beautifull  sticker owner pins


