# AbracaDABra
Abraca DAB radio is DAB and DAB+ Software Defined Radio (SDR) application. It works with cheap RTL-SDR (RTL2832U) USB sticks but also with AirSpy devices and with devices supported by <a href="https://github.com/pothosware/SoapySDR/wiki">SoapySDR</a>. 

Application is based on Qt6 cross-platform software development framework and can run on any platform supported by Qt6. 
Prebuilt binaries are released for Windows, MacOS (both Intel and Apple Silicon) and Linux x86-64 (AppImage). 
AbracaDABra can run also under Linux AARCH64 (ARM64) for example on Raspberry Pi 4. 

<img width="1634" alt="Snímek obrazovky 2022-06-28 v 22 32 25" src="https://user-images.githubusercontent.com/6438380/176279785-1e212201-3c1d-428f-9416-b1b0b464238b.png">

## Features
* Following input devices are supported:
  * RTL-SDR (default device)
  * Airspy (optional)
  * SoapySDR (optional)
  * RTL-TCP
  * Raw file input (in expert mode only, INT16 or UINT8 format)
* Band scan with automatic service list
* Service list management
* DAB (mp2) and DAB+ (AAC) audio decoding
* Announcements (all types including alarm test)
* Dynamic label (DL) and Dynamic label plus (DL+)
* MOT slideshow (SLS) and categorized slideshow (CatSLS) from PAD or from secondary data service
* Audio services reconfiguration
* Ensemble structure view with all technical details
* Raw file dumping
* Only band III and DAB mode 1 is supported.
* Simple user-friendly interface, trying to follow DAB _Rules of implementation (<a href="https://www.etsi.org/deliver/etsi_ts/103100_103199/103176/02.04.01_60/ts_103176v020401p.pdf">TS 103 176</a>)_
* Multiplatform (Windows, MacOS and Linux)

## Basic mode
<img width="663" alt="Snímek obrazovky 2022-06-28 v 22 19 47" src="https://user-images.githubusercontent.com/6438380/176277787-7737ca9b-adb1-4d91-bf5b-bd9331d27663.png">

Simple user interface that is focused on radio listening. Just select your favorite service from service list on the right side 
and enjoy the music with slideshow and DL(+). 
Service can be easily added to favorites by clicking "star" icon.  Most of the elements in UI have tool tip with more information.

## Expert mode
<img width="921" alt="Snímek obrazovky 2023-02-25 v 19 29 45" src="https://user-images.githubusercontent.com/6438380/221374046-2d5558fb-fe7e-4a32-b754-1174cb525bc0.png">

In addition to basic mode, expert mode shows ensemble tree with structure of services and additional details of currenly tuned service. 
Furthermore it is possible to change the DAB channel manually in this mode. 
This is particularly useful when antenna adjustment is done in order to receive ensemble that is not captured during automatic band scan.
Expert mode can be enabled in "three-dot" menu.

## DAB Announcements support

An announcement is a period of elevated interest within an audio programme. It is typically a spoken audio message, often with a lead-in and lead-out 
audio pattern (for example, a musical jingle). It may refer to various types of information such as traffic, news, sports and others. [<a href="https://www.etsi.org/deliver/etsi_ts/103100_103199/103176/02.04.01_60/ts_103176v020401p.pdf">TS 103 176</a>]

All DAB(+) announcement types are supported by AbracaDABra, including test alarm feature (ClusterID 0xFE according to <a href="https://www.etsi.org/deliver/etsi_ts/103100_103199/103176/02.04.01_60/ts_103176v020401p.pdf">TS 103 176</a>). 
The application is monitoring an announcement switching information and when the announcement is active, AbracaDABra switches audio output to a target 
subchannel (service). This behavior can be disabled by unchecking a particular announcement type in a Setup dialog. 
If an announcement occurs on the currently tuned service, it is only indicated by an icon after the service name. This icon can be clicked on, which 
suspends/resumes the ongoing announcement coming from another service. OE Announcements are not supported. 

The option "Bring window to foreground" tries to focus the application window when the alarm announcement starts. 
The alarm announcements carry emergency warning information that is of utmost importance to all radio listeners and they have the highest priority 
(according to <a href="https://www.etsi.org/deliver/etsi_ts/103100_103199/103176/02.04.01_60/ts_103176v020401p.pdf">TS 103 176</a>) in the sense that alarm announcement cannot be disabled and it can interrupt any ongoing regular announcement.

<img width="1279" alt="Snímek obrazovky 2023-01-12 v 22 07 17" src="https://user-images.githubusercontent.com/6438380/212181613-a7e163e2-d6e1-46cc-bf3d-6dfce276a8ae.png">


Announcements from other service display a thematic placeholder. <a href="https://www.flaticon.com/authors/basic-miscellany/lineal-color" title="linear color">The artwork of placeholders are created by Smashicons - Flaticon</a>

## Expert settings
Some settings can only be changed by editting of the INI file. File location is OS dependent:
* MacOS & Linux: `$HOME/.config/AbracaDABra/AbracaDABra.ini`
* Windows: `%USERPROFILE%\AppData\Roaming\AbracaDABra\AbracaDABra.ini`

Following settings can be changed by editing AbracaDABra.ini:

      [RTL-SDR]
      bandwidth=0     # 0 means automatic bandwidth, positive value means bandwidth value in Hz (e.g. bandwidth=1530000 is 1.53MHz BW)
      bias-T=false    # set to true to enable bias-T
      
      [AIRSPY]
      bias-T=false    # set to true to enable bias-T      

      [SOAPYSDR]
      bandwidth=0     # 0 means default bandwidth, positive value means bandwidth value in Hz (e.g. bandwidth=1530000 is 1.53MHz BW)

Application shall not run while changing INI file, otherwise the settings will be overwritten.

It is also possible to use other than default INI file using `--ini` or `-i` command line parameter.

## How to build
Following libraries are required:
* Qt6
* libusb
* rtldsdr
* faad2 (default) or fdk-aac (optional)
* mpg123
* portaudio
* airspy (optional)
* SoapySDR (optional)

For a fresh Ubuntu 22.04 installation you can use the following commands:

       sudo apt-get install git cmake build-essential mesa-common-dev
       sudo apt-get install libusb-dev librtlsdr-dev libfaad2 mpg123 libmpg123-dev libfaad-dev
       sudo apt-get install portaudio19-dev qt6-base-dev qt6-multimedia-dev libqt6svg6-dev rtl-sdr   
       sudo apt-get install qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools
       
Optional Airspy support:       

       sudo apt-get install libairspy-dev

Optional SoapySDR support:       

       sudo apt-get install libsoapysdr-dev

Then clone the project:

       git clone https://github.com/KejPi/AbracaDABra/
       cd AbracaDABra

1. Create build directory inside working copy and change to it

       mkdir build
       cd build

2. Run cmake

       cmake ..
       
    Optional Airspy support:          
       
       cmake .. -DAIRSPY=ON

    Optional SoapySDR support:          
       
       cmake .. -DSOAPYSDR=ON

3. Run make

       make      
       
## Known limitations
* Reconfiguration not supported for data services
* Only SLS data service is currently supported
* Application is hanging when audio output device is removed (like bluetooth headphones disconnected) - PortAudio does not support hot swap
* MacOS Ventura refuses to start application downloaded from internet. This command run from Terminal after installation seems to solve the problem:

       xattr -cr /Applications/AbracaDABra.app
       
