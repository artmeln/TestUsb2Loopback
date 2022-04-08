# Simple USB2 loopback test for use with usb2zynq_loopback repo

This project is meant for testing a USB device that is able to loop back data transfers on two endpoints (an example of such a device is provided in usb2zynq_loopback repo); it should be run on a Windows OS. To compile this project, you’ll need to add libusb.h, libusb-1.0.lib, and libusb-1.0.dll files to the project directory (see [libusb](https://libusb.info)). You also need to make sure that your USB device is using the Winusb driver. This can be done by downloading and running Zadig.
