/*
Copyright (c) 2015 Graham Chow

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "Bombardier.h"
#include "USBInterface.h"

USBInterface::USBInterface() :
	_handle(0),
	DLLInitializeDevice(0),
	DLLTerminateDevice(0),
	DLLWriteUSBPacket(0),
	_cachePwm(-1)
{
}

USBInterface::~USBInterface()
{
}

void USBInterface::LoadAllHandles()
{
	FString filePath1 = FPaths::Combine(*FPaths::GamePluginsDir(), TEXT("TivaCBulkPlugin/"), TEXT("TivaCBulkPlugin.dll")); // Concatenate the plugins folder and the DLL file.
	FString filePath2 = FPaths::Combine(*FPaths::GameDir(), TEXT("TivaCBulkPlugin.dll"));
	FString filePath3 = FPaths::Combine(*FPaths::LaunchDir(), TEXT("TivaCBulkPlugin.dll"));
	FString *filePath = 0;
	if (FPaths::FileExists(filePath1))
		filePath = &filePath1;
	else if (FPaths::FileExists(filePath2))
		filePath = &filePath2;
	else if (FPaths::FileExists(filePath3))
		filePath = &filePath3;
	if (filePath != 0)
	{
		void *DLLHandle;
		DLLHandle = FPlatformProcess::GetDllHandle(**filePath); // Retrieve the DLL.
		if (DLLHandle != NULL)
		{
			FString procNameInit = "InitializeDeviceEasy"; // The exact name of the DLL function.
			DLLInitializeDevice = (_InitializeDevice)FPlatformProcess::GetDllExport(DLLHandle, *procNameInit); // Export the DLL function.
			FString procNameTerm = "TerminateDevice"; // The exact name of the DLL function.
			DLLTerminateDevice = (_TerminateDevice)FPlatformProcess::GetDllExport(DLLHandle, *procNameTerm); // Export the DLL function.
			FString procNameWrite = "WriteUSBPacket"; // The exact name of the DLL function.
			DLLWriteUSBPacket = (_WriteUSBPacket)FPlatformProcess::GetDllExport(DLLHandle, *procNameWrite); // Export the DLL function.

		}
	}
}

bool USBInterface::Initialise()
{
	LoadAllHandles();
	if (DLLInitializeDevice != NULL)
	{
		int installed = 0;
		_handle = DLLInitializeDevice(&installed); // Call the DLL function, with arguments corresponding to the signature and return type of the function.
		return (_handle != 0);
	}
	return false;
}

bool USBInterface::TerminateDevice()
{
	if (_handle != 0)
	{
		if (DLLTerminateDevice != NULL)
		{
			if (DLLTerminateDevice(_handle) != 0)
			{
				_handle = 0;
				return true;
			}
		}
	}
	return false;
}

bool USBInterface::WriteUSBPacket(unsigned char *pcBuffer, unsigned long ulSize, unsigned long *pulWritten)
{
	if (_handle != 0)
	{
		if (DLLWriteUSBPacket != NULL)
		{
			return DLLWriteUSBPacket(_handle, pcBuffer, ulSize, pulWritten) != 0; // Call the DLL function, with arguments corresponding to the signature and return type of the function.
		}
	}
	return false;
}


bool USBInterface::WritePwm(float alertLevel)
{
	if (_cachePwm != alertLevel)
	{
		// 1 = 200 us
		// max 20ms
		int pwm_value = FMath::Clamp((int)(alertLevel * 100), 1, 100);
		unsigned char pwm[3] = { 0xAA, 0xAA, pwm_value };
		unsigned long written;
		if (WriteUSBPacket(pwm, 3, &written))
		{
			_cachePwm = alertLevel;
			return true;
		}
	}
	return true;

}