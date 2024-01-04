
# Picture-Frame-with-ESP32-and-Camera

<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/10.JPG" width="80%">

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
HALL sensors | DRV5032FA TI | [TI datasheet](https://www.ti.com/lit/ds/symlink/drv5032.pdf?ts=1704351535372&ref_url=https%253A%252F%252Fwww.google.com%252F)
Analog Switch | TS3A5018PW TI | [TI datasheet](https://www.ti.com/lit/ds/symlink/ts3a5018.pdf?ts=1704375672641&ref_url=https%253A%252F%252Fwww.google.com%252F)
Powerbank | Charmast 26800mAh | [Amazon link](https://www.amazon.de/Charmast-Powerbank-Ladeger%C3%A4t-Eing%C3%A4ngen-Smartphones-Schwarz/dp/B073FJ9X2J)



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
- take picture and send to display and µSD card: 300mA for 8s
- load picture from µSD card and send to display: 250mA for 5s
  
We need to know the average current consumption  
Iavg = (300mA * 8s + 250mA * 5s * 2 + 2mA * 3582s) / 3600s = 2.73mA  
to calculate how long the powerbank lasts  
days = 26800mAh / Iavg / 24h = 409 days (1.1 years).
  
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/11.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/12.JPG" width="70%">
<img src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/blob/main/_readme_pics/13.JPG" width="70%">


[comment]: <video width="320" height="240" controls>
<video autoplay loop style="width:100%; height: auto;">
  <source src="https://github.com/ft-13/Picture-Frame-with-ESP32-and-Camera/tree/master/readme_pics/WIN_20231124_15_52_13_Pro.mp4" type="video/mp4" />
</video>

<table>
    <tr>
        <td>Foo</td>
    </tr>
</table>
