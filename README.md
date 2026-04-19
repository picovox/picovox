# picovox
> Current state: *alpha 1*

## What is it?
picovox is a low-cost device that serves as an *LPT audio card for old computers* (primarily DOS).
picovox is able to emulate seven audio devices: *Covox*, *Stereo-On-1*, *FTL Sound Adapter*, *Disney Sound Source*, *OPL2LPT*, *TNDLPT* and *CMSLPT*.
For more information and current progress, check out: [Dedicated VOGONS thread](https://www.vogons.org/viewtopic.php?t=109086)

<details>
<summary>Current state of emulation</summary>

| Device | Description |
|-----|-----------|
|Covox|✅ Almost perfect, 96 kHz sampling rate.|
|FTL Sound Adapter|✅ Same as Covox with proper detection.|
|Stereo-On-1|✅ Very close to Covox, with stereo and proper detection|
|Disney Sound Source|✅ Sounds about right, detection should be flawless.|
|OPL2LPT|✅ Close enough to the original AdLib, 48 kHz sampling rate.|
|TNDLPT|✅ Sound is very close to the original, detection flawless.|
|CMSLPT|✅ Sound is close enough to the original.|

</details>

## Why?
I wanted to make a device similar to [PicoGUS](https://picog.us/) for my laptop (which has no ISA slot), especially since the original devices are a bit pricey and I am a poor student.
As a matter of fact, not only is this a passion project, but I also use it for my thesis at university.

### Why does the code look the way it does?
I know that it is probably not the best code you have ever seen. Pardon me, it's my first bigger project and I am working on it on my own. 
Slowly, I will refactor some parts of it to look better and be more readable to all of you.

## How can I build one?
Hardware diagram is finally released, check it out [here](https://github.com/picovox/hardware/). Instructions regarding hardware are located here too.

After successful construction of your device (either by yourself or your handy friend), you also need to upload the firmware part.

## How can I upload/update firmware of my picovox?
Easiest way is to go to [releases](https://github.com/picovox/picovox/releases) and download the latest firmware for your board design.

> [!CAUTION]
> Before updating, always unplug your picovox from both the LPT port and the power supply.

- Unplug your picovox power USB, LPT port and audio jack.
- Plug in micro USB cable into your Raspberry Pi Pico 2.
- **While holding BOOTSEL button** on your Pico, plug in the cable into your PC.
- Now, your Raspberry Pi Pico should show up as a mass storage device.
- Move your downloaded *.uf2* file into the root directory (do not create any folders).
- Unplug the Pico.

Everything should be done. In case anything does not work, try to repeat the process and make sure you have followed all the steps. If issues persist, please report them so we can fix the firmware.

## How do I use it?
You just need to plug your picovox into any LPT port of your computer, plug in the power to the power USB (not the one on the Pico) and your favorite pair of speakers into the audio jack.
In case of Covox/FTL/DSS/Stereo-On-1 you can use them right out of the box. 
In case of OPL2LPT, TNDLPT and CMSLPT, you can use them in the same way as the listed devices; either with games with native support, or load their drivers/patches.
You can switch between modes via a tool for DOS called [PCVX.COM](https://github.com/picovox/DOS/).

> [!CAUTION]
> Although volume between devices should be standardized, always be cautious! Do not damage your hearing!

## Progress and future
Right now, we are in *alpha 1* state. For *alpha 2* I would like to complete the following milestones:

- Support for OPL3LPT. ❌
- PCB design using SMD components (fully compatible with current model). ❌
- Control utility for Linux. ❌
- Fixing of the major bugs in current codebase. ❌

As the name *alpha 1* implies, there may be more alpha versions. I would like to finalize the device to generally usable state by the end of the alpha phase. 
Beta will focus mostly on testing and potentially adding more features.

**picovox is basically a Pico 2 connected to the LPT port. I have designed the PCB with bidirectional data pins in mind.** 
This device might serve as a printer and/or storage emulator in the future as well.

## Licensing
All the code provided in this repository is licensed under *GPL-3.0-or-later*. 
Some parts are dual-licensed (e.g. the square library under BSD-3 clause and the emu8950 under MIT license, everything is explicitly specified). 
*Also, if you happen to meet me by accident and this project seems at least a bit useful to you, you can buy me a beer. Or a soda. Or a chocolate bar.
It's entirely up to you – I won't be mad at any gift, you know?*

## Thanks
Thank you all for your support. Not only have you visited this page, you have also read all the way down there! (Unless you skipped right at the end and missed potentially useful information.) 
Still, there are some people and projects I would like to thank explicitly:

- [Serdaco](https://www.serdaco.com/default.aspx) for distribution of the OPL2LPT, CMSLPT, TNDLPT and many other devices. Please, in case you prefer real sound over emulated, 
these are the devices you are probably looking for. Furthermore for his tips and feedback regarding the picovox project.
- [Ian Scott](@polpo) and the whole team around PicoGUS for inspiring me and for helping me with OPL2 emulation.
- [Jarno Lehtinen](https://github.com/mcgurk/Covox) for his McDSS, pretty similar device built on top of ESP32.
- [The Raspberry Pi](@raspberrypi) for providing not only excellent pieces of hardware, but also great C SDK together with tons of libraries (and pico-extras library too).
- [Graham Sanderson](https://github.com/kilograham/rp2040-doom) (OPL2) and [Aaron Giles](https://aarongiles.com/) (TND, CMS) for providing sound emulation libraries to PicoGUS (that I could use).
- [VOGONS community](https://www.vogons.org/) for a place to share my ideas and to find people with similar interests.

- *My family, my friends and everyone who has supported me in any way. Without you I would not have done it.*
