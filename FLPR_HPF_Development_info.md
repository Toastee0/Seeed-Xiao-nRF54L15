FLPR development guide for xiao nrf54l15
===============================================
documentation sources:
https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/device_guides/coprocessors/index.html
https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/device_guides/nrf54l/vpr_flpr.html
https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/applications/hpf/hpf.html


From the nrf54l15dk overlay for blinky hpf:

&led0 {
	gpios = <&hpf_gpio 9 GPIO_ACTIVE_HIGH>;
};

hpf_gpio mapping:
Mapping pins
VIO pin numbering differs from general pin numbering. See the following table for pin mapping between GPIO and VIO for specific targets:

Pin mapping between GPIO and VIO
Target: nrf54L15 FLPR

HPF_GPIO VIO pins available
4,0,1,3,2,5..10
Corresponding GPIO pins
p2.0=hpf_gpio4 = xiao led0
p2.1=hpf_gpio0 =&xiao_d 8/sck
p2.2=hpf_gpio1 =&xiao_d 10/mosi
p2.3=hpf_gpio3 =xiao RF_SW_PWR
p2.4=hpf_gpio2 =&xiao_d 9/miso
p2.5=hpf_gpio5 =xiao RF_SW_CTRL
p2.6=hpf_gpio6 =&xiao_d d15 (bottom connector)
p2.7=hpf_gpio7 =&xiao_d 7/RX 
p2.8=hpf_gpio8 =&xiao_d 6/TX
p2.9=hpf_gpio9 =&xiao_d d14 (bottom connector)
p2.10=hpf_gpio10 =&xiao_d 13 (bottom connector)


Separate images
You can build and program application sample and the FLPR sample as separate images using nRF Connect for VS Code or command line. To use nRF Util, see Programming application firmware on the nRF54L SoCs. Depending on the selected method, complete the following steps:

Command line
Start the toolchain environment in a terminal window.

Build the application core image, and based on your build target include the appropriate snippet:
```
west build -p -b board target -S nordic-flpr --no-sysbuild
```
Program the application core image by running the west flash command without –erase.
```
west flash
```
Build the FLPR core image:
```
west build -p -b board target --no-sysbuild
```
You can also customize the command for additional options, by adding build parameters.
Once you have successfully built the FLPR core image, program it by running the west flash command without –erase.
```
west flash
```
In the GUI:
1.Open nRF Connect for VS Code.
2.Complete the steps listed on the How to build an application page in the nRF Connect for VS Code documentation.
3.Build the application image by setting the following options:
a.Appropriate board target, depending on the device you are using.
b. Choose either nordic-flpr or nordic-flpr-xip snippet depending on the FLPR image target.
c. System build to No sysbuild.
4. Build the FLPR image by setting the following options:
a.Board target to board target/cpuflpr (recommended) or board target/cpuflpr/xip.
b.System build to No sysbuild.
