
# Picture-Frame-with-ESP32-and-Camera
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/10.JPG" width="80%">
The picture frame consists of Firebeetle2 (ESP32-S3) with OV2640 camera, epaper display, powerbank and a self-desined carrier PCB. The whole electronic is mounted on a 3D printed insert. There are 3 Hall sensors attached to the wood of picture frame: top, bottom left and bottom right. µSD card shield is mounted to the carrier PCB and makes it possible to save the camera picture or to load a picture. Using a magnet triggers one of the three Hall sensors and wakes up ESP32 from deep sleep. When the top Hall sensor is triggered, the picture frame takes a picture and sends it to the epeaper and the µSD card. If the left or right sensor is triggered, the previous or next image is loaded from the µSD card to the epeaper display.  
  
Special thanks go to my wife, who has always been very patient with my handicrafts :)

<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/demo_video2.gif" width="50%" />

## Software
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/flowchart.JPG" width="70%">


## Hardware
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/1.JPG" width="80%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/9.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/2.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/3.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/4.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/5.JPG" width="70%">

### BOM
Part | Description | Link 
--- | --- | ---
Firebeetle2 | ESP32-S3 8MB PSRAM and OV2640 camera | [dfrobot link](https://www.dfrobot.com/product-2677.html)
IT8951 display HAT | - | [Waveshare link](https://www.waveshare.com/9.7inch-e-paper-hat.htm)
epaper display | 1200x825, 9.7inch | [Waveshare datasheet](https://www.waveshare.com/w/upload/f/ff/9.7inch_E-paper_Specification.PDF)
µSD shield | D1 mini shield adapter | [AZ delivery link](https://www.az-delivery.de/products/micro-sd-card-adapter-shield)
Hall sensors | DRV5032FA TI | [TI datasheet](https://www.ti.com/lit/ds/symlink/drv5032.pdf?ts=1704351535372&ref_url=https%253A%252F%252Fwww.google.com%252F)
Analog Switch | TS3A5018PW TI | [TI datasheet](https://www.ti.com/lit/ds/symlink/ts3a5018.pdf?ts=1704375672641&ref_url=https%253A%252F%252Fwww.google.com%252F)
Powerbank | Charmast 26800mAh | [Amazon link](https://www.amazon.de/Charmast-Powerbank-Ladeger%C3%A4t-Eing%C3%A4ngen-Smartphones-Schwarz/dp/B073FJ9X2J)
Picture frame | Ikea black 21x30 cm | [Ikea link](https://www.ikea.com/de/de/p/ribba-rahmen-schwarz-60378396/)



### Powerbank
The powerbank has a hugh capacity of 26800mAh. When ESP32 is in deep sleep the entire electronic draws a current of <=2mA. If the current draw from powerbank is about <70mA the powerbank automatically turns off its outputs after 30 seconds. Then the electronic has no chance to power up unless the button of powerbank is pressed to switch it on again. I figured out that the powerbank has two DCDC converters: IP5306 and IP5310. Both chips can be configured via I2C. I found the I2C protocol of IP5306 on github: [github link](https://github.com/bheesma-10/IP5306_I2C) but no chance for IP5310. For IP5306 there is a register bit which can be set to tell the chip to stay on. Unfortunatelly this bit is not the same for IP5310. Why not using IP5306 instead of IP5310? Because I could get access only to I2C pins of IP5310 via charging state LEDs. The next problem is that IP5310 is responsible for battery charging.

I soldered I2C wires to the LEDs which are connected to IP5310 and tried to write to different registers and voila! Bit 7 of register 0x02 tells the chip to stay on.
What else? The powerbank on/off switch is connected to IP5306. As soon as IP5306 wakes up, it will wake up IP5310 via a transitor. Therefore I had to do some modifications on powerbank PCB. Now I can switch IP5310 off and on by pressing the power button of the powerbank (single push: ON, double push: OFF).

<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/6.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/7.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/8.JPG" width="70%">

### Current Consumption
My goal was it that the powerbank lasts for about 1 year. Of course it depends on how many pictures do you take and how many picture do you load from µSD card.
In deep sleep mode the entire electronic (excluding powerbank) draws a current of about 344µA. Unfortunately, this is not the figure that can be used for the calculation. The IP5310 DCDC converter has a converter efficiency that drives the figure up at the end. That's why I measured the current consumption from the battery itself. The unloaded powerbank draws a current of 1.4mA and in loaded condition (334µA) the current consumption is about 2mA.
If I never took a photo, i.e. ESP32 is in deep sleep all the time, the power bank would last:
days = 26800mAh / 2mA / 24h = 558 days (1.5 years).

But lets make the calculation with some assumptions:  
(I have not yet made any exact measurements, so this is a simplified calculation)
- 1 picture per day and 2 µSD card read access' per day.
- take picture and send to display and µSD card: 300mA for 14s
- load picture from µSD card and send to display: 250mA for 8s
  
We need to know the average current consumption  
Iavg = (300mA * 14s + 250mA * 8s * 2 + 2mA * 3570s) / 3600s = 4.26mA  
to calculate how long the powerbank lasts  
days = 26800mAh / Iavg / 24h = 262 days.
  
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/11.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/12.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/13.JPG" width="70%">

## FAQ
1. Why is the carrier PCB so hugh?  
   Firebeetle2 (Camera has short flex PCB) and IT8951 cannot not be postioned on the same side, as the flex PCB of epaper fills the entire bottom side. Therefore there is no space left where the camera could see through.
2. Why did you use Firebeetle2? What's so special about?  
   The Firebeetle2 has a DCDC Converter, which can be configured over I2C and can deactivate the power of the camera, thus bringing the power consumption of the camera to null.
3. Why is the size of the camera picture (XGA 1024x768) smaller than the available size of the display (1200x825)?  
   Because the passpartout cut-out of the picture frame has nearly the size of XGA. Of course I could have bought a passpartout with a larger cut-out, which I will do next.
4. If you have already designed your own PCB, why didn't you include a charging circuit instead of struggling with the power bank?  
   I didn't do much research but I couldn't find a suitable Li-Ion charging IC with integrated boost converter and pass through function.
   I also wanted to keep the development time as short as possible and do without non-SOICs, as I soldered the components on by hand.
   And it was also fun "hacking" the powerbank.
5. Why did you decide to use Hall sensors instead of switches, proximity sensors or even gesture recognition using the camera?  
   The current consumption of the Hall sensors is a maximum of 3.5µA per sensor, as other sensor technologies cannot keep up.
   I wanted to do without electromechanical switches, as the overall picture would not have been nice for me, apart from the fact that I couldn't find any very small built-in pushbuttons.
   Gesture recognition using the ESP camera would have increased power consumption and drastically reduced battery life.
   A capacitive proximity switch consisting of e.g. an AD7151/AD7156 and a capacitive foil would certainly have worked as well, if not more elegantly, but in this case I wanted to save costs as I had the Hall sensors lying around anyway.
6. In which format are the files saved on the SD card or in which format do they have to be?  
   The recorded image is saved as raw bytes with an ascending number and the file extension .txt. The file size is always exactly 768432 bytes (1024x768 pixels and 8bit/pixel). 
7. Is there a way to view the images on the SD card on a PC and is it possible to save images other than those taken by the camera itself?  
   Yes, both are possible. It is possible with the free tool magick e.g. In the repository folder "Software/2_pic_converter" there is a self-written python tool.
   This tool converts all jpg images in a folder and stores them in an "output" folder. These can then simply be copied to the SD card (with ascending number).
   With the command "magick -depth 8 -size 1024x768 gray:files_on_sd_card.txt result.jpg" the raw bytes can be converted back into a jpg image.
   The tool is currently very simple and does not yet have any terminal arguments etc.
8. AI generated pictures?  
   The next upgrade will be for my Raspberry Pi server to generate an image once a day from an AI on the internet and then send it to the picture frame via Wifi.