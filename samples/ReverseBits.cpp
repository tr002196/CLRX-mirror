/*
 *  CLRadeonExtender - Unofficial OpenCL Radeon Extensions Library
 *  Copyright (C) 2014-2015 Mateusz Szpakowski
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

#include <cstdio>
#include <iostream>
#include <memory>
#include <CLRX/utils/Utilities.h>
#include "CLUtils.h"

using namespace CLRX;

static const char* reverseBitsSource = R"ffDXD(# ReverseBits example
.ifarch gcn1.2
    .error "Unsupported GCN1.2 architecture"
.endif
.globaldata     # const data (conversion table)
    .byte 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0
    .byte 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0
    .byte 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8
    .byte 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8
    .byte 0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4
    .byte 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4
    .byte 0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec
    .byte 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc
    .byte 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2
    .byte 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2
    .byte 0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea
    .byte 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa
    .byte 0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6
    .byte 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6
    .byte 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee
    .byte 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe
    .byte 0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1
    .byte 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1
    .byte 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9
    .byte 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9
    .byte 0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5
    .byte 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5
    .byte 0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed
    .byte 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd
    .byte 0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3
    .byte 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3
    .byte 0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb
    .byte 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb
    .byte 0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7
    .byte 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7
    .byte 0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef
    .byte 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
.iffmt amd    # if AMD Catalyst
.kernel reverseBits
    .config
        .dims x             # uses only one dimensions, required
        .uavid 11           # force 11 uavid
        .arg n, uint                        # uint n
        .arg input, uchar*, global, const   # const global uint* input
        .arg output, uchar*, global         # global uint* output
        .userdata ptr_uav_table, 0, 2, 2            # uav (buffer) table
        .userdata imm_const_buffer, 0, 4, 4         # kernel setup const buffer
        .userdata imm_const_buffer, 1, 8, 4         # kernel args const buffer
        .useconstdata                       # use global const buffer (const data)
    .text
    .if32 # 32-bit addressing
        s_buffer_load_dword s0, s[4:7], 4           # local_size(0)
        s_buffer_load_dword s1, s[4:7], 0x18        # global_offset(0)
        s_buffer_load_dword s7, s[4:7], 0x20        # constant buffer offset
        s_buffer_load_dword s4, s[8:11], 0          # n
        s_buffer_load_dword s5, s[8:11], 4          # input buffer offset
        s_buffer_load_dword s6, s[8:11], 8          # output buffer offset
        s_waitcnt  lgkmcnt(0)
        s_mul_i32  s0, s0, s12              # local_size(0)*group_id(0)
        s_add_u32  s0, s1, s0               # + global_offset(0)
        v_add_i32  v0, vcc, s0, v0          # global_id(0)
        v_cmp_gt_u32  vcc, s4, v0           # n<global_id(0)
        s_and_saveexec_b64  s[0:1], vcc     # deactive thread with id(0)>=n
        s_cbranch_execz  end                # skip if no active thread
        s_load_dwordx4 s[8:11], s[2:3], 0x60        # load input buffer descriptor
        s_load_dwordx4 s[12:15], s[2:3], 0x50       # load constant buffer descriptor
        s_waitcnt  lgkmcnt(0)
        buffer_load_ubyte  v1, v0, s[8:11], s5 offen    # load ubyte from input
        s_waitcnt  vmcnt(0)
        s_load_dwordx4 s[8:11], s[2:3], 0x68        # load output buffer
        # convert byte (convert table in global const buffer)
        buffer_load_ubyte  v1, v1, s[12:15], s7 offen
        s_waitcnt  vmcnt(0) & lgkmcnt(0)            # wait for result and descriptor
        buffer_store_byte  v1, v0, s[8:11], s6 offen  # write byte to output
    .else # 64-bit addressing mode
        s_buffer_load_dword s0, s[4:7], 4           # local_size(0)
        s_buffer_load_dword s1, s[4:7], 0x18        # global_offset(0)
        s_buffer_load_dwordx2 s[6:7], s[4:7], 0x20  # constant buffer offset
        s_buffer_load_dword s4, s[8:11], 0          # n
        s_waitcnt  lgkmcnt(0)
        s_mul_i32  s0, s0, s12              # local_size(0)*group_id(0)
        s_add_u32  s0, s1, s0               # + global_offset(0)
        v_add_i32  v0, vcc, s0, v0          # global_id(0)
        v_cmp_gt_u32  vcc, s4, v0           # n<global_id(0)
        s_and_saveexec_b64  s[0:1], vcc     # deactive thread with id(0)>=n
        s_cbranch_execz  end                # skip if no active thread
        s_load_dwordx4 s[8:11], s[2:3], 0x60        # load input buffer descriptor
        s_load_dwordx4 s[12:15], s[2:3], 0x50       # load constant buffer descriptor
        s_buffer_load_dwordx2 s[0:1], s[8:11], 4  # input buffer offset
        s_buffer_load_dwordx2 s[4:5], s[8:11], 8  # output buffer offset
        s_waitcnt  lgkmcnt(0)
        v_add_i32  v2, vcc, s0, v0          # v[2:3] - input_offset+global_id(0)
        v_mov_b32 v3, s1                    # move to vector reg
        v_addc_u32  v3, vcc, v3, 0, vcc     # v_addc_u32 with only vector regs
        s_waitcnt  lgkmcnt(0)
        buffer_load_ubyte  v1, v[2:3], s[8:11], 0 addr64 # load ubyte from input
        s_waitcnt  vmcnt(0)
        s_load_dwordx4 s[8:11], s[2:3], 0x68        # load output buffer
        v_add_i32  v4, vcc, s6, v1          # v[4:5] - constbuf_offset+char
        v_mov_b32 v5, s7                    # move to vector reg
        v_addc_u32  v5, vcc, v5, 0, vcc     # v_addc_u32 with only vector regs
        # convert byte (convert table in global const buffer)
        buffer_load_ubyte  v1, v[4:5], s[12:15], 0 addr64
        v_add_i32  v2, vcc, s4, v0          # v[2:3] - output_offset+global_id(0)
        v_mov_b32 v3, s5                    # move to vector reg
        v_addc_u32  v3, vcc, v3, 0, vcc     # v_addc_u32 with only vector regs
        s_waitcnt  vmcnt(0) & lgkmcnt(0)            # wait for result and descriptor
        buffer_store_byte  v1, v[2:3], s[8:11], 0 addr64 # write byte to output
    .endif
end:
        s_endpgm
.else   # GalliumCompute code
.endif
)ffDXD";

class ReverseBits: public CLFacade
{
private:
    const Array<cxbyte>& inData;
    cl_mem input, output;
public:
    ReverseBits(cl_uint deviceIndex, const Array<cxbyte>& input);
    ~ReverseBits() = default;
    
    void run();
};

ReverseBits::ReverseBits(cl_uint deviceIndex, const Array<cxbyte>& _inData)
            : CLFacade(deviceIndex, reverseBitsSource, "reverseBits"), inData(_inData)
{
    cl_int error;
    input = clCreateBuffer(context, CL_MEM_READ_ONLY, inData.size(), nullptr, &error);
    if (error != CL_SUCCESS)
        throw CLError(error, "clCreateBuffer");
    memObjects.push_back(input);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, inData.size(), nullptr, &error);
    if (error != CL_SUCCESS)
        throw CLError(error, "clCreateBuffer");
    memObjects.push_back(output);
}

void ReverseBits::run()
{
    const size_t size = inData.size();
    cl_int error;
    error = clEnqueueWriteBuffer(queue, input, CL_TRUE, 0, size, inData.data(),
                         0, nullptr, nullptr);
    if (error != CL_SUCCESS)
        throw CLError(error, "clEnqueueWriteBuffer");
    {   // zeroing buffer
        std::unique_ptr<cxbyte[]> filledData(new cxbyte[inData.size()]);
        std::fill(filledData.get(), filledData.get() + size, cxbyte(0));
        error = clEnqueueWriteBuffer(queue, output, CL_TRUE, 0, size, filledData.get(),
                         0, nullptr, nullptr);
        if (error != CL_SUCCESS)
            throw CLError(error, "clEnqueueWriteBuffer");
    }
    
    cl_uint argSize = size;
    clSetKernelArg(kernels[0], 0, sizeof(cl_uint), &argSize);
    clSetKernelArg(kernels[0], 1, sizeof(cl_mem), &input);
    clSetKernelArg(kernels[0], 2, sizeof(cl_mem), &output);
    
    /// execute kernel
    size_t workGroupSize, workGroupSizeMul;
    getKernelInfo(kernels[0], workGroupSize, workGroupSizeMul);
    
    size_t workSize = (size+workGroupSize-1)/workGroupSize*workGroupSize;
    std::cout << "WorkSize: " << workSize << ", LocalSize: " << workGroupSize << std::endl;
    callNDRangeKernel(kernels[0], 1, nullptr, &workSize, &workGroupSize);
    
    std::unique_ptr<cxbyte[]> outData(new cxbyte[size]);
    /// read output buffer
    error = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, size, outData.get(),
                    0, nullptr, nullptr);
    if (error != CL_SUCCESS)
        throw CLError(error, "clEnqueueReadBuffer");
    
    /*for (size_t i = 0; i < size; i++)
    {   // verifying
        const cxbyte in = inData[i];
        const cxbyte expected = ((in>>7)&1) | ((in>>5)&2) | ((in>>3)&4) | ((in>>1)&8) |
                ((in<<1)&16) | ((in<<3)&32) | ((in<<5)&64) | ((in<<7)&128);
        if (expected != outData[i])
        {
            std::cerr << i << ": " << cl_uint(expected) << cl_uint(outData[i]) << std::endl;
            throw Exception("Data mismatch!");
        }
    }*/
    // print result
    for (size_t i = 0; i < size; i+=16)
    {
        printf("%08x:", cl_uint(i));
        for (size_t j = i; j < size && j < i+16; j++)
            printf(" %02hhx", outData[j]);
        putchar('\n');
    }
    fflush(stdout);
}

int main(int argc, const char** argv)
try
{
    cl_uint deviceIndex = 0;
    const char* end;
    if (argc >= 2)
        deviceIndex = cstrtovCStyle<cl_uint>(argv[1], nullptr, end);
    Array<cxbyte> data;
    if (argc >= 3)
        data = loadDataFromFile(argv[2]);
    else
        data = loadDataFromFile("input");
    
    ReverseBits reverseBits(deviceIndex, data);
    reverseBits.run();
    return 0;
}
catch(const std::exception& ex)
{
    std::cerr << ex.what() << std::endl;
    return 1;
}
catch(...)
{
    std::cerr << "unknown exception!" << std::endl;
    return 1;
}
