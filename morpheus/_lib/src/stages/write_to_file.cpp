/**
 * SPDX-FileCopyrightText: Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "morpheus/stages/write_to_file.hpp"

#include "morpheus/utilities/matx_util.hpp"

#include <exception>
#include <memory>
#include <mutex>
#include <string>

namespace morpheus {
// Component public implementations
// ************ WriteToFileStage **************************** //
WriteToFileStage::WriteToFileStage(const std::string &filename, std::ios::openmode mode, FileTypes file_type) :
  PythonNode(base_t::op_factory_from_sub_fn(build_operator())),
  m_is_first(true)
{
    if (file_type == FileTypes::Auto)
    {
        file_type = determine_file_type(filename);
    }

    if (file_type == FileTypes::CSV)
    {
        m_write_func = [this](auto &&PH1) { write_csv(std::forward<decltype(PH1)>(PH1)); };
    }
    else if (file_type == FileTypes::JSON)
    {
        m_write_func = [this](auto &&PH1) { write_json(std::forward<decltype(PH1)>(PH1)); };
    }
    else  // FileTypes::AUTO
    {
        LOG(FATAL) << "Unknown extension for file: " << filename;
        throw std::runtime_error("Unknown extension");
    }

    // Enable throwing exceptions in case something fails.
    m_fstream.exceptions(std::fstream::failbit | std::fstream::badbit);

    m_fstream.open(filename, mode);
}

void WriteToFileStage::write_json(WriteToFileStage::sink_type_t &msg)
{
    // Call df_to_json passing our fstream
    df_to_json(msg->get_info(), m_fstream);
}

void WriteToFileStage::write_csv(WriteToFileStage::sink_type_t &msg)
{
    // Call df_to_csv passing our fstream
    df_to_csv(msg->get_info(), m_fstream, m_is_first);
}

void WriteToFileStage::close()
{
    if (m_fstream.is_open())
    {
        m_fstream.close();
    }
}

WriteToFileStage::subscribe_fn_t WriteToFileStage::build_operator()
{
    return [this](rxcpp::observable<sink_type_t> input, rxcpp::subscriber<source_type_t> output) {
        return input.subscribe(rxcpp::make_observer<sink_type_t>(
            [this, &output](sink_type_t msg) {
                this->m_write_func(msg);
                m_is_first = false;
                output.on_next(std::move(msg));
            },
            [&](std::exception_ptr error_ptr) {
                this->close();
                output.on_error(error_ptr);
            },
            [&]() {
                this->close();
                output.on_completed();
            }));
    };
}

// ************ WriteToFileStageInterfaceProxy ************* //
std::shared_ptr<srf::segment::Object<WriteToFileStage>> WriteToFileStageInterfaceProxy::init(
    srf::segment::Builder &builder,
    const std::string &name,
    const std::string &filename,
    const std::string &mode,
    FileTypes file_type)
{
    std::ios::openmode fsmode = std::ios::out;

    if (StringUtil::str_contains(mode, "r"))
    {
        // Dont support reading
        throw std::invalid_argument("Read mode ('r') is not supported by WriteToFileStage. Mode: " + mode);
    }
    if (StringUtil::str_contains(mode, "b"))
    {
        // Dont support binary
        throw std::invalid_argument("Binary mode ('b') is not supported by WriteToFileStage. Mode: " + mode);
    }
    if (StringUtil::str_contains(mode, "+"))
    {
        // Dont support binary
        throw std::invalid_argument("Read/Write mode ('+') is not supported by WriteToFileStage. Mode: " + mode);
    }

    // Default is write
    if (StringUtil::str_contains(mode, "w"))
    {
        fsmode |= std::ios::trunc;
    }
    else if (StringUtil::str_contains(mode, "a"))
    {
        // Check for appending
        fsmode |= std::ios::app;
    }
    else
    {
        // Ensure something was set
        throw std::runtime_error(std::string("Unsupported file mode. Must choose either 'w' or 'a'. Mode: ") + mode);
    }

    auto stage = builder.construct_object<WriteToFileStage>(name, filename, fsmode, file_type);

    return stage;
}
}  // namespace morpheus
