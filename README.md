LedStrip2
=========

Intro
-----

Imagine you have 2 RGB addressable led strips and you want to split each in 3 parts,
for example 2 x 3 x 20 LEDS :
- Strip A is connected to pin 10 and strip B to pin 11
- A is divided in 3 trunks A1, A2 and A3, "data out" of last LED in A1 being connected to "data in" in A2 and so on

<pre>

               A1              A2              A3
    pin 10 ---o-o...o-o-------o-o...o-o-------o-o...o-o

               B1              B2              B3
    pin 11 ---o-o...o-o-------o-o...o-o-------o-o...o-o
</pre>

With this library, you can declare 6 objects stripA1, stripA2 ... stripB3 and control each of them as if it was 6 independent strips.
That all.

Usage
-----

The project contains a directory "ledStrip" that can be copied into your "Libraries"
Arduino directory to make it available fron standard Arduino programs.

You can also use the whole directory as an Eclipse project to compile with [my toolchain](https://github.com/piif/arddude)
Then the LedStrip class can be used to access to methods which are documented into the .h file.

The only limitation is that chains of strips must be initialized (begin() method) in the physical order of the chain.
The first strip is called with the data pin as arguments, the others with the first strip as "parent" argument.

References
-----

The library contains a raw copy of code from [MakeBlock firmware](https://github.com/Makeblock-official/Makeblock-Firmware/blob/master/firmware/MeRGBLed.cpp "source on github")
