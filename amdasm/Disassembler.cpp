/*
 *  CLRadeonExtender - Unofficial OpenCL Radeon Extensions Library
 *  Copyright (C) 2014 Mateusz Szpakowski
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <CLRX/Config.h>
#include <string>
#include <ostream>
#include <locale>
#include <vector>
#include <algorithm>
#include <CLRX/Utilities.h>
#include <CLRX/AmdBinaries.h>
#include <CLRX/Assembler.h>
#include <CLRX/MemAccess.h>

using namespace CLRX;

ISADisassembler::ISADisassembler(Disassembler& disassembler_)
        : disassembler(disassembler_)
{ }

ISADisassembler::~ISADisassembler()
{ }

void ISADisassembler::setInput(size_t inputSize, const cxbyte* input)
{
    this->inputSize = inputSize;
    this->input = input;
}

GCNDisassembler::GCNDisassembler(Disassembler& disassembler)
        : ISADisassembler(disassembler)
{
}

GCNDisassembler::~GCNDisassembler()
{ }

void GCNDisassembler::beforeDisassemble()
{
}

size_t GCNDisassembler::disassemble(size_t maxSize, char* buffer)
{
    return 0;
}

/* helpers for main Disassembler class */

struct GPUDeviceCodeEntry
{
    uint16_t elfMachine;
    GPUDeviceType deviceType;
};

static const GPUDeviceCodeEntry gpuDeviceCodeTable[6] =
{
    { 0x3ff, GPUDeviceType::CAPE_VERDE },
    { 0x3fe, GPUDeviceType::PITCAIRN },
    { 0x3fd, GPUDeviceType::TAHITI },
    { 0x402, GPUDeviceType::OLAND },
    { 0x403, GPUDeviceType::BONAIRE },
    { 0x408, GPUDeviceType::HAWAII }
};

static const char* gpuDeviceNameTable[7] =
{
    "UNDEFINED",
    "CapeVerde",
    "Pitcairn",
    "Tahiti",
    "Oland",
    "Bonaire",
    "Hawaii"
};

template<typename AmdMainBinary>
static const DisasmInput* getDisasmInputFromBinary(const AmdMainBinary& binary)
{
    DisasmInput* input = new DisasmInput;
    cxuint index = 0;
    const uint16_t elfMachine = ULEV(binary.getHeader().e_machine);
    input->is64BitMode = (binary.getHeader().e_ident[EI_CLASS] == ELFCLASS64);
    const cxuint entriesNum = sizeof(gpuDeviceCodeTable)/sizeof(GPUDeviceCodeEntry);
    for (index = 0; index < entriesNum; index++)
        if (gpuDeviceCodeTable[index].elfMachine == elfMachine)
            break;
    if (entriesNum == index)
        throw Exception("Cant determine GPU device type");
    input->deviceType = gpuDeviceCodeTable[index].deviceType;
    input->metadata.compileOptions = binary.getCompileOptions();
    input->metadata.driverInfo = binary.getDriverInfo();
    input->globalDataSize = binary.getGlobalDataSize();
    input->globalData = binary.getGlobalData();
    const size_t kernelInfosNum = binary.getKernelInfosNum();
    const size_t kernelHeadersNum = binary.getKernelHeadersNum();
    const size_t innerBinariesNum = binary.getInnerBinariesNum();
    input->kernelInputs.resize(kernelInfosNum);
    
    for (cxuint i = 0; i < kernelInfosNum; i++)
    {
        const KernelInfo& kernelInfo = binary.getKernelInfo(i);
        const AmdInnerGPUBinary32* innerBin = nullptr;
        if (i < innerBinariesNum)
            innerBin = &binary.getInnerBinary(i);
        if (innerBin == nullptr || kernelInfo.kernelName != kernelInfo.kernelName)
        {   // fallback if not in order
            try
            { innerBin = &binary.getInnerBinary(kernelInfo.kernelName.c_str()); }
            catch(const Exception& ex)
            { innerBin = nullptr; }
        }
        DisasmKernelInput& kernelInput = input->kernelInputs[i];
        kernelInput.metadataSize = binary.getMetadataSize(i);
        kernelInput.metadata = binary.getMetadata(i);
        
        // kernel header
        kernelInput.headerSize = 0;
        kernelInput.header = nullptr;
        const AmdGPUKernelHeader* khdr = nullptr;
        if (i < kernelHeadersNum)
            khdr = &binary.getKernelHeaderEntry(i);
        if (khdr == nullptr || khdr->kernelName != kernelInfo.kernelName)
        {   // fallback if not in order
            try
            { khdr = &binary.getKernelHeaderEntry(kernelInfo.kernelName.c_str()); }
            catch(const Exception& ex) // failed
            { khdr = nullptr; }
        }
        if (khdr != nullptr)
        {
            kernelInput.headerSize = khdr->size;
            kernelInput.header = khdr->data;
        }
        
        kernelInput.kernelName = kernelInfo.kernelName;
        
        kernelInput.codeSize = kernelInput.dataSize = 0;
        kernelInput.code = kernelInput.data = nullptr;
        
        if (innerBin != nullptr)
        {   // if innerBinary exists
            bool codeFound = false;
            bool dataFound = false;
            
            for (cxint j = innerBin->getSectionHeadersNum()-1; j >= 0; j--)
            {
                const char* secName = innerBin->getSectionName(j);
                if (!codeFound && ::strcmp(secName, ".text") == 0)
                {   // if found last .text
                    const auto& shdr = innerBin->getSectionHeader(j);
                    kernelInput.codeSize = ULEV(shdr.sh_size);
                    kernelInput.code = innerBin->getSectionContent(j);
                    codeFound = true;
                }
                else if (!dataFound && ::strcmp(secName, ".data") == 0)
                {   // if found last .data
                    const auto& shdr = innerBin->getSectionHeader(j);
                    kernelInput.dataSize = ULEV(shdr.sh_size);
                    kernelInput.data = innerBin->getSectionContent(j);
                    dataFound = true;
                }
                
                if (codeFound && dataFound)
                    break; // end of finding
            }
        
            kernelInput.calNotes.resize(innerBin->getCALNotesNum());
            for (cxuint j = 0; j < kernelInput.calNotes.size(); j++)
            {
                const CALNoteHeader& calNoteHdr = innerBin->getCALNoteHeader(j);
                AsmCALNote& outCalNote = kernelInput.calNotes[j];
                outCalNote.header.nameSize = ULEV(calNoteHdr.nameSize);
                outCalNote.header.type = ULEV(calNoteHdr.type);
                outCalNote.header.descSize = ULEV(calNoteHdr.descSize);
                std::copy(calNoteHdr.name, calNoteHdr.name+8, outCalNote.header.name);
                outCalNote.data = const_cast<cxbyte*>(innerBin->getCALNoteData(j));
            }
        }
    }
    return input;
}

Disassembler::Disassembler(const AmdMainGPUBinary32& binary, std::ostream& _output,
            cxuint flags) : fromBinary(true), output(_output)
{
    this->flags = flags;
    output.imbue(std::locale::classic());
    output.exceptions(std::ios::failbit | std::ios::badbit);
    input = getDisasmInputFromBinary(binary);
    isaDisassembler = new GCNDisassembler(*this);
}

Disassembler::Disassembler(const AmdMainGPUBinary64& binary, std::ostream& _output,
            cxuint flags) : fromBinary(true), output(_output)
{
    this->flags = flags;
    output.imbue(std::locale::classic());
    output.exceptions(std::ios::failbit | std::ios::badbit);
    input = getDisasmInputFromBinary(binary);
    isaDisassembler = new GCNDisassembler(*this);
}

Disassembler::Disassembler(const DisasmInput* disasmInput, std::ostream& _output,
            cxuint flags) : fromBinary(false), input(disasmInput), output(_output)
{
    this->flags = flags;
    output.imbue(std::locale::classic());
    output.exceptions(std::ios::failbit | std::ios::badbit);
    isaDisassembler = new GCNDisassembler(*this);
}

Disassembler::~Disassembler()
{
    if (fromBinary)
        delete input;
    delete isaDisassembler;
}

static void printDisasmData(size_t size, const cxbyte* data, std::ostream& output,
                bool secondAlign = false)
{
    char buf[6];
    const char* linePrefix = "    .byte ";
    if (secondAlign)
        linePrefix = "        .byte ";
    for (size_t p = 0; p < size; p++)
    {
        if ((p & 15) == 0)
            output << linePrefix;
        u32tocstrCStyle(data[p], buf, 6, 16, 2);
        output << buf;
        if (p+1 < size)
            output << ",";
        if ((p & 15) == 15 || p+1 >= size)
            output << '\n';
    }
}

static void printDisasmDataU32(size_t size, const uint32_t* data, std::ostream& output,
                bool secondAlign = false)
{
    char buf[12];
    const char* linePrefix = "    .dword ";
    if (secondAlign)
        linePrefix = "        .dword ";
    for (size_t p = 0; p < size; p++)
    {
        if ((p & 3) == 0)
            output << linePrefix;
        u32tocstrCStyle(data[p], buf, 12, 16, 8);
        output << buf;
        if (p+1 < size)
            output << ",";
        if ((p & 3) == 3 || p+1 >= size)
            output << '\n';
    }
}


static void printDisasmLongString(size_t size, const char* data, std::ostream& output,
            bool secondAlign = false)
{
    const char* linePrefix = "    .string \"";
    if (secondAlign)
        linePrefix = "        .string \"";
    for (size_t pos = 0; pos < size; )
    {
        const size_t end = std::min(pos+size_t(80), size);
        const size_t oldPos = pos;
        while (pos < end && data[pos] != '\n') pos++;
        if (pos < end && data[pos] == '\n') pos++; // embrace newline
        output << linePrefix <<
                escapeStringCStyle(pos-oldPos, data+oldPos) << "\"\n";
    }
}

static const char* disasmCALNoteNamesTable[] =
{
    ".proginfo",
    ".inputs",
    ".outputs",
    ".condout",
    ".floatconsts",
    ".intconsts",
    ".boolconsts",
    ".earlyexit",
    ".globalbuffers",
    ".constantbuffers",
    ".inputsamplers",
    ".persistentbuffers",
    ".scratchbuffers",
    ".subconstantbuffers",
    ".uavmailboxsize",
    ".uav",
    ".uavopmask"
};

void Disassembler::disassemble()
{
    output << ".gpu " << gpuDeviceNameTable[cxuint(input->deviceType)] << "\n" <<
            ((input->is64BitMode) ? ".64bit\n" : ".32bit\n");
    
    if ((flags & DISASM_METADATA) != 0)
        output << ".compile_options \"" <<
                        escapeStringCStyle(input->metadata.compileOptions) << "\"\n" <<
                  ".driver_info \"" <<
                        escapeStringCStyle(input->metadata.driverInfo) << "\"\n";
    
    if ((flags & DISASM_DUMPDATA) != 0 &&
        input->globalData != nullptr && input->globalDataSize != 0)
    {   //
        output << ".data\n";
        printDisasmData(input->globalDataSize, input->globalData, output);
    }
    
    for (const DisasmKernelInput& kinput: input->kernelInputs)
    {
        output << ".kernel \"" << escapeStringCStyle(kinput.kernelName) << "\"\n";
        if ((flags & DISASM_METADATA) != 0)
        {
            if (kinput.header != nullptr && kinput.headerSize != 0)
            {   // if kernel header available
                output << "    .header\n";
                printDisasmData(kinput.headerSize, kinput.header, output, true);
            }
            if (kinput.metadata != nullptr && kinput.metadataSize != 0)
            {   // if kernel metadata available
                output << "    .metadata\n";
                printDisasmLongString(kinput.metadataSize, kinput.metadata, output, true);
            }
        }
        if ((flags & DISASM_DUMPDATA) != 0 &&
            kinput.data != nullptr && kinput.dataSize != 0)
        {   // if kernel data available
            output << "    .kerneldata\n";
            printDisasmData(kinput.dataSize, kinput.data, output, true);
        }
        
        if ((flags & DISASM_CALNOTES) != 0)
            for (const AsmCALNote& calNote: kinput.calNotes)
            {
                char buf[32];
                if (calNote.header.type != 0 && calNote.header.type <= CALNOTE_ATI_MAXTYPE)
                    output << "    " << disasmCALNoteNamesTable[calNote.header.type-1];
                else
                {
                    u32tocstrCStyle(calNote.header.type, buf, 32, 16);
                    output << "    .calnote " << buf;
                }
                
                switch(calNote.header.type)
                {   // handle CAL note types
                    case CALNOTE_ATI_PROGINFO:
                    {
                        output << '\n';
                        const cxuint progInfosNum =
                                calNote.header.descSize/sizeof(CALProgramInfoEntry);
                        const CALProgramInfoEntry* progInfos =
                            reinterpret_cast<const CALProgramInfoEntry*>(calNote.data);
                        for (cxuint k = 0; k < progInfosNum; k++)
                        {
                            const CALProgramInfoEntry& progInfo = progInfos[k];
                            u32tocstrCStyle(ULEV(progInfo.address), buf, 32, 16);
                            output << "        .set " << buf << ", ";
                            u32tocstrCStyle(ULEV(progInfo.value), buf, 32, 16);
                            output << buf << '\n';
                        }
                        /// rest
                        printDisasmData(calNote.header.descSize -
                                progInfosNum*sizeof(CALProgramInfoEntry),
                                calNote.data + progInfosNum*sizeof(CALProgramInfoEntry),
                                output, true);
                        break;
                    }
                    case CALNOTE_ATI_INPUTS:
                    case CALNOTE_ATI_OUTPUTS:
                    case CALNOTE_ATI_GLOBAL_BUFFERS:
                    case CALNOTE_ATI_SCRATCH_BUFFERS:
                    case CALNOTE_ATI_PERSISTENT_BUFFERS:
                        output << '\n';
                        printDisasmDataU32(calNote.header.descSize>>2,
                               reinterpret_cast<const uint32_t*>(calNote.data),
                               output, true);
                        printDisasmData(calNote.header.descSize&3,
                               calNote.data + (calNote.header.descSize&~3U), output, true);
                        break;
                    case CALNOTE_ATI_INT32CONSTS:
                    case CALNOTE_ATI_FLOAT32CONSTS:
                    case CALNOTE_ATI_BOOL32CONSTS:
                    {
                        output << '\n';
                        const cxuint segmentsNum =
                                calNote.header.descSize/sizeof(CALDataSegmentEntry);
                        const CALDataSegmentEntry* segments =
                                reinterpret_cast<const CALDataSegmentEntry*>(calNote.data);
                        for (cxuint k = 0; k < segmentsNum; k++)
                        {
                            const CALDataSegmentEntry& segment = segments[k];
                            u32tocstrCStyle(ULEV(segment.offset), buf, 32);
                            output << "        .segment " << buf << ", ";
                            u32tocstrCStyle(ULEV(segment.size), buf, 32);
                            output << buf << '\n';
                        }
                        /// rest
                        printDisasmData(calNote.header.descSize -
                                segmentsNum*sizeof(CALDataSegmentEntry),
                                calNote.data + segmentsNum*sizeof(CALDataSegmentEntry),
                                output, true);
                        break;
                    }
                    case CALNOTE_ATI_INPUT_SAMPLERS:
                    {
                        output << '\n';
                        const cxuint samplersNum =
                                calNote.header.descSize/sizeof(CALSamplerMapEntry);
                        const CALSamplerMapEntry* samplers =
                                reinterpret_cast<const CALSamplerMapEntry*>(calNote.data);
                        for (cxuint k = 0; k < samplersNum; k++)
                        {
                            const CALSamplerMapEntry& segment = samplers[k];
                            u32tocstrCStyle(ULEV(segment.input), buf, 32);
                            output << "        .sampler " << buf << ", ";
                            u32tocstrCStyle(ULEV(segment.sampler), buf, 32, 16);
                            output << buf << '\n';
                        }
                        /// rest
                        printDisasmData(calNote.header.descSize -
                                samplersNum*sizeof(CALSamplerMapEntry),
                                calNote.data + samplersNum*sizeof(CALSamplerMapEntry),
                                output, true);
                        break;
                    }
                    case CALNOTE_ATI_CONSTANT_BUFFERS:
                    {
                        output << '\n';
                        const cxuint constBufMasksNum =
                                calNote.header.descSize/sizeof(CALConstantBufferMask);
                        const CALConstantBufferMask* constBufMasks =
                            reinterpret_cast<const CALConstantBufferMask*>(calNote.data);
                        for (cxuint k = 0; k < constBufMasksNum; k++)
                        {
                            const CALConstantBufferMask& cbufMask = constBufMasks[k];
                            u32tocstrCStyle(ULEV(cbufMask.index), buf, 32);
                            output << "        .cbmask " << buf << ", ";
                            u32tocstrCStyle(ULEV(cbufMask.size), buf, 32);
                            output << buf << '\n';
                        }
                        /// rest
                        printDisasmData(calNote.header.descSize -
                            constBufMasksNum*sizeof(CALConstantBufferMask),
                            calNote.data + constBufMasksNum*sizeof(CALConstantBufferMask),
                            output, true);
                        break;
                    }
                    case CALNOTE_ATI_EARLYEXIT:
                    case CALNOTE_ATI_UAV_OP_MASK:
                    case CALNOTE_ATI_UAV_MAILBOX_SIZE:
                        if (calNote.header.descSize == 4)
                        {
                            u32tocstrCStyle(ULEV(*reinterpret_cast<const uint32_t*>(
                                        calNote.data)), buf, 32);
                            output << " " << buf << '\n';
                        }
                        else // otherwise if size is not dword
                        {
                            output << '\n';
                            printDisasmData(calNote.header.descSize,
                                    calNote.data, output, true);
                        }
                        break;
                    default:
                        output << '\n';
                        printDisasmData(calNote.header.descSize, calNote.data,
                                        output, true);
                        break;
                }
            }
        
        if (kinput.code != nullptr && kinput.codeSize != 0)
        {   // input kernel code (main disassembly)
        }
    }
    output.flush();
}
