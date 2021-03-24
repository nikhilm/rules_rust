// Copyright 2020 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "util/process_wrapper/system.h"
#include "util/process_wrapper/utils.h"
#include "util/worker/worker_protocol.pb.h"

// TODO: Library and app split.
using blaze::worker::WorkRequest;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::FileInputStream;

using CharType = process_wrapper::System::StrType::value_type;

bool ReadRequest(CodedInputStream *stream, WorkRequest *request)
{
  uint32_t req_len;
  if (!stream->ReadVarint32(&req_len)) {
    std::cerr << "Unable to read message length\n";
    return false;
  }

  CodedInputStream::Limit limit = stream->PushLimit(req_len);
  if (!request->MergeFromCodedStream(stream)) {
    std::cerr << "Unable to merge from stream\n";
    return false;
  }

  if (!stream->ConsumedEntireMessage()) {
    std::cerr << "Did not consume entire message\n";
    return false;
  }

  stream->PopLimit(limit);
  return true;
}

// Simple process wrapper allowing us to not depend on the shell to run a
// process to perform basic operations like capturing the output or having
// the $pwd used in command line arguments or environment variables
int PW_MAIN(int argc, const CharType* argv[], const CharType* envp[]) {
  using namespace process_wrapper;

  System::EnvironmentBlock environment_block;
  // Taking all environment variables from the current process
  // and sending them down to the child process
  for (int i = 0; envp[i] != nullptr; ++i) {
    environment_block.push_back(envp[i]);
  }

  using Subst = std::pair<System::StrType, System::StrType>;

  // This will need support for understanding param file argument.
  // As well as parsing other flags generally.

  System::StrType exec_path;
  std::vector<Subst> subst_mappings;
  System::StrType stdout_file;
  System::StrType stderr_file;
  System::StrType touch_file;
  System::StrType copy_source;
  System::StrType copy_dest;
  System::Arguments arguments;
  System::Arguments file_arguments;

  bool as_worker = false;

  // Processing current process argument list until -- is encountered
  // everthing after gets sent down to the child process
  for (int i = 1; i < argc; ++i) {
    System::StrType arg = argv[i];
    if (arg == PW_SYS_STR("--persistent_worker")) {
        as_worker = true;
    } else {
      std::cerr << "worker wrapper error: unknown argument \"" << ToUtf8(arg)
                << "\"." << '\n';
      return -1;
    }
  }

  if (!as_worker) {
    std::cerr << "Don't know how to run as non-worker yet\n";
    return -1;
  }

  CodedInputStream *stream = new CodedInputStream(new FileInputStream(0));
  while (true) {
    WorkRequest request;
    if (!ReadRequest(stream, &request)) {
      return 1;
    }
    for (int i = 0; i < request.arguments_size(); i++) {
      std::cerr << request.arguments(i) << " ";
    }
    std::cerr << "\n";
    return 1;
  }

  // Have the last values added take precedence over the first.
  // This is simpler than needing to track duplicates and explicitly override them.
  std::reverse(environment_block.begin(), environment_block.end());

  int exit_code = System::Exec(exec_path, arguments, environment_block,
                               stdout_file, stderr_file);
  if (exit_code == 0) {
    if (!touch_file.empty()) {
      std::ofstream file(touch_file);
      if (file.fail()) {
        std::cerr << "process wrapper error: failed to create touch file: \""
                  << ToUtf8(touch_file) << "\"\n";
        return -1;
      }
      file.close();
    }

    // we perform a copy of the output if necessary
    if (!copy_source.empty() && !copy_dest.empty()) {
      std::ifstream source(copy_source, std::ios::binary);
      if (source.fail()) {
        std::cerr << "process wrapper error: failed to open copy source: \""
                  << ToUtf8(copy_source) << "\"\n";
        return -1;
      }
      std::ofstream dest(copy_dest, std::ios::binary);
      if (dest.fail()) {
        std::cerr << "process wrapper error: failed to open copy dest: \""
                  << ToUtf8(copy_dest) << "\"\n";
        return -1;
      }
      dest << source.rdbuf();
    }
  }
  return exit_code;
}
