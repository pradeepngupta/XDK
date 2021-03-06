/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

#ifndef __TBA_XDK_HOOKER_IA32_SEGMENTS_H
#define __TBA_XDK_HOOKER_IA32_SEGMENTS_H

/**
 * segments.h
 *
 * A parser for memory segment selectors.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xdk/hooker/Processors/ia32/descriptorTable.h"

// Change the packing method for single byte. (No alignment)
#pragma pack(push)
#pragma pack(1)

/**
 * Parser for the selector.
 * The selector has 16 bits:
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         |T|RPL|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
class Selector {
public:
    /**
     * Construct the selector indicator.
     *
     * selector - The requested selector
     */
    Selector(uint16 selector);

    /**
     * Return the requested privilege level
     */
    uint getRpl() const;

    /**
     * Return true if the selector is inside the GDT or false for the LDT
     */
    bool isGDTselector() const;

    /**
     * Return the index inside the table
     */
    uint getIndex() const;

    // Copy constructor and operator = will be generated by the compiler

    #ifdef COMMON_DUMP
    /**
     * Return a string which describes the selector
     */
    cString getSelectorName() const;
    #endif // COMMON_DUMP

private:
    /// The paresed selector
    union {
        // The selector raw data
        uint16 m_selector;
        // The bits types
        struct {
            /// The request priviledge level of the selector
            unsigned m_requestPrivilageLevel : 2;
            /// The table indicator 0 - GDT, 1 - LDT
            unsigned m_tableIndicator : 1;
            /// The index inside the table
            unsigned m_index : 13;
        } m_bits;
    } m_selector;
};

/*
 * The rings in the system. Used for RPL (Request Priviledge)
 *                                   CPL (Code Priviledge) and
 *                                   DPL (Data Priviledge)
 */
enum Rings {
    // Ring0, kernel mode
    RING0 = 0,
    KERNEL_MODE = 0,
    // Ring3, user mode
    RING3 = 3,
    USER_MODE = 3
};

/**
 * Gets the GDT table using the GDTR
 */
class GDTR : public DescriptorTableRegister
{
public:
    /**
     * Default constructor. Invoke SGDT in order to query the GDTR
     * register
     */
    GDTR();
};


/**
 * Each entry inside the GDT contains 8 bytes:
 *                                           12
 *   31               24      19     16   14   11     8
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    | Base 24:31    |G|D|0|A|SL16:19|P |DPL|S| Type |   Base 16:23  |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |    Base address 0:15          |      Segment limit 0:15       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
class SegmentDescriptor {
public:
    /**
     * Parse the vector for a spesific IDT.
     *
     * dtr      - The descriptor for the table
     * selector - The index inside the table
     */
    SegmentDescriptor(const DescriptorTableRegister& dtr,
                      const Selector& selector);

    // Copy constructor and operator = will be generated by the compiler

    /**
     * Return true if the descriptor is present
     */
    bool isPresent() const;

    /**
     * Calculate the offset for the descriptor.
     */
    uint32 getOffset() const;

    /**
     * Returns the segment limit.
     *
     * The segment limit are 20 bits longs which represent the number of bytes
     * or the number of pages the segment uses.
     * The speration between bytes and pages is done by looking at the
     * granularity flags.
     *
     * Return segment limit in bytes;
     */
    uint32 getSegmentLimit() const;

    /**
     * Return the selector for the IDT
     */
    const Selector& getSelector() const;

    /**
     * Return true if the selector is a system selector
     */
    bool isSystem() const;

    /**
     * Return the type of the segment for system descriptor
     * See SystemSegmentTypes
     */
    uint getSystemSegmentType() const;

    /**
     * Return true if the segment is code segment or false if the segment is
     *        data segment
     * The selector musn't be a system selector, otherwise an exception will
     * be throw
     */
    bool isCode() const;

    /**
     * Return true if the segment is a data segment and it's expand downward.
     */
    bool isDataExpandDown() const;

    /**
     * Return true if the segment is a data segment and it's read only.
     *         false means that the segment is read and write
     */
    bool isDataReadOnly() const;

    // TODO! Accessed bit (When the segment was last access to since the
    // OS clear the bit). - So far not in any used.

    /**
     * Return true if the segment is a code segment and it's conforming,
     *        which means that the segment can be called from higher
     *        privilege level and return to it.
     */
    bool isCodeConforming() const;

    /**
     * Return true if the segment is a code segment and it's execute only
     *         false means that the segment is a code segment and it's also
     *         read-only code.
     */
    bool isCodeExecuteOnly() const;

    /**
     * The different system segments types
     * See getSystemSegmentType()
     */
    enum SystemSegmentTypes {
        // Reserved for future use
        SEGMENT_RESERVED    = 0,
        // 16 bit Task Segment (avaliable)
        SEGMENT_TSS16_FREE  = 1,
        // The local descriptor table segment
        SEGMENT_LDT         = 2,
        // 16 bit Task Segment (busy)
        SEGMENT_TSS16_BUSY  = 3,
        // 16 bit call gate
        SEGMENT_CALLGATE_16 = 4,
        // Task gate (selector)
        SEGMENT_TASK_GATE   = 5,
        // 16 bit interrupt gate
        SEGMENT_INTGATE_16  = 6,
        // 16 bit trap gate
        SEGMENT_TRAPGATE_16 = 7,
        // Reserved for future use
        SEGMENT_RESERVED1   = 8,
        // 32 bit Task Segment (avaliable)
        SEGMENT_TSS32_FREE  = 9,
        // Reserved for future use
        SEGMENT_RESERVED2   = 10,
        // 32 bit Task Segment (busy)
        SEGMENT_TSS32_BUSY  = 11,
        // 32 bit call gate
        SEGMENT_CALLGATE_32 = 12,
        // Reserved for future use
        SEGMENT_RESERVED3   = 13,
        // 32 bit interrupt gate
        SEGMENT_INTGATE_32  = 14,
        // 32 bit trap gate
        SEGMENT_TRAPGATE_32 = 15
    };

    /**
     * Trace the content of the descriptor in a TRACE statments
     */
    #ifdef COMMON_DUMP
    void traceOut() const;
    #endif // COMMON_DUMP

private:
    /// The selector number
    Selector m_selector;

    /// The flags of the descriptors
    union {
        // The content of the union
        DescriptorValue m_descriptor;
        // The common flags representation
        struct {
            // The lower 16 bits of the segments limit
            uint16 m_segmentLimit0_15;
            // The lower 16 bits of the segments base
            uint16 m_base0_15;
            // The base address 16..23 bits
            uint8 m_base16_23;
            // The flags of the selector
            uint8 m_flags;
            // The flags of the selector
            uint8 m_flags1;
            // The base address 24..32 bits
            uint8 m_base24_31;
        } m_bits;
    } m_descriptor;

    // The type of the first 8 bits flags
    typedef union {
        // Raw-data
        uint8 m_flags;
        struct {
            /// The type of descriptor.
            unsigned m_type : 4;
            /// The type of the descriptor: 0 - system, 1 - code or data
            unsigned m_descriptorType : 1;
            /// The privieliage level
            unsigned m_dpl : 2;
            /// The present flag
            unsigned m_present : 1;
        } m_bits;
    } SegmentFlags;

    // The content of the 4 bits for the SegmentFlags.m_type
    typedef union {
        uint8 m_type;
        struct {
            // Set to 1 by the processor when the segment is used
            unsigned m_accessed : 1;
            // See SegementTypeFlagsCode, SegementTypeFlagsData
            unsigned m_attributes : 2;
            // Is this segement is a code segment. (0 - Data, 1 - Code)
            unsigned m_code : 1;
        } m_bits;
    } SegementTypeFlags;

    // Code segment bits
    typedef union {
        uint8 m_typeFlags;
        struct {
            // 0 For execute only segment 1 for exec/read only seg
            unsigned m_executeOnly : 1;
            // Is this segment is conforming
            unsigned m_conforming : 1;
        } m_bits;
    } SegementTypeFlagsCode;

    // Data segment bits
    typedef union {
        uint8 m_typeFlags;
        struct {
            // 0 for read only segment 1 for read/write segment
            unsigned m_readOnly : 1;
            // 0 for expand up segment and 1 for expand down segment
            unsigned m_expandDown : 1;
        } m_bits;
    } SegementTypeFlagsData;

    // The type of the second 8 bits flags
    typedef union {
        /// The raw-content of the flags
        uint8 m_flags;
        struct {
            /// The type of descriptor.
            unsigned m_segmentLimit16_19 : 4;
            /// Avaliable for operating system use
            unsigned m_avliable : 1;
            /// Must be 0
            unsigned m_zero : 1;
            /// The default operation size (0 - 16bit, 1 - 32bit)
            unsigned m_operationSize : 1;
            /// The granularity (The limit is bytes or pages)
            unsigned m_granularity : 1;
        } m_bits;
    } SegmentFlags1;

    /// Debug mode contains strings which describes the different system segments
    #ifdef COMMON_DUMP
    typedef char* lpString;
    static const lpString m_systemSegmentsStrings[];
    #endif // COMMON_DUMP
};

// Restore packing method
#pragma pack(pop)

#endif // __TBA_XDK_HOOKER_IA32_SEGMENTS_H
