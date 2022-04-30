// TestUsb2Loopback.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/* Disable: warning C4200: nonstandard extension used : zero-sized array in struct/union */
#pragma warning(disable:4200)

#include <iostream>
#include <vector>
#include <sstream>
#include "libusb.h"

using namespace std;

static const unsigned char bulk_ep1_out = 0x01;
static const unsigned char bulk_ep1_in = 0x81;
static const unsigned char bulk_ep2_out = 0x02;
static const unsigned char bulk_ep2_in = 0x82;
static const unsigned char bulk_ep3_out = 0x03;
static const unsigned char bulk_ep3_in = 0x83;

string MakeUniqueUSBDescriptor(char* strDesc, unsigned char serialNumber) {
	stringstream ss;
	ss << string(strDesc) << " (" << (int)serialNumber << ")";
	return ss.str();
}

int main()
{
	vector<string> vecDevices;
	int ret = libusb_init(NULL);
	if (ret >= 0) {
		// Get a list of all USB devices    
		struct libusb_device** devs;
		struct libusb_device* dev;
		struct libusb_device* devToConnect = 0;
		struct libusb_device_descriptor desc;
		struct libusb_device_handle* usbHandle=0;
		ssize_t nDevices = libusb_get_device_list(NULL, &devs);
		for (ssize_t ii = 0; ii < nDevices; ii++) {
			dev = devs[ii];
			ret = libusb_get_device_descriptor(dev, &desc);
			if (ret < 0) continue;
			if (desc.bDeviceClass == LIBUSB_CLASS_VENDOR_SPEC) {
				ret = libusb_open(dev, &usbHandle);
				if (ret < 0) continue;
				char strDesc[64];
				libusb_get_string_descriptor_ascii(usbHandle, desc.iProduct, (unsigned char*)strDesc, sizeof(strDesc));
				vecDevices.push_back(MakeUniqueUSBDescriptor(strDesc, desc.iSerialNumber));
				if (vecDevices.size()==1) devToConnect = dev; // first device in the list
			}
		}
		if (vecDevices.size() == 0) {
			libusb_free_device_list(devs, 1);
			std::cout << "No devices were found!\n";
		}
		else {
			printf("Found the following devices:\n");
			stringstream ss;
			for (int ii = 0; ii < vecDevices.size(); ii++) {
				ss << "Device " << ii << ": " << vecDevices.at(ii) << "\n";
			}
			std::cout << ss.str();
		}

		stringstream ssErrorMessage;
		// opening first available device
		if (devToConnect != 0) {
			// get configuration
			int usbConfig;
			ret = libusb_get_configuration(usbHandle, &usbConfig);
			if (ret != 0) {
				ssErrorMessage << "Error (" << ret<< "): unable to get device configuration";
				libusb_close(usbHandle);
				return ret;
			}
			// make sure configuration is set to 1
			if (usbConfig != 1) {
				ret = libusb_set_configuration(usbHandle, usbConfig);
				if (ret != 0) {
					ssErrorMessage << "Error (" << ret << "): unable to set device configuration to " << usbConfig;
					libusb_close(usbHandle);
					return ret;
				}
			}
			// detach kernel driver if currently active
			if (libusb_kernel_driver_active(usbHandle, 0) == 1) {
				if (libusb_detach_kernel_driver(usbHandle, 0) != 0) {
					ssErrorMessage << "Error (" << ret << "): unable to detach active kernel driver";
					libusb_close(usbHandle);
					return ret;
				}
			}
			// claim interface 0
			int usbInterface = 0;
			ret = libusb_claim_interface(usbHandle, usbInterface);
			if (ret < 0) {
				ssErrorMessage << "Error (" << ret << "): unable to claim interface" << usbInterface;
				libusb_close(usbHandle);
				return ret;
			}
		}
		else {
			libusb_free_device_list(devs, 1);
			return -1;
		}

		// communication on Endpoint1
		// write
		int sentBytes = 0;
		int receivedBytes=0;
		unsigned char* buffOut = new unsigned char[] { "Checking Ep1" };
		int length = (int)strlen((char*)buffOut);
		int ret = libusb_bulk_transfer(usbHandle, bulk_ep1_out, buffOut, length, &sentBytes, 1000);
		if (ret == 0 && sentBytes == length) {
			printf("Write on Ep1 successful!\n");
			printf("Sent string with %i", sentBytes);
			printf("bytes: %s\n", buffOut);
		}
		else {
			printf("Error in Ep1 write (code = %u", ret);
			printf("); transferred %i bytes\n", sentBytes);
		}
		Sleep(3);
		// read
		unsigned char* buffIn = new unsigned char[512];
		ret = libusb_bulk_transfer(usbHandle, bulk_ep1_in, buffIn, 512, &receivedBytes, 1000);
		buffIn[receivedBytes] = '\0';
		if (ret == 0 && receivedBytes == strlen((char*)buffIn)) {
			printf("Read on Ep1 successful!\n");
			printf("Received %d", receivedBytes);
			printf(" bytes with string: %s\n", buffIn);
		}
		else {
			printf("Error in Ep1 read (code = %li", ret);
			printf("); received %i bytes\n", receivedBytes);
		}

		// communication on Endpoint2
		// write
		sentBytes = 0;
		receivedBytes = 0;
		buffOut = new unsigned char[] { "and checking Ep2" };
		length = (int)strlen((char*)buffOut);
		ret = libusb_bulk_transfer(usbHandle, bulk_ep2_out, buffOut, length, &sentBytes, 1000);
		if (ret == 0 && sentBytes == length) {
			printf("Write on Ep2 successful!\n");
			printf("Sent string with %i", sentBytes);
			printf(" bytes: %s\n", buffOut);
		}
		else {
			printf("Error in Ep2 write (code = %u", ret);
			printf("); transferred %i bytes\n", sentBytes);
		}
		Sleep(3);
		// read
		buffIn = new unsigned char[512];
		ret = libusb_bulk_transfer(usbHandle, bulk_ep2_in, buffIn, 512, &receivedBytes, 1000);
		buffIn[receivedBytes] = '\0';
		if (ret == 0 && receivedBytes == strlen((char*)buffIn)) {
			printf("Read on Ep2 successful!\n");
			printf("Received %d", receivedBytes);
			printf(" bytes with string: %s\n", buffIn);
		}
		else {
			printf("Error in Ep2 read (code = %u", ret);
			printf("); received %i bytes\n", receivedBytes);
		}

		// communication on Endpoint3
		// write
		sentBytes = 0;
		receivedBytes = 0;
		buffOut = new unsigned char[] { "finally checking Ep3" };
		length = (int)strlen((char*)buffOut);
		ret = libusb_bulk_transfer(usbHandle, bulk_ep3_out, buffOut, length, &sentBytes, 1000);
		if (ret == 0 && sentBytes == length) {
			printf("Write on Ep3 successful!\n");
			printf("Sent string with %i", sentBytes);
			printf(" bytes: %s\n", buffOut);
		}
		else {
			printf("Error in Ep3 write (code = %u", ret);
			printf("); transferred %i bytes\n", sentBytes);
		}
		Sleep(3);
		// read
		buffIn = new unsigned char[512];
		ret = libusb_bulk_transfer(usbHandle, bulk_ep3_in, buffIn, 512, &receivedBytes, 1000);
		buffIn[receivedBytes] = '\0';
		if (ret == 0 && receivedBytes == strlen((char*)buffIn)) {
			printf("Read on Ep3 successful!\n");
			printf("Received %d", receivedBytes);
			printf(" bytes with string: %s\n", buffIn);
		}
		else {
			printf("Error in Ep3 read (code = %u", ret);
			printf("); received %i bytes\n", receivedBytes);
		}

	}

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
