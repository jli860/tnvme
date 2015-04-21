/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _CMD_H_
#define _CMD_H_

#include "tnvme.h"
#include "trackable.h"
#include "prpData.h"
#include "metaData.h"
#include "../Exception/frmwkEx.h"

class SQ;     // forward definition

class Cmd;    // forward definition
typedef boost::shared_ptr<Cmd>              SharedCmdPtr;
#define CAST_TO_Cmd(shared_trackable_ptr)   \
        boost::dynamic_pointer_cast<Cmd>(shared_trackable_ptr);


/**
* This class is the base class to all other cmd classes. It is not meant to
* be instantiated. This class contains all things common to cmds at a high
* level. After instantiation by a child the Init() method must be called be
* to attain something useful.
*
* @note This class may throw exceptions.
*/
class Cmd : public Trackable, public PrpData, public MetaData
{
public:
    /**
     * @param objBeingCreated Pass the type of object this child class is
     */
    Cmd(Trackable::ObjType objBeingCreated);
    virtual ~Cmd();

    /// Dump the entire contents of the cmd buffer to the logging endpoint
    void LogCmd() const;

    /// Access to the actual cmd bytes
    SharedMemBufferPtr GetCmd() const { return mCmdBuf; }

    uint16_t  GetCmdSizeB() const { return mCmdBuf->GetBufSize(); }
    uint16_t  GetCmdSizeW() const { return (mCmdBuf->GetBufSize() / 2); }
    uint8_t   GetCmdSizeDW() const { return (mCmdBuf->GetBufSize() / 4); }
    uint8_t   GetOpcode() const { return GetByte(0, 0); }
    string    GetName() const { return mCmdName; }

    /// Returns the direction of data transfer for the PRP buffer
    virtual DataDir GetDataDir() const { return mDataDir; }

    static const uint8_t BITMASK_FUSE_B;
    static const uint32_t BITMASK_FUSE_DW;
    void     SetFUSE(uint8_t newVal);
    uint8_t  GetFUSE() const;

    /**
     * Namespace ID (NSID). A value of 0 is used when this field is not
     * utilized for the present cmd, and a value of 0xffffffff refers to all
     * namespaces within the DUT.
     * @param newVal Pass the new value to set for the NSID
     */
    void     SetNSID(uint32_t newVal);
    uint32_t GetNSID() const;

    /**
     * This value cannot be set because dnvme overwrites any value we would
     * set. dnvme does this to guarantee the uniqueness of the ID. However,
     * dnvme copies the CID back to user space when a cmd is sent for
     * submission to a SQ. So the exact value is only valid after a cmd is
     * submitted to an NVME device.
     */
    static const uint32_t BITMASK_CID_DW;
    uint16_t GetCID() const;

    /**
     * @param newVal Pass the new DWORD value to set in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     */
    void SetDword(uint32_t newVal, uint8_t whichDW);
    uint32_t GetDword(uint8_t whichDW) const;

    /**
     * @param newVal Pass the new WORD value to set in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     * @param dwOffset Pass [0->1] for which word offset within the DW to set
     */
    void SetWord(uint16_t newVal, uint8_t whichDW, uint8_t dwOffset);
    uint16_t GetWord(uint8_t whichDW, uint8_t dwOffset) const;

    /**
     * @param newVal Pass the new DWORD value to set in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     * @param dwOffset Pass [0->3] for which byte offset within the DW to set
     */
    void SetByte(uint8_t newVal, uint8_t whichDW, uint8_t dwOffset);
    uint8_t GetByte(uint8_t whichDW, uint8_t dwOffset) const;

    /**
     * @param newVal Pass true to set, false to reset in the cmd buffer
     * @param whichDW Pass [0->(GetCmdSizeDW()-1)] which DWORD to set
     * @param dwOffset Pass [0->31] for which bit offset within the DW to set
     */
    void SetBit(bool newVal, uint8_t whichDW, uint8_t dwOffset);
    bool GetBit(uint8_t whichDW, uint8_t dwOffset) const;

    /**
     * Append the entire contents of this cmd's command bytes to the named file.
     * @param filename Pass the filename as generated by macro
     *      FileSystem::PrepDumpFile().
     * @param fileHdr Pass a custom file header description to dump
     */
    virtual void Dump(DumpFilename filename, string fileHdr) const;
    void Print();

protected:
    /**
     * Initialize this object.
     * @param opcode Pass the opcode defining this cmd, per NVME spec.
     * @param dataDir Pass the direction of data for this cmd. This is used
     *      to notify dnvme which way to send base classes PrpData. The kernel
     *      requires special calls dependent upon the direction of xfer. If this
     *      is not correct, unknown outcomes will be observed.
     * @param cmdSize Pass the number of bytes consisting of a single cmd.
     */
    void Init(uint8_t opcode, DataDir dataDir, uint16_t cmdSize);


private:
    Cmd();

    SharedMemBufferPtr mCmdBuf;
    DataDir mDataDir;
    string mCmdName;

    // Allows setting the dnvme assigned CID to the cmd after it has been SQ->Send()
    friend class SQ;
    void SetCID(uint16_t cid);
};


#endif
