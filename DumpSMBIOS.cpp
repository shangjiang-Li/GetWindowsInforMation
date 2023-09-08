// DumpSMBIOS.cpp : Defines the entry point for the console application.
//
#include "DumpSMBIOS.h"
#include <sysinfoapi.h>
#include <QObject>
#include <QDebug>
#include <winnls.h>
const char* LocateStringA(const char* str, UINT i)
{
    static const char strNull[] = "Null String";

    if (0 == i || 0 == *str) return strNull;

    while (--i)
    {
        str += strlen((char*)str) + 1;
    }
    return str;
}
const wchar_t* LocateStringW(const char* str, UINT i)
{
    static wchar_t buff[2048];
    const char *pStr = LocateStringA(str, i);
    RtlSecureZeroMemory(buff, sizeof(buff));
    const int convSize = MultiByteToWideChar(CP_OEMCP, 0, pStr, (int) strlen(pStr), buff, sizeof(buff) / 2);
    if (convSize > 0)
        return buff;
    return NULL;
}
const char* toPointString(void* p)
{
    return (char*)p + ((PSMBIOSHEADER)p)->Length;
}
bool DumpSMBIOS::ProcBIOSInfo(void* p)
{
	PBIOSInfo pBIOS = (PBIOSInfo)p;
       const char *str = toPointString(p);
       qDebug()<<"Vendor: "<<QString::fromStdWString(LocateStringW(str, pBIOS->Vendor));
        biosver = QString::fromStdWString(LocateStringW(str, pBIOS->Version));
        qDebug()<<"BIOS Starting Segment: "<< pBIOS->StartingAddrSeg;
        qDebug()<<"Release Date: "<< QString::fromStdWString(LocateStringW(str, pBIOS->ReleaseDate));
        qDebug()<<"Image Size: "<<(pBIOS->ROMSize + 1) * 64<<"K";
    if (pBIOS->Header.Length > 0x14)
    {   // for spec v2.4 and later
        qDebug()<<"System BIOS version: "<< pBIOS->MajorRelease<<"."<< pBIOS->MinorRelease;
        qDebug()<<"EC Firmware version: "<< pBIOS->ECFirmwareMajor<<"."<<pBIOS->ECFirmwareMinor;
    }
	return true;
}

bool DumpSMBIOS::ProcSysInfo(void* p)
{
	PSystemInfo pSystem = (PSystemInfo)p;

//	_tprintf(TEXT("Manufacturer: %s\n"), LocateString(str, pSystem->Manufacturer));
//	_tprintf(TEXT("Product Name: %s\n"), LocateString(str, pSystem->ProductName));
//	_tprintf(TEXT("Version: %s\n"), LocateString(str, pSystem->Version));
//	_tprintf(TEXT("Serial Number: %s\n"), LocateString(str, pSystem->SN));
	// for v2.1 and later
	if (pSystem->Header.Length > 0x08)
	{
//		_tprintf(TEXT("UUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n"),
//			pSystem->UUID[0], pSystem->UUID[1], pSystem->UUID[2], pSystem->UUID[3],
//			pSystem->UUID[4], pSystem->UUID[5], pSystem->UUID[6], pSystem->UUID[7],
//			pSystem->UUID[8], pSystem->UUID[9], pSystem->UUID[10], pSystem->UUID[11],
//			pSystem->UUID[12], pSystem->UUID[13], pSystem->UUID[14], pSystem->UUID[15]);
	}

	// process wake-up type, for spec2.1 or later
	if (pSystem->WakeUpType >= 0 && pSystem->WakeUpType < WAKEUP_TYPE_COUNT) 
	{	
//		const TCHAR* WakeupTypeStrings[] =
//		{
//			TEXT("Reserved"), TEXT("Other"), TEXT("Unknown"), TEXT("APM Timer"), TEXT("Modem Ring"),
//			TEXT("LAN Remote"),TEXT("Power Switch") , TEXT("PCI PME#"), TEXT("AC Power Restored")
//		};
//		_tprintf(TEXT("Wake-up Type: %s\n"), WakeupTypeStrings[pSystem->WakeUpType]);
	}

	if (pSystem->Header.Length > 0x19)
	{
		// fileds for spec. 2.4
//		_tprintf(TEXT("SKU Number: %s\n"), LocateString(str, pSystem->SKUNumber));
//		_tprintf(TEXT("Family: %s\n"), LocateString(str, pSystem->Family));
	}
	return true;
}

bool DumpSMBIOS::ProcBoardInfo(void* p)
{
	PBoardInfo pBoard = (PBoardInfo)p;
	const char HandleCount = pBoard->NumObjHandle;

	return true;
}

bool DumpSMBIOS::ProcSystemEnclosure(void* p)
{
	PSystemEnclosure pSysEnclosure = (PSystemEnclosure)p;



	return true;
}

bool DumpSMBIOS::ProcProcessorInfo(void* p)
{
	PProcessorInfo	pProcessor = (PProcessorInfo)p;
	const TCHAR *ProcessTypeStrings[] = 
	{
		TEXT("Other"), TEXT("Unknown"), TEXT("Central Processor"),
		TEXT("Math Processor"), TEXT("DSP Processor"), TEXT("Video Processor"),
	};

	return true;
}

bool DumpSMBIOS::ProcMemModuleInfo(void* p)
{
	PMemModuleInfo	pMemModule = (PMemModuleInfo)p;


	return true;
}

bool DumpSMBIOS::ProcCacheInfo(void *p)
{
	PCacheInfo	pCache = (PCacheInfo)p;

	return true;
}

std::variant<bool,const QStringList> DumpSMBIOS::ProcOEMString(void* p)
{
    POemString pOemString = (POemString)p;
    const char *str = toPointString(p);
    QStringList OEMString;
    if (!OEMString.isEmpty())
        OEMString.clear();
    for(int i = 1; i <= pOemString->Count; i++)
    {
        if (QString::fromStdWString(LocateStringW(str, i))==" ")
                continue;
        else
            OEMString<<QString::fromStdWString(LocateStringW(str, i));
    }
    if (OEMString.isEmpty())
          return false;
    else return OEMString;

}

bool DumpSMBIOS::ProcMemoryDevice(void* p)
{
	PMemoryDevice pMD = (PMemoryDevice)p;

	if (pMD->Header.Length > 0x15)
	{

	}
	if (pMD->Header.Length == 0x54)
	{

	}

	return true;
}

bool DumpSMBIOS::ProcMemoryArrayMappedAddress(void*	p)
{
	PMemoryArrayMappedAddress pMAMA = (PMemoryArrayMappedAddress)p;


	return true;
}

const TCHAR* DumpSMBIOS::getBuiltinPointDeviceTypeString(const UCHAR type)
{
	const TCHAR* typeString[10] = {
		TEXT("Unsupport Type"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Mouse"),
		TEXT("Track Ball"),
		TEXT("Track Point"),
		TEXT("Glide Point"),
		TEXT("Touch Pad"),
		TEXT("Touch Screen"),
		TEXT("Optical Sensor"),
	};

	if ((type >= 1) && (type <= 9))
		return typeString[type];

	return typeString[0];
}

const TCHAR* DumpSMBIOS::getBuiltinPointDeviceInterfaceString(const UCHAR Interface)
{
	const TCHAR* interfaceString[12] = {
		TEXT("Unsupport Interface"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Serial"),
		TEXT("PS/2"),
		TEXT("Infrared"),
		TEXT("HP-HIL"),
		TEXT("Bus mouse"),
		TEXT("Apple Desktop Bus"),
		TEXT("Bus mouse DB-9"),
		TEXT("Bus mouse micro-DIN"),
		TEXT("USB"),
	};

	if ((Interface >= 1) && Interface <= 8)
		return interfaceString[Interface];
	else if ((Interface >= 0xA0) && (Interface <= 0xA2))
		return interfaceString[Interface - 0xA0 + 9];

	return interfaceString[0];
}

bool DumpSMBIOS::ProcBuiltinPointDevice(void* p)
{
	PBuiltinPointDevice pBPD = (PBuiltinPointDevice)p;
	return true;
}

const TCHAR* DumpSMBIOS::getBatteryChemistry(const UCHAR chemistry)
{
	const TCHAR* typeString[9] = {
		TEXT("Unsupport type"),
		TEXT("Other"),
		TEXT("Unknown"),
		TEXT("Lead Acid"),
		TEXT("Nickel Cadmium"),
		TEXT("Nickel metal hydride"),
		TEXT("Lithium-ion"),
		TEXT("Zinc air"),
		TEXT("Lithium Polyme")
	};

	if ((chemistry >= 1) && (chemistry <= 8))
	{
		return typeString[chemistry];
	}

	return typeString[0];
}

bool DumpSMBIOS::ProcPortableBattery(void* p)
{
	PPortableBattery pPB = (PPortableBattery)p;
	return true;
}

void DumpSMBIOS::DumpSMBIOSStruct(void *Addr, UINT Len)
{
    LPBYTE p = (LPBYTE)(Addr);
    const LPBYTE lastAddress = p + Len;
    PSMBIOSHEADER pHeader;

    for (;;) {
        pHeader = (PSMBIOSHEADER)p;
        switch (pHeader->Type) {
                case 0: ProcBIOSInfo(pHeader);break;
                case 1: ProcSysInfo(pHeader);break;
                case 2: ProcBoardInfo(pHeader);break;
                case 3: ProcSystemEnclosure(pHeader);break;
                case 4: ProcProcessorInfo(pHeader);break;
                case 6: ProcMemModuleInfo(pHeader);break;
                case 7: ProcCacheInfo(pHeader);break;
                case 11: {
                                std::variant<bool,const  QStringList>  oemstring = ProcOEMString(pHeader);
                                if (std::get_if<const QStringList> (&oemstring)) {
                                      foreach (QString var, std::get<const QStringList>(oemstring)) {
                                          if (var.contains("BUILDID")) {
                                                    if (var.split("#").count()>2) {
                                                        buildid = var.split("#")[1];
                                                    }
                                          }
                                    }
                                };
                }break;
                case 17: ProcMemoryDevice(pHeader);break;
                case 19: ProcMemoryArrayMappedAddress(pHeader);break;
                case 21: ProcBuiltinPointDevice(pHeader);break;
                case 22: ProcPortableBattery(pHeader);break;
                default:
                    break;
        }
        if ((pHeader->Type == 127) && (pHeader->Length == 4))
            break; // last avaiable tables
        LPBYTE nt = p + pHeader->Length; // point to struct end
        while (0 != (*nt | *(nt + 1))) nt++; // skip string area
        nt += 2;
        if (nt >= lastAddress)
            break;
        p = nt;
    }
}

bool DumpSMBIOS::DumpsmbiosMain()
{
    DWORD needBufferSize = 0;
    const DWORD Signature = 'RSMB';
    LPBYTE pBuff = NULL;
    needBufferSize = GetSystemFirmwareTable(Signature, 0, NULL, 0);
    pBuff = (LPBYTE) malloc(needBufferSize);
    if (pBuff)
    {
        GetSystemFirmwareTable(Signature, 0, pBuff, needBufferSize);
        const PRawSMBIOSData pDMIData = (PRawSMBIOSData)pBuff;
        qDebug()<<"SMBIOS version:"<< pDMIData->SMBIOSMajorVersion<<"."<<pDMIData->SMBIOSMinorVersion;
        qDebug()<<"DMI Revision:"<< pDMIData->DmiRevision;
        qDebug()<<"Total length: "<<pDMIData->Length;
        qDebug()<<"DMI at address"<<&pDMIData->SMBIOSTableData;
        DumpSMBIOSStruct(&(pDMIData->SMBIOSTableData), pDMIData->Length);
    }
    else
        return false;
    if (pBuff)
        free(pBuff);
    return true;
}

