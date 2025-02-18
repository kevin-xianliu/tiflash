// Copyright 2023 PingCAP, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Core/Spiller.h>
#include <DataStreams/NativeBlockInputStream.h>
#include <Encryption/ReadBufferFromFileProvider.h>
#include <IO/CompressedReadBuffer.h>
#include <IO/ReadBufferFromFile.h>

namespace DB
{

class SpilledFilesInputStream : public IProfilingBlockInputStream
{
public:
    SpilledFilesInputStream(const std::vector<String> & spilled_files, const Block & header, const FileProviderPtr & file_provider, Int64 max_supported_spill_version);
    Block getHeader() const override;
    String getName() const override;

protected:
    Block readImpl() override;

private:
    struct SpilledFileStream
    {
        ReadBufferFromFileProvider file_in;
        CompressedReadBuffer<> compressed_in;
        BlockInputStreamPtr block_in;

        SpilledFileStream(const std::string & path, const Block & header, const FileProviderPtr & file_provider, Int64 max_supported_spill_version)
            : file_in(file_provider, path, EncryptionPath(path, ""))
            , compressed_in(file_in)
        {
            Int64 file_spill_version = 0;
            readVarInt(file_spill_version, compressed_in);
            if (file_spill_version > max_supported_spill_version)
                throw Exception(fmt::format("Spiller meet spill files that is not supported, max supported version {}, file version {}",
                                            max_supported_spill_version,
                                            file_spill_version));
            block_in = std::make_shared<NativeBlockInputStream>(compressed_in, header, file_spill_version);
        }
    };

    std::vector<String> spilled_files;
    size_t current_reading_file_index;
    Block header;
    FileProviderPtr file_provider;
    Int64 max_supported_spill_version;
    std::unique_ptr<SpilledFileStream> current_file_stream;
};

} // namespace DB
