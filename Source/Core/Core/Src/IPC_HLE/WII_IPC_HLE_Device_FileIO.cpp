// Copyright (C) 2003-2008 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include "Common.h"

#include "WII_IPC_HLE_Device_FileIO.h"

// smash bros writes to  /shared2/sys/SYSCONF


// __________________________________________________________________________________________________
//
CWII_IPC_HLE_Device_FileIO::CWII_IPC_HLE_Device_FileIO(u32 _DeviceID, const std::string& _rDeviceName ) 
    : IWII_IPC_HLE_Device(_DeviceID, _rDeviceName)
    , m_pFileHandle(NULL)
    , m_FileLength(0)
    , m_Seek(0)
{
}

// __________________________________________________________________________________________________
//
CWII_IPC_HLE_Device_FileIO::~CWII_IPC_HLE_Device_FileIO()
{
    if (m_pFileHandle != NULL)
    {
        fclose(m_pFileHandle);
        m_pFileHandle = NULL;
    }
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_FileIO::Open(u32 _CommandAddress)  
{ 
    std::string Filename("WII");
    Filename += GetDeviceName();

    m_pFileHandle = fopen(Filename.c_str(), "r+b");

    if (m_pFileHandle != NULL)
    {
        fseek(m_pFileHandle, 0, SEEK_END);
        m_FileLength = ftell(m_pFileHandle);
        rewind(m_pFileHandle);
    }
    else
    {
        //PanicAlert("CWII_IPC_HLE_Device_FileIO: cant open %s", Filename.c_str());
    }

    Memory::Write_U32(GetDeviceID(), _CommandAddress+4);
    return true; 
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_FileIO::Seek(u32 _CommandAddress) 
{
    LOG(WII_IPC_HLE, "FileIO: Seek (Device=%s)", GetDeviceName().c_str());	
    DumpCommands(_CommandAddress);

    PanicAlert("CWII_IPC_HLE_Device_FileIO: Seek");

    u32 ReturnValue = 1;
    Memory::Write_U32(ReturnValue, _CommandAddress + 0x4);

    return true;
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_FileIO::Read(u32 _CommandAddress) 
{    
    u32 ReturnValue = 0;
    u32 Address = Memory::Read_U32(_CommandAddress +0xC);
    u32 Size = Memory::Read_U32(_CommandAddress +0x10);       

    if (m_pFileHandle != NULL)
    {
        fread(Memory::GetPointer(Address), Size, 1, m_pFileHandle);
        ReturnValue = Size;
        LOG(WII_IPC_HLE, "FileIO reads from %s (Addr=0x%08x Size=0x%x)", GetDeviceName().c_str(), Address, Size);	
    }
    else
    {
        LOG(WII_IPC_HLE, "FileIO failed to read from %s (Addr=0x%08x Size=0x%x) - file not open", GetDeviceName().c_str(), Address, Size);	
    }

    Memory::Write_U32(ReturnValue, _CommandAddress + 0x4);

    return true;
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_FileIO::Write(u32 _CommandAddress) 
{        
    LOG(WII_IPC_HLE, "FileIO: Write (Device=%s)", GetDeviceName().c_str());	
    DumpCommands(_CommandAddress);

    //PanicAlert("CWII_IPC_HLE_Device_FileIO: Write (Device=%s)", GetDeviceName().c_str());

    u32 ReturnValue = 1;
    Memory::Write_U32(ReturnValue, _CommandAddress + 0x4);

    return true;
}

// __________________________________________________________________________________________________
//
bool 
CWII_IPC_HLE_Device_FileIO::IOCtl(u32 _CommandAddress) 
{
    LOG(WII_IPC_HLE, "FileIO: IOCtl (Device=%s)", GetDeviceName().c_str());	
    DumpCommands(_CommandAddress);

    u32 Parameter =  Memory::Read_U32(_CommandAddress + 0xC);

    u32 BufferIn =  Memory::Read_U32(_CommandAddress + 0x10);
    u32 BufferInSize =  Memory::Read_U32(_CommandAddress + 0x14);
    u32 BufferOut = Memory::Read_U32(_CommandAddress + 0x18);
    u32 BufferOutSize = Memory::Read_U32(_CommandAddress + 0x1C);

    switch(Parameter)
    {
    case ISFS_IOCTL_GETFILESTATS:
        {
            LOG(WII_IPC_HLE, "FileIO: ISFS_IOCTL_GETFILESTATS");
            LOG(WII_IPC_HLE, "Length: %i   Seek: %i", m_FileLength, m_Seek);

            Memory::Write_U32(m_FileLength, BufferOut);
            Memory::Write_U32(m_Seek, BufferOut+4);
        }
        break;

    default:
        {
            PanicAlert("CWII_IPC_HLE_Device_FileIO: Parameter %i", Parameter);
        }
        break;

    }

    // Return Value
    u32 ReturnValue = 0;  // no error
    Memory::Write_U32(ReturnValue, _CommandAddress + 0x4);

    CCPU::Break();

    return true;
}
