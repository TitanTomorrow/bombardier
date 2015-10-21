// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TIVACBULKPLUGIN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TIVACBULKPLUGIN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TIVACBULKPLUGIN_EXPORTS
#define TIVACBULKPLUGIN_API __declspec(dllexport)
#else
#define TIVACBULKPLUGIN_API __declspec(dllimport)
#endif

//// This class is exported from the TivaCBulkPlugin.dll
//class TIVACBULKPLUGIN_API CTivaCBulkPlugin {
//public:
//	CTivaCBulkPlugin(void);
//	// TODO: add your methods here.
//};
//
//extern TIVACBULKPLUGIN_API int nTivaCBulkPlugin;
//
//TIVACBULKPLUGIN_API int fnTivaCBulkPlugin(void);


	//****************************************************************************
	//
	// A handle returned by InitializeDevice().
	//
	//****************************************************************************
	typedef void *LMUSB_HANDLE;

	//****************************************************************************
	//
	// Flags used in constructing the ucRequestType parameter to Endpoint0Transfer().
	//
	//****************************************************************************
#define REQUEST_TRANSFER_IN             0x80
#define REQUEST_TRANSFER_OUT            0x00

#define REQUEST_TYPE_STANDARD           0x00
#define REQUEST_TYPE_CLASS              0x20
#define REQUEST_TYPE_VENDOR             0x40

#define REQUEST_RECIPIENT_DEVICE        0x00
#define REQUEST_RECIPIENT_INTERFACE     0x01
#define REQUEST_RECIPIENT_ENDPOINT      0x02
#define REQUEST_RECIPIENT_OTHER         0x03

	//****************************************************************************
	//
	// Prototypes for functions exported by the DLL.
	//
	//****************************************************************************
	TIVACBULKPLUGIN_API LMUSB_HANDLE InitializeDevice(unsigned short usVID,
		unsigned short usPID,
		LPGUID lpGUID,
		BOOL *pbDriverInstalled);
	TIVACBULKPLUGIN_API LMUSB_HANDLE InitializeDeviceEasy(BOOL *installed);
	TIVACBULKPLUGIN_API LMUSB_HANDLE InitializeDeviceByIndex(unsigned short usVID,
		unsigned short usPID,
		LPGUID lpGUID,
		DWORD dwIndex,
		BOOL bOpenDataEndpoints,
		BOOL *pbDriverInstalled);
	//__stdcall 
	TIVACBULKPLUGIN_API BOOL TerminateDevice(LMUSB_HANDLE hHandle);
	TIVACBULKPLUGIN_API BOOL WriteUSBPacket(LMUSB_HANDLE hHandle,
		unsigned char *pcBuffer,
		unsigned long ulSize,
		unsigned long *pulWritten);
	TIVACBULKPLUGIN_API DWORD ReadUSBPacket(LMUSB_HANDLE hHandle,
		unsigned char *pcBuffer,
		unsigned long ulSize,
		unsigned long *pulRead,
		unsigned long ulTimeoutMs,
		HANDLE hBreak);
	TIVACBULKPLUGIN_API BOOL Endpoint0Transfer(LMUSB_HANDLE hHandle, UCHAR ucRequestType,
		UCHAR ucRequest, USHORT usValue,
		USHORT usIndex, USHORT usLength,
		PUCHAR pucBuffer, PUSHORT pusCount);

	//****************************************************************************
	//
	// Typedefs for each of the exported functions.  This helps if applications
	// want to link the DLL dynamically using LoadLibrary rather than linking
	// directly to the lib file.
	//
	//****************************************************************************
	typedef LMUSB_HANDLE(__stdcall *tInitializeDevice)(unsigned short usVID,
		unsigned short usPID,
		LPGUID lpGUID,
		BOOL *pbDriverInstalled);
	typedef BOOL(__stdcall *tTerminateDevice)(LMUSB_HANDLE hHandle);
	typedef BOOL(__stdcall *tWriteUSBPacket)(LMUSB_HANDLE hHandle,
		unsigned char *pcBuffer,
		unsigned long ulSize,
		unsigned long *pulWritten);
	typedef DWORD(__stdcall *tReadUSBPacket)(LMUSB_HANDLE hHandle,
		unsigned char *pcBuffer,
		unsigned long ulSize,
		unsigned long *pulRead,
		unsigned long ulTimeoutMs,
		HANDLE hBreak);
	typedef DWORD(__stdcall *tEndpoint0Transfer)(LMUSB_HANDLE hHandle,
		UCHAR ucRequestType,
		UCHAR ucRequest, USHORT usValue,
		USHORT usIndex, USHORT usLength,
		PUCHAR pucBuffer,
		PUSHORT pusCount);
