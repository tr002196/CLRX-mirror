/*
 *  CLRadeonExtender - Unofficial OpenCL Radeon Extensions Library
 *  Copyright (C) 2014-2018 Mateusz Szpakowski
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
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>
#include <CLRX/utils/Containers.h>
#include <CLRX/amdbin/ROCmBinaries.h>
#include "../TestUtils.h"

using namespace CLRX;

static void testMsgPackBytes()
{
    const cxbyte tc0[2] = { 0x91, 0xc0 };
    const cxbyte* dataPtr;
    // parseNil
    dataPtr = tc0;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc0));
        arrParser.parseNil();
        assertValue("MsgPack0", "tc0: DataPtr", dataPtr, tc0 + sizeof(tc0));
    }
    dataPtr = tc0;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc0)-1);
        assertCLRXException("MsgPack0", "tc0_1.Ex", "MsgPack: Can't parse nil value",
                    [&arrParser]() { arrParser.parseNil(); });
        assertValue("MsgPack0", "tc0_1.DataPtr", dataPtr, tc0 + sizeof(tc0)-1);
    }
    
    // parseBool
    const cxbyte tc1[2] = { 0x91, 0xc2 };
    dataPtr = tc1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc1));
        assertTrue("MsgPack0", "tc1.value0", !arrParser.parseBool());
        assertValue("MsgPack0", "tc1.DataPtr", dataPtr, tc1 + sizeof(tc1));
    }
    dataPtr = tc1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc1)-1);
        assertCLRXException("MsgPack0", "tc1_1.Ex", "MsgPack: Can't parse bool value",
                    [&arrParser]() { arrParser.parseBool(); });
        assertValue("MsgPack0", "tc1_1.DataPtr", dataPtr, tc1 + sizeof(tc1)-1);
    }
    
    const cxbyte tc2[2] = { 0x91, 0xc3 };
    dataPtr = tc2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc2));
        assertTrue("MsgPack0", "tc2.value0", arrParser.parseBool());
        assertValue("MsgPack0", "tc2.DataPtr", dataPtr, tc2 + sizeof(tc2));
    }
    
    dataPtr = tc0;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc0));
        assertCLRXException("MsgPack0", "tc3.Ex", "MsgPack: Can't parse bool value",
                    [&arrParser]() { arrParser.parseBool(); });
        assertValue("MsgPack0", "tc3.DataPtr", dataPtr, tc0 + sizeof(tc0)-1);
    }
    
    // parseInteger
    const cxbyte tc4[2] = { 0x91, 0x21 };
    dataPtr = tc4;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc4));
        assertValue("MsgPack0", "tc4.value0", uint64_t(33),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc4.DataPtr", dataPtr, tc4 + sizeof(tc4));
    }
    dataPtr = tc4;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc4)-1);
        assertCLRXException("MsgPack0", "tc4_1.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc4_1.DataPtr", dataPtr, tc4 + sizeof(tc4)-1);
    }
    const cxbyte tc5[2] = { 0x91, 0x5b };
    dataPtr = tc5;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc5));
        assertValue("MsgPack0", "tc5.value0", uint64_t(91),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc5.DataPtr", dataPtr, tc5 + sizeof(tc5));
    }
    const cxbyte tc6[2] = { 0x91, 0xe9 };
    dataPtr = tc6;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc6));
        assertValue("MsgPack0", "tc6.value0", uint64_t(-23),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc6.DataPtr", dataPtr, tc6 + sizeof(tc6));
    }
    const cxbyte tc6_1[2] = { 0x91, 0xe0 };
    dataPtr = tc6_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc6_1));
        assertValue("MsgPack0", "tc6_1.value0", uint64_t(-32),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc6_1.DataPtr", dataPtr, tc6_1 + sizeof(tc6_1));
    }
    // longer integers
    const cxbyte tc7[3] = { 0x91, 0xcc, 0x4d };
    dataPtr = tc7;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc7));
        assertValue("MsgPack0", "tc7.value0", uint64_t(77),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc7.DataPtr", dataPtr, tc7 + sizeof(tc7));
    }
    const cxbyte tc7_1[3] = { 0x91, 0xcc, 0xba };
    dataPtr = tc7_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc7_1));
        assertValue("MsgPack0", "tc7_1.value0", uint64_t(0xba),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc7_1.DataPtr", dataPtr, tc7_1 + sizeof(tc7_1));
    }
    const cxbyte tc7_2[2] = { 0x91, 0xcc };
    dataPtr = tc7_2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc7_2));
        assertCLRXException("MsgPack0", "tc7_2.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc7_2.DataPtr", dataPtr, tc7_2 + sizeof(tc7_2));
    }
    const cxbyte tc8[3] = { 0x91, 0xd0, 0x4d };
    dataPtr = tc8;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc8));
        assertValue("MsgPack0", "tc8.value0", uint64_t(77),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc8.DataPtr", dataPtr, tc8 + sizeof(tc8));
    }
    const cxbyte tc8_1[3] = { 0x91, 0xd0, 0xba };
    dataPtr = tc8_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc8_1));
        assertValue("MsgPack0", "tc8_1.value0", uint64_t(-70),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc8_1.DataPtr", dataPtr, tc8_1 + sizeof(tc8_1));
    }
    dataPtr = tc8_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc8_1));
        assertCLRXException("MsgPack0", "tc8_2.Ex",
                    "MsgPack: Negative value for unsigned integer",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc8_2.DataPtr", dataPtr, tc8_1 + sizeof(tc8_1));
    }
    // 16-bit integers
    const cxbyte tc9[4] = { 0x91, 0xcd, 0x37, 0x1c };
    dataPtr = tc9;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc9));
        assertValue("MsgPack0", "tc9.value0", uint64_t(0x1c37),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc9.DataPtr", dataPtr, tc9 + sizeof(tc9));
    }
    const cxbyte tc9_1[4] = { 0x91, 0xcd, 0x7e, 0xb3 };
    dataPtr = tc9_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc9_1));
        assertValue("MsgPack0", "tc9_1.value0", uint64_t(0xb37e),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc9_1.DataPtr", dataPtr, tc9_1 + sizeof(tc9_1));
    }
    const cxbyte tc9_2[3] = { 0x91, 0xcd, 0x7e };
    dataPtr = tc9_2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc9_2));
        assertCLRXException("MsgPack0", "tc9_2.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc9_2.DataPtr", dataPtr, tc9_2 + sizeof(tc9_2)-1);
    }
    const cxbyte tc9_3[2] = { 0x91, 0xcd };
    dataPtr = tc9_3;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc9_3));
        assertCLRXException("MsgPack0", "tc9_3.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc9_3.DataPtr", dataPtr, tc9_3 + sizeof(tc9_3));
    }
    const cxbyte tc10[4] = { 0x91, 0xd1, 0x37, 0x1c };
    dataPtr = tc10;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc10));
        assertValue("MsgPack0", "tc10.value0", uint64_t(0x1c37),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc10.DataPtr", dataPtr, tc10 + sizeof(tc10));
    }
    const cxbyte tc10_1[4] = { 0x91, 0xd1, 0x7e, 0xb3 };
    dataPtr = tc10_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc10_1));
        assertValue("MsgPack0", "tc10_1.value0", uint64_t(0)+int16_t(0xb37e),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc10_1.DataPtr", dataPtr, tc10_1 + sizeof(tc10_1));
    }
    dataPtr = tc10_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc10_1));
        assertCLRXException("MsgPack0", "tc10_2.Ex",
                    "MsgPack: Negative value for unsigned integer",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc10_2.DataPtr", dataPtr, tc10_1 + sizeof(tc10_1));
    }
    const cxbyte tc10_3[3] = { 0x91, 0xcd, 0x7e };
    dataPtr = tc10_3;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc10_3));
        assertCLRXException("MsgPack0", "tc10_3.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_SIGNED); });
        assertValue("MsgPack0", "tc10_3.DataPtr", dataPtr, tc10_3+ sizeof(tc10_3)-1);
    }
    // 32-bit integers
    const cxbyte tc11[6] = { 0x91, 0xce, 0xbe, 0x96, 0x41, 0x2e };
    dataPtr = tc11;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc11));
        assertValue("MsgPack0", "tc11.value0", uint64_t(0x2e4196beU),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc11.DataPtr", dataPtr, tc11 + sizeof(tc11));
    }
    const cxbyte tc11_1[6] = { 0x91, 0xce, 0xbe, 0x96, 0x42, 0xda };
    dataPtr = tc11_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc11_1));
        assertValue("MsgPack0", "tc11_1.value0", uint64_t(0xda4296beU),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc11_1.DataPtr", dataPtr, tc11_1 + sizeof(tc11_1));
    }
    for (cxuint i = 1; i <= 3; i++)
    {
        dataPtr = tc11_1;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc11_1)-i);
        assertCLRXException("MsgPack0", "tc11_2.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc11_2.DataPtr", dataPtr, tc11_1 + 2);
    }
    const cxbyte tc12[6] = { 0x91, 0xd2, 0xbe, 0x96, 0x41, 0x2e };
    dataPtr = tc12;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc12));
        assertValue("MsgPack0", "tc12.value0", uint64_t(0x2e4196beU),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc12.DataPtr", dataPtr, tc12 + sizeof(tc12));
    }
    for (cxuint i = 1; i <= 3; i++)
    {
        dataPtr = tc12;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc12)-i);
        assertCLRXException("MsgPack0", "tc12_1.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_SIGNED); });
        assertValue("MsgPack0", "tc12_1.DataPtr", dataPtr, tc12 + 2);
    }
    const cxbyte tc12_2[6] = { 0x91, 0xd2, 0xaf, 0x11, 0x79, 0xc1 };
    dataPtr = tc12_2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc12_2));
        assertValue("MsgPack0", "tc12_2.value0", uint64_t(0)+int32_t(0xc17911afU),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc12_2.DataPtr", dataPtr, tc12_2 + sizeof(tc12_2));
    }
    dataPtr = tc12_2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc12_2));
        assertCLRXException("MsgPack0", "tc12_3.Ex",
                    "MsgPack: Negative value for unsigned integer",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc12_3.DataPtr", dataPtr, tc12_2 + sizeof(tc12_2));
    }
    // 64-bit integers
    const cxbyte tc13[10] = { 0x91, 0xcf, 0x11, 0x3a, 0xf1, 0x4b, 0x13, 0xd9, 0x1e, 0x62 };
    dataPtr = tc13;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc13));
        assertValue("MsgPack0", "tc13.value0", uint64_t(0x621ed9134bf13a11ULL),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc13.DataPtr", dataPtr, tc13 + sizeof(tc13));
    }
    const cxbyte tc13_1[10] = { 0x91, 0xcf, 0x14, 0x3a, 0xf1,
                            0x4b, 0x13, 0xd9, 0x1e, 0xd7 };
    dataPtr = tc13_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc13_1));
        assertValue("MsgPack0", "tc13_1.value0", uint64_t(0xd71ed9134bf13a14ULL),
                   arrParser.parseInteger(MSGPACK_WS_UNSIGNED));
        assertValue("MsgPack0", "tc13_1.DataPtr", dataPtr, tc13_1 + sizeof(tc13_1));
    }
    for (cxuint i = 1; i <= 7; i++)
    {
        dataPtr = tc13_1;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc13_1)-i);
        assertCLRXException("MsgPack0", "tc13_2.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_UNSIGNED); });
        assertValue("MsgPack0", "tc13_2.DataPtr", dataPtr, tc13_1 + 2);
    }
    dataPtr = tc13_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc13_1));
        assertCLRXException("MsgPack0", "tc13_3.Ex",
                    "MsgPack: Positive value out of range for signed integer",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_SIGNED); });
        assertValue("MsgPack0", "tc13_3.DataPtr", dataPtr, tc13_1 + sizeof(tc13_1));
    }
    const cxbyte tc14[10] = { 0x91, 0xd3, 0x17, 0x3a, 0xf1, 0x4b, 0x13, 0xd9, 0x1e, 0x62 };
    dataPtr = tc14;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc14));
        assertValue("MsgPack0", "tc14.value0", uint64_t(0x621ed9134bf13a17ULL),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc14.DataPtr", dataPtr, tc14 + sizeof(tc14));
    }
    const cxbyte tc14_1[10] = { 0x91, 0xd3, 0x14, 0x3a, 0xf1,
                            0x4b, 0x13, 0xd9, 0x1e, 0xd7 };
    dataPtr = tc14_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc14_1));
        assertValue("MsgPack0", "tc14_1.value0", uint64_t(0xd71ed9134bf13a14ULL),
                   arrParser.parseInteger(MSGPACK_WS_SIGNED));
        assertValue("MsgPack0", "tc14_1.DataPtr", dataPtr, tc14_1 + sizeof(tc14_1));
    }
    for (cxuint i = 1; i <= 7; i++)
    {
        dataPtr = tc14_1;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc14_1)-i);
        assertCLRXException("MsgPack0", "tc14_2.Ex", "MsgPack: Can't parse integer value",
                    [&arrParser]() { arrParser.parseInteger(MSGPACK_WS_SIGNED); });
        assertValue("MsgPack0", "tc14_2.DataPtr", dataPtr, tc14_1 + 2);
    }
    
    // parseFloat
    const cxbyte tc15[6] = { 0x91, 0xca, 0xf3, 0x9c, 0x76, 0x42 };
    dataPtr = tc15;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc15));
        assertValue("MsgPack0", "tc15.value0", double(61.653271f), arrParser.parseFloat());
        assertValue("MsgPack0", "tc15.DataPtr", dataPtr, tc15 + sizeof(tc15));
    }
    for (cxuint i = 1; i <= 3; i++)
    {
        dataPtr = tc15;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc15)-i);
        assertCLRXException("MsgPack0", "tc15_1.Ex", "MsgPack: Can't parse float value",
                    [&arrParser]() { arrParser.parseFloat(); });
        assertValue("MsgPack0", "tc15_1.DataPtr", dataPtr, tc15 + 2);
    }
    const cxbyte tc15_2[6] = { 0x91, 0xca, 0xf3, 0x9c, 0x76, 0xc2 };
    dataPtr = tc15_2;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc15_2));
        assertValue("MsgPack0", "tc15_2.value0", double(-61.653271f),
                            arrParser.parseFloat());
        assertValue("MsgPack0", "tc15_2.DataPtr", dataPtr, tc15_2 + sizeof(tc15_2));
    }
    const cxbyte tc16[10] = { 0x91, 0xcb, 0xa3, 0x0b, 0x43, 0x99, 0x3c, 0xd1, 0x8d, 0x40 };
    dataPtr = tc16;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc16));
        assertValue("MsgPack0", "tc16.value0", double(954.1545891988683663),
                    arrParser.parseFloat());
        assertValue("MsgPack0", "tc16.DataPtr", dataPtr, tc16 + sizeof(tc16));
    }
    for (cxuint i = 1; i <= 7; i++)
    {
        dataPtr = tc16;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc16)-i);
        assertCLRXException("MsgPack0", "tc16_1.Ex", "MsgPack: Can't parse float value",
                    [&arrParser]() { arrParser.parseFloat(); });
        assertValue("MsgPack0", "tc16_1.DataPtr", dataPtr, tc16 + 2);
    }
    const cxbyte tc16_1[10] = { 0x91, 0xcb, 0xa3, 0x0b, 0x43, 0x99,
                0x3c, 0xd1, 0x8d, 0xc0 };
    dataPtr = tc16_1;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc16_1));
        assertValue("MsgPack0", "tc16_1.value0", double(-954.1545891988683663),
                    arrParser.parseFloat());
        assertValue("MsgPack0", "tc16_1.DataPtr", dataPtr, tc16_1 + sizeof(tc16_1));
    }
    
    // parseData
    const cxbyte tc17[8] = { 0x91, 0xc4, 0x05, 0x91, 0xff, 0xa1, 0x5e, 0x1b };
    dataPtr = tc17;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc17));
        assertArray("MsgPack0", "tc17.value", Array<cxbyte>(tc17+3, tc17+sizeof(tc17)),
                        arrParser.parseData());
        assertValue("MsgPack0", "tc17.DataPtr", dataPtr, tc17 + sizeof(tc17));
    }
    for (cxuint i = 1; i <= 5; i ++)
    {
        dataPtr = tc17;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc17)-i);
        assertCLRXException("MsgPack0", "tc17_1.Ex", "MsgPack: Can't parse byte-array",
                    [&arrParser]() { arrParser.parseData(); });
        assertValue("MsgPack0", "tc17_1.DataPtr", dataPtr, tc17 + 3);
    }
    dataPtr = tc17;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
        assertCLRXException("MsgPack0", "tc17_2.Ex", "MsgPack: Can't parse byte-array size",
                    [&arrParser]() { arrParser.parseData(); });
        assertValue("MsgPack0", "tc17_2.DataPtr", dataPtr, tc17 + 2);
    }
    // longer data (16-bit size)
    {
        Array<cxbyte> tc18(4 + 12615);
        tc18[0] = 0x91;
        tc18[1] = 0xc5;
        tc18[2] = (12615&0xff);
        tc18[3] = (12615>>8);
        for (cxuint i = 0; i < tc18.size()-4; i++)
            tc18[i+4] = ((i*0x71f)^i) + (12342%(i+1));
        dataPtr = tc18.data();
        const cxbyte* dataEnd = tc18.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc18.size());
        assertArray("MsgPack0", "tc18.value", Array<cxbyte>(tc18.begin()+4, tc18.end()),
                    arrParser.parseData());
        assertValue("MsgPack0", "tc18.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 5; i ++)
        {
            dataPtr = tc18.data();
            dataEnd = tc18.begin()+4;
            MsgPackArrayParser arrParser(dataPtr, dataPtr + tc18.size()-i);
            assertCLRXException("MsgPack0", "tc18_1.Ex", "MsgPack: Can't parse byte-array",
                        [&arrParser]() { arrParser.parseData(); });
            assertValue("MsgPack0", "tc18_1.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc18.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
            assertCLRXException("MsgPack0", "tc18_2.Ex",
                        "MsgPack: Can't parse byte-array size",
                        [&arrParser]() { arrParser.parseData(); });
            dataEnd = tc18.begin()+2;
            assertValue("MsgPack0", "tc18_2.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc18.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 3);
            assertCLRXException("MsgPack0", "tc18_3.Ex",
                        "MsgPack: Can't parse byte-array size",
                        [&arrParser]() { arrParser.parseData(); });
            dataEnd = tc18.begin()+2;
            assertValue("MsgPack0", "tc18_3.DataPtr", dataPtr, dataEnd);
        }
    }
    // longer data (32-bit size)
    {
        Array<cxbyte> tc19(6 + 20818241);
        tc19[0] = 0x91;
        tc19[1] = 0xc6;
        tc19[2] = (20818241&0xff);
        tc19[3] = (20818241>>8)&0xff;
        tc19[4] = (20818241>>16)&0xff;
        tc19[5] = (20818241>>24);
        for (cxuint i = 0; i < tc19.size()-6; i++)
            tc19[i+6] = ((i*0x11f)^i)*3 + (1334123421%(i*5+1));
        dataPtr = tc19.data();
        const cxbyte* dataEnd = tc19.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc19.size());
        assertArray("MsgPack0", "tc19.value", Array<cxbyte>(tc19.begin()+6, tc19.end()),
                        arrParser.parseData());
        assertValue("MsgPack0", "tc19.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 5; i ++)
        {
            dataPtr = tc19.data();
            dataEnd = tc19.begin()+6;
            MsgPackArrayParser arrParser(dataPtr, dataPtr + tc19.size()-i);
            assertCLRXException("MsgPack0", "tc19_1.Ex", "MsgPack: Can't parse byte-array",
                        [&arrParser]() { arrParser.parseData(); });
            assertValue("MsgPack0", "tc19_1.DataPtr", dataPtr, dataEnd);
        }
        for (cxuint i = 1; i <= 3; i++)
        {
            dataPtr = tc19.data();
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2 + i);
            assertCLRXException("MsgPack0", "tc19_2.Ex",
                        "MsgPack: Can't parse byte-array size",
                        [&arrParser]() { arrParser.parseData(); });
            dataEnd = tc19.begin()+2;
            assertValue("MsgPack0", "tc19_2.DataPtr", dataPtr, dataEnd);
        }
    }
    // parseString
    const cxbyte tc20[7] = { 0x91, 0xa5, 0x91, 0x7b, 0xa1, 0x5e, 0x1b };
    dataPtr = tc20;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc20));
        const std::string v = arrParser.parseString();
        assertArray("MsgPack0", "tc20.value", Array<cxbyte>(tc20+2, tc20+sizeof(tc20)),
                        v.size(), reinterpret_cast<const cxbyte*>(v.c_str()));
        assertValue("MsgPack0", "tc20.DataPtr", dataPtr, tc20 + sizeof(tc20));
    }
    for (cxuint i = 1; i <= 5; i ++)
    {
        dataPtr = tc20;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc20)-i);
        assertCLRXException("MsgPack0", "tc20_1.Ex", "MsgPack: Can't parse string",
                    [&arrParser]() { arrParser.parseString(); });
        assertValue("MsgPack0", "tc20_1.DataPtr", dataPtr, tc20 + 2);
    }
    const cxbyte tc21[0x2c] = { 0x91, 0xd9, 0x29,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x99,
        0x17, 0x27, 0x73, 0x54, 0x55, 0x56, 0x33, 0xab,
        0xff, 0xaa, 0xcc, 0x32, 0x44, 0x55, 0x66, 0x53,
        0x32, 0x41, 0x52, 0x56, 0x6a, 0x7b, 0x4a, 0x49,
        0x41, 0x9a, 0x12, 0x0f, 0x05, 0x0d, 0x13, 0x14, 0x12 };
    dataPtr = tc21;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc21));
        const std::string v = arrParser.parseString();
        assertArray("MsgPack0", "tc21.value", Array<cxbyte>(tc21+3, tc21+sizeof(tc21)),
                        v.size(), reinterpret_cast<const cxbyte*>(v.c_str()));
        assertValue("MsgPack0", "tc21.DataPtr", dataPtr, tc21 + sizeof(tc21));
    }
    for (cxuint i = 1; i <= 20; i ++)
    {
        dataPtr = tc21;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc21)-i);
        assertCLRXException("MsgPack0", "tc21_1.Ex", "MsgPack: Can't parse string",
                    [&arrParser]() { arrParser.parseString(); });
        assertValue("MsgPack0", "tc21_1.DataPtr", dataPtr, tc21 + 3);
    }
    dataPtr = tc21;
    {
        MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
        assertCLRXException("MsgPack0", "tc21_2.Ex", "MsgPack: Can't parse string size",
                    [&arrParser]() { arrParser.parseString(); });
        assertValue("MsgPack0", "tc21_2.DataPtr", dataPtr, tc21 + 2);
    }
    // longer data (16-bit size)
    {
        Array<cxbyte> tc22(4 + 12615);
        tc22[0] = 0x91;
        tc22[1] = 0xda;
        tc22[2] = (12615&0xff);
        tc22[3] = (12615>>8);
        for (cxuint i = 0; i < tc22.size()-4; i++)
            tc22[i+4] = ((i*0x71f)^i) + (12342%(i+1));
        dataPtr = tc22.data();
        const cxbyte* dataEnd = tc22.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc22.size());
        const std::string v = arrParser.parseString();
        assertArray("MsgPack0", "tc22.value", Array<cxbyte>(tc22.begin()+4, tc22.end()),
                    v.size(), reinterpret_cast<const cxbyte*>(v.c_str()));
        assertValue("MsgPack0", "tc22.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 5; i ++)
        {
            dataPtr = tc22.data();
            dataEnd = tc22.begin()+4;
            MsgPackArrayParser arrParser(dataPtr, dataPtr + tc22.size()-i);
            assertCLRXException("MsgPack0", "tc22_1.Ex", "MsgPack: Can't parse string",
                        [&arrParser]() { arrParser.parseString(); });
            assertValue("MsgPack0", "tc22_1.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc22.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
            assertCLRXException("MsgPack0", "tc18_2.Ex",
                        "MsgPack: Can't parse string size",
                        [&arrParser]() { arrParser.parseString(); });
            dataEnd = tc22.begin()+2;
            assertValue("MsgPack0", "tc22_2.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc22.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 3);
            assertCLRXException("MsgPack0", "tc18_3.Ex",
                        "MsgPack: Can't parse string size",
                        [&arrParser]() { arrParser.parseString(); });
            dataEnd = tc22.begin()+2;
            assertValue("MsgPack0", "tc22_3.DataPtr", dataPtr, dataEnd);
        }
    }
    // longer data (32-bit size)
    {
        Array<cxbyte> tc23(6 + 20818241);
        tc23[0] = 0x91;
        tc23[1] = 0xdb;
        tc23[2] = (20818241&0xff);
        tc23[3] = (20818241>>8)&0xff;
        tc23[4] = (20818241>>16)&0xff;
        tc23[5] = (20818241>>24);
        for (cxuint i = 0; i < tc23.size()-6; i++)
            tc23[i+6] = ((i*0x11f)^i)*3 + (1334123421%(i*5+1));
        dataPtr = tc23.data();
        const cxbyte* dataEnd = tc23.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc23.size());
        const std::string v = arrParser.parseString();
        assertArray("MsgPack0", "tc23.value", Array<cxbyte>(tc23.begin()+6, tc23.end()),
                        v.size(), reinterpret_cast<const cxbyte*>(v.c_str()));
        assertValue("MsgPack0", "tc23.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 5; i ++)
        {
            dataPtr = tc23.data();
            dataEnd = tc23.begin()+6;
            MsgPackArrayParser arrParser(dataPtr, dataPtr + tc23.size()-i);
            assertCLRXException("MsgPack0", "tc23_1.Ex", "MsgPack: Can't parse string",
                        [&arrParser]() { arrParser.parseString(); });
            assertValue("MsgPack0", "tc23_1.DataPtr", dataPtr, dataEnd);
        }
        for (cxuint i = 1; i <= 3; i++)
        {
            dataPtr = tc23.data();
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2 + i);
            assertCLRXException("MsgPack0", "tc23_2.Ex",
                        "MsgPack: Can't parse string size",
                        [&arrParser]() { arrParser.parseString(); });
            dataEnd = tc23.begin()+2;
            assertValue("MsgPack0", "tc23_2.DataPtr", dataPtr, dataEnd);
        }
    }
    
    // parseArray
    const cxbyte tc24[10] = { 0x91, 0x96, 0x11, 0x33, 0xcc, 0xb4, 0x74, 0xcc, 0x99, 0x21 };
    {
        dataPtr = tc24;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc24));
        MsgPackArrayParser childParser = arrParser.parseArray();
        std::vector<cxuint> res;
        while (childParser.haveElements())
            res.push_back(childParser.parseInteger(MSGPACK_WS_BOTH));
        const cxuint expected[6] = { 0x11, 0x33, 0xb4, 0x74, 0x99, 0x21 };
        assertArray("MsgPack0", "tc24.value", Array<cxuint>(expected, expected + 6),
                        Array<cxuint>(res.begin(), res.end()));
        assertValue("MsgPack0", "tc24.DataPtr", dataPtr, tc24 + sizeof(tc24));
        assertTrue("MsgPack0", "No elements", !childParser.haveElements());
    }
    // longer array (more than 15 elements)
    const cxbyte tc25[39] = { 0x91, 0xdc, 0x23, 0x00,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x78,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x21, 0x12, 0x23, 0x54, 0x25, 0x46, 0x77, 0x58,
        0x0f, 0x0a, 0x0b, 0x0d, 0x0e, 0x01, 0x21, 0x22,
        0x79, 0x7b, 0x7f };
    {
        dataPtr = tc25;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc25));
        MsgPackArrayParser childParser = arrParser.parseArray();
        std::vector<cxuint> res;
        while (childParser.haveElements())
            res.push_back(childParser.parseInteger(MSGPACK_WS_BOTH));
        const cxuint expected[35] = {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x78,
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x21, 0x12, 0x23, 0x54, 0x25, 0x46, 0x77, 0x58,
            0x0f, 0x0a, 0x0b, 0x0d, 0x0e, 0x01, 0x21, 0x22,
            0x79, 0x7b, 0x7f };
        assertArray("MsgPack0", "tc25.value", Array<cxuint>(expected, expected + 35),
                        Array<cxuint>(res.begin(), res.end()));
        assertValue("MsgPack0", "tc25.DataPtr", dataPtr, tc25 + sizeof(tc25));
        assertTrue("MsgPack0", "tc25.No elements", !childParser.haveElements());
    }
    // longer array (16-bit size)
    {
        Array<cxbyte> tc26(4 + 12615);
        tc26[0] = 0x91;
        tc26[1] = 0xdc;
        tc26[2] = (12615&0xff);
        tc26[3] = (12615>>8);
        for (cxuint i = 0; i < tc26.size()-4; i++)
            tc26[i+4] = (((i*0x71f)^i) + (12342%(i+1)))&0x7f;
        dataPtr = tc26.data();
        const cxbyte* dataEnd = tc26.end();
        std::vector<cxbyte> res;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc26.size());
        MsgPackArrayParser childParser = arrParser.parseArray();
        while (childParser.haveElements())
            res.push_back(childParser.parseInteger(MSGPACK_WS_BOTH));
        assertArray("MsgPack0", "tc26.value",
                    Array<cxbyte>(tc26.begin()+4, tc26.end()), res);
        assertValue("MsgPack0", "tc26.DataPtr", dataPtr, dataEnd);
        dataPtr = tc26.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
            assertCLRXException("MsgPack0", "tc26_2.Ex", "MsgPack: Can't parse array size",
                        [&arrParser]() { arrParser.parseArray(); });
            dataEnd = tc26.begin()+2;
            assertValue("MsgPack0", "tc26_2.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc26.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 3);
            assertCLRXException("MsgPack0", "tc26_3.Ex", "MsgPack: Can't parse array size",
                        [&arrParser]() { arrParser.parseArray(); });
            dataEnd = tc26.begin()+2;
            assertValue("MsgPack0", "tc26_3.DataPtr", dataPtr, dataEnd);
        }
    }
    // longer array (32-bit size)
    {
        Array<cxbyte> tc27(6 + 1818241);
        tc27[0] = 0x91;
        tc27[1] = 0xdd;
        tc27[2] = (1818241&0xff);
        tc27[3] = (1818241>>8)&0xff;
        tc27[4] = (1818241>>16)&0xff;
        tc27[5] = (1818241>>24);
        for (cxuint i = 0; i < tc27.size()-6; i++)
            tc27[i+6] = (((i*0x11f)^i)*3 + (1334123421%(i*5+1)))&0x7f;
        dataPtr = tc27.data();
        const cxbyte* dataEnd = tc27.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc27.size());
        std::vector<cxbyte> res;
        MsgPackArrayParser childParser = arrParser.parseArray();
        while (childParser.haveElements())
            res.push_back(childParser.parseInteger(MSGPACK_WS_BOTH));
        assertArray("MsgPack0", "tc27.value",
                    Array<cxbyte>(tc27.begin()+6, tc27.end()), res);
        assertValue("MsgPack0", "tc27.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 3; i++)
        {
            dataPtr = tc27.data();
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2 + i);
            assertCLRXException("MsgPack0", "tc27_2.Ex", "MsgPack: Can't parse array size",
                        [&arrParser]() { arrParser.parseArray(); });
            dataEnd = tc27.begin()+2;
            assertValue("MsgPack0", "tc27_2.DataPtr", dataPtr, dataEnd);
        }
    }
    
    // parseMap
    const cxbyte tc28[16] = { 0x91, 0x85, 0x11, 0x33, 0xcc, 0xb4, 0x74, 0xcc, 0x99,
                        0x21, 0xcc, 0xff, 0x42, 0x71, 0xcc, 0xca };
    {
        dataPtr = tc28;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc28));
        MsgPackMapParser childParser = arrParser.parseMap();
        std::vector<cxuint> res;
        while (childParser.haveElements())
        {
            res.push_back(childParser.parseKeyInteger(MSGPACK_WS_BOTH));
            res.push_back(childParser.parseValueInteger(MSGPACK_WS_BOTH));
        }
        const cxuint expected[10] = { 0x11, 0x33, 0xb4, 0x74, 0x99,
                                    0x21, 0xff, 0x42, 0x71, 0xca };
        assertArray("MsgPack0", "tc28.value", Array<cxuint>(expected, expected + 10),
                        Array<cxuint>(res.begin(), res.end()));
        assertValue("MsgPack0", "tc28.DataPtr", dataPtr, tc28 + sizeof(tc28));
        assertTrue("MsgPack0", "tc28.No elements", !childParser.haveElements());
    }
    {
        dataPtr = tc28;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc28));
        MsgPackMapParser childParser = arrParser.parseMap();
        for (cxuint i = 0; i < 4; i++)
        {
            childParser.parseKeyInteger(MSGPACK_WS_BOTH);
            childParser.parseValueInteger(MSGPACK_WS_BOTH);
        }
        childParser.parseKeyInteger(MSGPACK_WS_BOTH);
        assertTrue("MsgPack0", "tc28_1.HaveElements", childParser.haveElements());
    }
    {
        dataPtr = tc28;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc28));
        MsgPackMapParser childParser = arrParser.parseMap();
        assertCLRXException("MsgPack0", "tc28_2.IsNotValue",
                    "MsgPack: This is not a value",
                    [&childParser]() { childParser.parseValueBool(); });
    }
    {
        dataPtr = tc28;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + sizeof(tc28));
        MsgPackMapParser childParser = arrParser.parseMap();
        childParser.parseKeyInteger(MSGPACK_WS_BOTH);
        assertCLRXException("MsgPack0", "tc28_2.IsNotKey",
                    "MsgPack: Key already parsed",
                    [&childParser]() { childParser.parseKeyBool(); });
    }
    // longer map (16-bit size)
    {
        Array<cxbyte> tc29(4 + 12615*2);
        tc29[0] = 0x91;
        tc29[1] = 0xde;
        tc29[2] = (12615&0xff);
        tc29[3] = (12615>>8);
        for (cxuint i = 0; i < tc29.size()-4; i++)
            tc29[i+4] = (((i*0x71f)^i) + (12342%(i+1)))&0x7f;
        dataPtr = tc29.data();
        const cxbyte* dataEnd = tc29.end();
        std::vector<cxbyte> res;
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc29.size());
        MsgPackMapParser childParser = arrParser.parseMap();
        while (childParser.haveElements())
        {
            res.push_back(childParser.parseKeyInteger(MSGPACK_WS_BOTH));
            res.push_back(childParser.parseValueInteger(MSGPACK_WS_BOTH));
        }
        assertArray("MsgPack0", "tc29.value",
                    Array<cxbyte>(tc29.begin()+4, tc29.end()), res);
        assertValue("MsgPack0", "tc29.DataPtr", dataPtr, dataEnd);
        dataPtr = tc29.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2);
            assertCLRXException("MsgPack0", "tc29_2.Ex", "MsgPack: Can't parse map size",
                        [&arrParser]() { arrParser.parseMap(); });
            dataEnd = tc29.begin()+2;
            assertValue("MsgPack0", "tc29_2.DataPtr", dataPtr, dataEnd);
        }
        dataPtr = tc29.data();
        {
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 3);
            assertCLRXException("MsgPack0", "tc29_3.Ex", "MsgPack: Can't parse map size",
                        [&arrParser]() { arrParser.parseMap(); });
            dataEnd = tc29.begin()+2;
            assertValue("MsgPack0", "tc29_3.DataPtr", dataPtr, dataEnd);
        }
    }
    // longer map (32-bit size)
    {
        Array<cxbyte> tc30(6 + 1818241*2);
        tc30[0] = 0x91;
        tc30[1] = 0xdf;
        tc30[2] = (1818241&0xff);
        tc30[3] = (1818241>>8)&0xff;
        tc30[4] = (1818241>>16)&0xff;
        tc30[5] = (1818241>>24);
        for (cxuint i = 0; i < tc30.size()-6; i++)
            tc30[i+6] = (((i*0x11f)^i)*3 + (1334123421%(i*5+1)))&0x7f;
        dataPtr = tc30.data();
        const cxbyte* dataEnd = tc30.end();
        MsgPackArrayParser arrParser(dataPtr, dataPtr + tc30.size());
        std::vector<cxbyte> res;
        MsgPackMapParser childParser = arrParser.parseMap();
        while (childParser.haveElements())
        {
            res.push_back(childParser.parseKeyInteger(MSGPACK_WS_BOTH));
            res.push_back(childParser.parseValueInteger(MSGPACK_WS_BOTH));
        }
        assertArray("MsgPack0", "tc30.value",
                    Array<cxbyte>(tc30.begin()+6, tc30.end()), res);
        assertValue("MsgPack0", "tc30.DataPtr", dataPtr, dataEnd);
        for (cxuint i = 1; i <= 3; i++)
        {
            dataPtr = tc30.data();
            MsgPackArrayParser arrParser(dataPtr, dataPtr + 2 + i);
            assertCLRXException("MsgPack0", "tc30_2.Ex", "MsgPack: Can't parse map size",
                        [&arrParser]() { arrParser.parseMap(); });
            dataEnd = tc30.begin()+2;
            assertValue("MsgPack0", "tc30_2.DataPtr", dataPtr, dataEnd);
        }
    }
}

static void testMsgPackSkip()
{
    const cxbyte tc0[3] = { 0x81, 0xc0, 0xc0 };
    const cxbyte* dataPtr;
    dataPtr = tc0;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc0));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc0.DataPtr", dataPtr, tc0 + sizeof(tc0));
    }
    
    // skip boolean
    const cxbyte tc1[3] = { 0x81, 0xc0, 0xc2 };
    dataPtr = tc1;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc1));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc1.DataPtr", dataPtr, tc1 + sizeof(tc1));
    }
    const cxbyte tc1_1[3] = { 0x81, 0xc0, 0xc3 };
    dataPtr = tc1_1;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc1_1));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc1_1.DataPtr", dataPtr, tc1_1 + sizeof(tc1_1));
    }
    // skip integers
    const cxbyte tc2[3] = { 0x81, 0xc0, 0x17 };
    dataPtr = tc2;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc2));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc2.DataPtr", dataPtr, tc2 + sizeof(tc2));
    }
    const cxbyte tc2_1[3] = { 0x81, 0xc0, 0xec };
    dataPtr = tc2_1;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc2_1));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc2_1.DataPtr", dataPtr, tc2_1 + sizeof(tc2_1));
    }
    const cxbyte tc3[4] = { 0x81, 0xc0, 0xcc, 0xda };
    dataPtr = tc3;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc3));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc3.DataPtr", dataPtr, tc3 + sizeof(tc3));
    }
    dataPtr = tc3;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc3)-1);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc3_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    const cxbyte tc4[5] = { 0x81, 0xc0, 0xcd, 0xda, 0x99 };
    dataPtr = tc4;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc4));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc4.DataPtr", dataPtr, tc4 + sizeof(tc4));
    }
    dataPtr = tc4;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc4)-1);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc4_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    dataPtr = tc4;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc4)-2);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc4_2.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    const cxbyte tc5[7] = { 0x81, 0xc0, 0xce, 0x11, 0xbb, 0xd1, 0xe6 };
    dataPtr = tc5;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc5));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc5.DataPtr", dataPtr, tc5 + sizeof(tc5));
    }
    for (cxuint i = 1; i <= 3; i++)
    {
        dataPtr = tc5;
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc5)-i);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc5_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    const cxbyte tc6[11] = { 0x81, 0xc0, 0xcf, 0x11, 0x77, 0xab, 0x3c,
                    0x1a, 0x88, 0x83, 0xde };
    dataPtr = tc6;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc6));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc6.DataPtr", dataPtr, tc6 + sizeof(tc6));
    }
    for (cxuint i = 1; i <= 7; i++)
    {
        dataPtr = tc6;
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc6)-i);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc6_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    // skip floats
    const cxbyte tc7[7] = { 0x81, 0xc0, 0xca, 0x11, 0xbb, 0xd1, 0xe6 };
    dataPtr = tc7;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc7));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc7.DataPtr", dataPtr, tc7 + sizeof(tc7));
    }
    for (cxuint i = 1; i <= 3; i++)
    {
        dataPtr = tc7;
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc7)-i);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc7_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    const cxbyte tc8[11] = { 0x81, 0xc0, 0xcf, 0x11, 0x77, 0xab, 0x3c,
                    0x1a, 0x88, 0x83, 0xde };
    dataPtr = tc8;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc8));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc8.DataPtr", dataPtr, tc8 + sizeof(tc8));
    }
    for (cxuint i = 1; i <= 7; i++)
    {
        dataPtr = tc8;
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc8)-i);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc8_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    /* skip string */
    const cxbyte tc9[8] = { 0x81, 0xc0, 0xa5, 0x41, 0x42, 0x53, 0x43, 0x47 };
    dataPtr = tc9;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc9));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc9.DataPtr", dataPtr, tc9 + sizeof(tc9));
    }
    dataPtr = tc9;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc9)-1);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc9_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    const cxbyte tc10[38] = { 0x81, 0xc0, 0xd9, 0x22,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x54, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x04, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x04, 0xf5, 0x06, 0x07, 0x08,
        0x33, 0x66 };
    dataPtr = tc10;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc10));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc9.DataPtr", dataPtr, tc10 + sizeof(tc10));
    }
    dataPtr = tc10;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc10)-1);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc10_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    dataPtr = tc10;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + 3);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc10_2.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    {
        Array<cxbyte> tc11(5 + 11232);
        tc11[0] = 0x81;
        tc11[1] = 0xc0;
        tc11[2] = 0xda;
        tc11[3] = (11232)&255;
        tc11[4] = (11232)>>8;
        for (size_t i = 0; i < tc11.size()-5; i++)
            tc11[i+5] = (i&0x80)&0xff;
        dataPtr = tc11.data();
        MsgPackMapParser mapParser(dataPtr, dataPtr + tc11.size());
        mapParser.parseKeyNil();
        mapParser.skipValue();
        const cxbyte* dataEnd = tc11.end();
        assertValue("MsgPackSkip", "tc11.DataPtr", dataPtr, dataEnd);
        {
            dataPtr = tc11.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + tc11.size()-1);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc11_1.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
        {
            dataPtr = tc11.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + 4);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc11_2.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
    }
    {
        Array<cxbyte> tc12(7 + 4511232);
        tc12[0] = 0x81;
        tc12[1] = 0xc0;
        tc12[2] = 0xdb;
        tc12[3] = (4511232)&255;
        tc12[4] = ((4511232)>>8)&255;
        tc12[5] = ((4511232)>>16)&255;
        tc12[6] = ((4511232)>>24);
        for (size_t i = 0; i < tc12.size()-7; i++)
            tc12[i+7] = (i&0x80)&0xff;
        dataPtr = tc12.data();
        MsgPackMapParser mapParser(dataPtr, dataPtr + tc12.size());
        mapParser.parseKeyNil();
        mapParser.skipValue();
        const cxbyte* dataEnd = tc12.end();
        assertValue("MsgPackSkip", "tc12.DataPtr", dataPtr, dataEnd);
        {
            dataPtr = tc12.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + tc12.size()-1);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc12_1.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
        {
            dataPtr = tc12.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + 6);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc12_2.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
    }
    /* skip bin data */
    const cxbyte tc13[38] = { 0x81, 0xc0, 0xc4, 0x22,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x54, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x04, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x01, 0x02, 0x03, 0x04, 0xf5, 0x06, 0x07, 0x08,
        0x33, 0x66 };
    dataPtr = tc13;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc13));
        mapParser.parseKeyNil();
        mapParser.skipValue();
        assertValue("MsgPackSkip", "tc9.DataPtr", dataPtr, tc13 + sizeof(tc13));
    }
    dataPtr = tc13;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + sizeof(tc13)-1);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc13_1.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    dataPtr = tc13;
    {
        MsgPackMapParser mapParser(dataPtr, dataPtr + 3);
        mapParser.parseKeyNil();
        assertCLRXException("MsgPackSkip", "tc13_2.DataPtr", "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
    }
    {
        Array<cxbyte> tc14(5 + 11232);
        tc14[0] = 0x81;
        tc14[1] = 0xc0;
        tc14[2] = 0xc5;
        tc14[3] = (11232)&255;
        tc14[4] = (11232)>>8;
        for (size_t i = 0; i < tc14.size()-5; i++)
            tc14[i+5] = (i&0x80)&0xff;
        dataPtr = tc14.data();
        MsgPackMapParser mapParser(dataPtr, dataPtr + tc14.size());
        mapParser.parseKeyNil();
        mapParser.skipValue();
        const cxbyte* dataEnd = tc14.end();
        assertValue("MsgPackSkip", "tc14.DataPtr", dataPtr, dataEnd);
        {
            dataPtr = tc14.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + tc14.size()-1);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc14_1.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
        {
            dataPtr = tc14.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + 4);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc14_2.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
    }
    {
        Array<cxbyte> tc15(7 + 4511232);
        tc15[0] = 0x81;
        tc15[1] = 0xc0;
        tc15[2] = 0xc6;
        tc15[3] = (4511232)&255;
        tc15[4] = ((4511232)>>8)&255;
        tc15[5] = ((4511232)>>16)&255;
        tc15[6] = ((4511232)>>24);
        for (size_t i = 0; i < tc15.size()-7; i++)
            tc15[i+7] = (i&0x80)&0xff;
        dataPtr = tc15.data();
        MsgPackMapParser mapParser(dataPtr, dataPtr + tc15.size());
        mapParser.parseKeyNil();
        mapParser.skipValue();
        const cxbyte* dataEnd = tc15.end();
        assertValue("MsgPackSkip", "tc15.DataPtr", dataPtr, dataEnd);
        {
            dataPtr = tc15.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + tc15.size()-1);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc15_1.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
        {
            dataPtr = tc15.data();
            MsgPackMapParser mapParser(dataPtr, dataPtr + 6);
            mapParser.parseKeyNil();
            assertCLRXException("MsgPackSkip", "tc15_2.DataPtr",
                    "MsgPack: Can't skip object",
                    [&mapParser]() { mapParser.skipValue(); });
        }
    }
    /* skip arrays */
}

int main(int argc, const char** argv)
{
    int retVal = 0;
    retVal |= callTest(testMsgPackBytes);
    retVal |= callTest(testMsgPackSkip);
    return retVal;
}