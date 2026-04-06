
# Future Composer Player on LS2K0300

This is a Future Composer 1.4 player ported to STM32. The sole purpose of this project is to play the tune of Keil uVision keygen*.

I ported it to LS2K0300 to test the function of my HAL.

\* The original music is from [I.C.E 2 of Cytax](https://whdload.de/mags/Cytax_ICE02.html).

# Building

This project uses MDK6 (or Keil Studio Pack) in VSCode to build.

# How to use

By default, this runs on a CTCI Forever pi. Audio is output on GPIO86 pin which is in LCD FPC as PWM wave.

As a practice on C++ compile time features, I've used C++20 compile time feature to invert the byte order of FC14 file header metadata when compiling. The smart pointers of libfc14audiodecoder and all heap operations are removed. This project doesn't use a byte of heap.

Modifications to libfc14audiodecoder are surrounded by conditional compilation ifdefs. Define `FC14_USE_OG` to use the original codebase.

# License

GPL-3.0
