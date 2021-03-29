# AppHeltecSpark
Spark 40 control via app, ESP 32 and amp

This allows the app and amp to work together with two Heltec ESP32 boars in between, which allows programmatic control of both app an amp.

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
|Pot 2|39|23|x|
|Pot 3|38|19|Control|
|Pot 4|37|22|Switch 8|
|Pot 5|36||x|


# Example circuit board

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/pic1.jpg)

![Example](https://github.com/paulhamsh/AppHeltecSpark/blob/main/pic2.jpg)
