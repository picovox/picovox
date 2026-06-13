# What is this abomination?
This is a fork of [Nuked OPL2 Lite](https://github.com/nukeykt/Nuked-OPL2-Lite) with optimizations shamelessly borrowed from [Nuked OPL3 Fast](https://github.com/tgies/Nuked-OPL3-fast).
I take no responsibility for this Frankenstein's monster. It somehow works, sounds reasonably accurate, and is significantly faster than the original implementation.
If you understand the code better than I do (and there's a good chance you do), feel free to fix any mistakes, questionable decisions, or accidental horrors that may have made their way into the codebase.

## How to use it
This library is intended to be a drop-in replacement for Nuked OPL2 Lite and should produce identical audio output.
If it does not, that is considered a bug, not a feature. Please open an issue or, even better, submit a fix.

## What is it used for?
This library was created primarily for [picovox](https://github.com/picovox/picovox).
On a Raspberry Pi Pico 2 overclocked to 280 MHz, it performs well enough for real-time use, and there is likely still room for further optimization.

## Licencing
Since both the original Nuked OPL2 Lite and Nuked OPL3 fast use LGPL, this whole library is also released under LGPL.
