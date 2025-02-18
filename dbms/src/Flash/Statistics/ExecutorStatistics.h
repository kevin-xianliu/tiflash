// Copyright 2022 PingCAP, Ltd.
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

#include <Common/Exception.h>
#include <Common/FmtUtils.h>
#include <Common/TiFlashException.h>
#include <DataStreams/IProfilingBlockInputStream.h>
#include <Flash/Coprocessor/DAGContext.h>
#include <Flash/Statistics/ExecutorStatisticsBase.h>
#include <Flash/Statistics/traverseExecutors.h>
#include <common/types.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <tipb/executor.pb.h>

#include <memory>
#include <vector>

namespace DB
{
class DAGContext;

template <typename ExecutorImpl>
class ExecutorStatistics : public ExecutorStatisticsBase
{
public:
    ExecutorStatistics(const tipb::Executor * executor, DAGContext & dag_context_)
        : dag_context(dag_context_)
    {
        RUNTIME_CHECK(executor->has_executor_id());
        executor_id = executor->executor_id();

        type = ExecutorImpl::type;

        getChildren(*executor).forEach([&](const tipb::Executor & child) {
            RUNTIME_CHECK(child.has_executor_id());
            children.push_back(child.executor_id());
        });
    }

    virtual String toJson() const override
    {
        FmtBuffer fmt_buffer;
        fmt_buffer.fmtAppend(
            R"({{"id":"{}","type":"{}","children":[)",
            executor_id,
            type);
        fmt_buffer.joinStr(
            children.cbegin(),
            children.cend(),
            [](const String & child, FmtBuffer & bf) { bf.fmtAppend(R"("{}")", child); },
            ",");
        fmt_buffer.fmtAppend(
            R"(],"outbound_rows":{},"outbound_blocks":{},"outbound_bytes":{},"execution_time_ns":{})",
            base.rows,
            base.blocks,
            base.bytes,
            base.execution_time_ns);
        if constexpr (ExecutorImpl::has_extra_info)
        {
            fmt_buffer.append(",");
            appendExtraJson(fmt_buffer);
        }
        fmt_buffer.append("}");
        return fmt_buffer.toString();
    }

    void collectRuntimeDetail() override
    {
        const auto & profile_streams_map = dag_context.getProfileStreamsMap();
        auto it = profile_streams_map.find(executor_id);
        if (it != profile_streams_map.end())
        {
            for (const auto & input_stream : it->second)
            {
                if (auto * p_stream = dynamic_cast<IProfilingBlockInputStream *>(input_stream.get()); p_stream)
                {
                    const auto & profile_info = p_stream->getProfileInfo();
                    base.append(profile_info);
                }
            }
        }
        if constexpr (ExecutorImpl::has_extra_info)
        {
            collectExtraRuntimeDetail();
        }
    }

    static bool isMatch(const tipb::Executor * executor)
    {
        return ExecutorImpl::isMatch(executor);
    }

protected:
    String executor_id;
    String type;

    std::vector<String> children;

    DAGContext & dag_context;

    virtual void appendExtraJson(FmtBuffer &) const {}

    virtual void collectExtraRuntimeDetail() {}
};
} // namespace DB
