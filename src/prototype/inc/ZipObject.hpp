//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "ObjectBase.hpp"
#include "Helpers.hpp"

#include <vector>
#include <memory>

// APPNOTE.TXT section 4.5.3
class Zip64ExtendedInformation final : public Meta::StructuredObject<
    Meta::Field2Bytes,  // 0 - tag for the "extra" block type               2 bytes(0x0001)
    Meta::Field2Bytes,  // 1 - size of this "extra" block                   2 bytes
    Meta::Field8Bytes,  // 2 - Original uncompressed file size              8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.
    Meta::Field8Bytes,  // 3 - Compressed file size                         8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.    
    Meta::Field8Bytes   // 4 - Offset of local header record                8 bytes
    //Meta::Field4Bytes // 5 - number of the disk on which the file starts  4 bytes -- ITS A FAAKEE!
>
{
public:
    Zip64ExtendedInformation(std::uint64_t uncompressedSize, std::uint64_t compressedSize, std::uint64_t relativeOffset);
};

// 4.3.12
class CentralDirectoryFileHeader final : public Meta::StructuredObject<
    Meta::Field4Bytes, // 0 - central file header signature   4 bytes(0x02014b50)
    Meta::Field2Bytes, // 1 - version made by                 2 bytes
    Meta::Field2Bytes, // 2 - version needed to extract       2 bytes
    Meta::Field2Bytes, // 3 - general purpose bit flag        2 bytes
    Meta::Field2Bytes, // 4 - compression method              2 bytes
    Meta::Field2Bytes, // 5 - last mod file time              2 bytes
    Meta::Field2Bytes, // 6 - last mod file date              2 bytes
    Meta::Field4Bytes, // 7 - crc - 32                        4 bytes
    Meta::Field4Bytes, // 8 - compressed size                 4 bytes
    Meta::Field4Bytes, // 9 - uncompressed size               4 bytes
    Meta::Field2Bytes, //10 - file name length                2 bytes
    Meta::Field2Bytes, //11 - extra field length              2 bytes
    Meta::Field2Bytes, //12 - file comment length             2 bytes
    Meta::Field2Bytes, //13 - disk number start               2 bytes
    Meta::Field2Bytes, //14 - internal file attributes        2 bytes
    Meta::Field4Bytes, //15 - external file attributes        4 bytes
    Meta::Field4Bytes, //16 - relative offset of local header 4 bytes
    Meta::FieldNBytes, //17 - file name                       (variable size)
    Meta::FieldNBytes  //18 - extra field                     (variable size)
    //Meta::FieldNBytes  //19 - file comment                  (variable size) NOT USED
    >
{
public:
    CentralDirectoryFileHeader(std::string& name, std::uint32_t crc, std::uint64_t compressedSize,
        std::uint64_t uncompressedSize, std::uint64_t relativeOffset, std::uint16_t compressionMethod);
};

// 4.3.9. Header is optional, but we always use it. 4.3.9.3. We always use zip64, compress and uncompressed
// size are 8 and not 4 bytes.
class DataDescriptor final : public Meta::StructuredObject<
    Meta::Field4Bytes, // 0 - data descriptor header signature  4 bytes(0x08074b50)
    Meta::Field4Bytes, // 1 - crc -32                           4 bytes
    Meta::Field8Bytes, // 2 - compressed size                   8 bytes(zip64)
    Meta::Field8Bytes  // 3 - uncompressed size                 8 bytes(zip64)
>
{
public:
    DataDescriptor(std::uint32_t crc, std::uint64_t compressSize, std::uint64_t uncompressSize);
};

// 4.3.7
class LocalFileHeader final : public Meta::StructuredObject< 
    Meta::Field4Bytes,  // 0 - local file header signature     4 bytes(0x04034b50)
    Meta::Field2Bytes,  // 1 - version needed to extract       2 bytes
    Meta::Field2Bytes,  // 2 - general purpose bit flag        2 bytes
    Meta::Field2Bytes,  // 3 - compression method              2 bytes
    Meta::Field2Bytes,  // 4 - last mod file time              2 bytes
    Meta::Field2Bytes,  // 5 - last mod file date              2 bytes
    Meta::Field4Bytes,  // 6 - crc - 32                        4 bytes
    Meta::Field4Bytes,  // 7 - compressed size                 4 bytes
    Meta::Field4Bytes,  // 8 - uncompressed size               4 bytes
    Meta::Field2Bytes,  // 9 - file name length                2 bytes
    Meta::Field2Bytes,  // 10- extra field length              2 bytes
    Meta::FieldNBytes   // 11- file name                       (variable size)
    // Meta::FieldNBytes   // 12- extra field                  (variable size) NOT USED
>
{
public:
    LocalFileHeader(std::string& name, bool isCompressed, std::uint64_t offset);

    std::string GetName() { return std::string(Field<11>().value.begin(), Field<11>().value.end()); }
    std::uint64_t GetOffset() { return m_offset; }
    std::uint16_t GetCompressionMethod() { return Field<3>().value; }
protected:
    std::uint64_t m_offset;
};

// 4.3.14
class Zip64EndOfCentralDirectoryRecord final : public Meta::StructuredObject< 
    Meta::Field4Bytes, // 0 - zip64 end of central dir signature                            4 bytes(0x06064b50)
    Meta::Field8Bytes, // 1 - size of zip64 end of central directory record                 8 bytes
    Meta::Field2Bytes, // 2 - version made by                                               2 bytes
    Meta::Field2Bytes, // 3 - version needed to extract                                     2 bytes
    Meta::Field4Bytes, // 4 - number of this disk                                           4 bytes
    Meta::Field4Bytes, // 5 - number of the disk with the start of the central directory    4 bytes
    Meta::Field8Bytes, // 6 - total number of entries in the central directory on this disk 8 bytes
    Meta::Field8Bytes, // 7 - total number of entries in the central directory              8 bytes
    Meta::Field8Bytes, // 8 - size of the central directory                                 8 bytes
    Meta::Field8Bytes  // 9 - offset of start of central directory with respect to the
                       //     starting disk number                                          8 bytes
    // Meta::FieldNBytes  //10 - zip64 extensible data sector                                  (variable size) NOT USED
>
{
public:
    Zip64EndOfCentralDirectoryRecord(std::uint64_t numCentralDirs, std::uint64_t sizeCentralDir, std::uint64_t offsetStartCentralDirectory);
    // Gets & sets?
};

// 4.3.15
class Zip64EndOfCentralDirectoryLocator final : public Meta::StructuredObject<
    Meta::Field4Bytes,  // 0 - zip64 end of central dir locator signature        4 bytes(0x07064b50)
    Meta::Field4Bytes,  // 1 - number of the disk with the start of the zip64
                        //     end of central directory                          4 bytes
    Meta::Field8Bytes,  // 2 - relative offset of the zip64 end of central
                        //     directory record                                  8 bytes
    Meta::Field4Bytes   // 3 - total number of disks                             4 bytes
>
{
public:
    Zip64EndOfCentralDirectoryLocator(std::uint64_t zip64EndCdrOffset);
    // Gets & sets?
};

// 4.3.16
class EndCentralDirectoryRecord final : public Meta::StructuredObject<
    Meta::Field4Bytes,  // 0 - end of central dir signature              4 bytes  (0x06054b50)
    Meta::Field2Bytes,  // 1 - number of this disk                       2 bytes
    Meta::Field2Bytes,  // 2 - number of the disk with the start of the
                        //     central directory                         2 bytes
    Meta::Field2Bytes,  // 3 - total number of entries in the central
                        //     directory on this disk                    2 bytes
    Meta::Field2Bytes,  // 4 - total number of entries in the central
                        //     directory                                 2 bytes
    Meta::Field4Bytes,  // 5 - size of the central directory             4 bytes
    Meta::Field4Bytes,  // 6 - offset of start of central directory with
                        //     respect to the starting disk number       4 bytes
    Meta::Field2Bytes   // 7 - .ZIP file comment length                  2 bytes
    // Meta::FieldNBytes   // 8 - .ZIP file comment                      (variable size) NOT USED
>
{
public:
    EndCentralDirectoryRecord();
};

class ZipObjectWriter final
{

public:
    ZipObjectWriter(std::string& name);

    std::unique_ptr<LocalFileHeader> WriteLfh(std::string& name, bool isCompressed);
    void WriteBuffer(std::vector<std::uint8_t>& buffer);
    void WriteCdh(LocalFileHeader& lfh, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize);
    void Close();

protected:
    enum class State
    {
        ReadyForLfhOrClose,
        ReadyForBuffer,
        ReadyForBufferOrCdh,
        Closed,
    };

    State m_state;
    std::string m_packageName;
    std::unique_ptr<File> m_package;
    std::vector<std::unique_ptr<CentralDirectoryFileHeader>> m_cdhs;
};
