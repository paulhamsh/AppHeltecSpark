# AppHeltecSpark
Spark 40 control via app, ESP 32 and amp

This allows the app and amp to work together with two Heltec ESP32 boars in between, which allows programmatic control of both app an amp.

v6 controls are:

Sw1   Change up hardware preset   
Sw3   Change down effect type (amp, drive etc)   
Sw4   Change up effect type   
Sw5   Change down effect model (Twin, Roland JC120 etc)    
Sw6   Change up effect model

To operate:

1.  Turn on   
2.  Press reset on both boards   
3.  Turn on Spark   
4.  Wait until connected   
5.  Turn on App   
6.  Pair   
7.  Connect   

Then, use Sw3 and Sw4 to select which effect type to change   
Use Sw5 and Sw6 to change the model   

# AppHeltec

|Description|GPIO|GPIO|Description|
|---|---|---|---|
|GND|GND|||
|VCC|3V3|||
|x||||
|x||||
|x||||
|SER TX|12|||
|SER RX|14|||
|x||||
|x||||
|x||||
|x||||
|x||||
|x||||
|x||||
|x||||
|Control|38|||
|x||||
|x||||


# HeltecSpark

|Description|GPIO|GPIO|Description|
|---|---|---|---|
|x||GND|GND|
|x|||x|
|x||3V3|VCC|
|x|||x|
|x|||x|
|Switch 1|12||x|
|Switch 2|14||x|
|Switch 3|27|2|Switch 7|
|Switch 4|26|0|Switch 5|
|x|||x|
|x|||x|
|Pot 1|32|17|Switch 6|
|x||5|SER RX|
|x||18|SER TX|
|Pot 3|39|23|x|
|Pot 2|38|19|Control|
|Pot 4|37|22|Switch 8|
|Pot 5|36||x|


# Example circuit board

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/pic1.jpg)

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/pic2.jpg)

# Circuit diagram  

Freehand version   

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/image2.jpeg)

Nice version   

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/image0.jpeg)

Better version

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/AppSparkCircuit.jpg)
