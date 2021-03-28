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
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "util/process_wrapper/system.h"
#include "util/process_wrapper/utils.h"
#include "util/worker/worker_protocol.pb.h"

// TODO: Library and app split.
using blaze::worker::WorkRequest;
using blaze::worker::WorkResponse;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;

using namespace process_wrapper;

using CharType = process_wrapper::System::StrType::value_type;

bool ReadRequest(CodedInputStream &stream, WorkRequest *request)
{
  uint32_t req_len;
  if (!stream.ReadVarint32(&req_len)) {
    std::cerr << "Unable to read message length\n";
    return false;
  }

  CodedInputStream::Limit limit = stream.PushLimit(req_len);
  if (!request->MergeFromCodedStream(&stream)) {
    std::cerr << "Unable to merge from stream\n";
    return false;
  }

  if (!stream.ConsumedEntireMessage()) {
    std::cerr << "Did not consume entire message\n";
    return false;
  }

  stream.PopLimit(limit);
  return true;
}

std::unique_ptr<WorkResponse> HandleRequest(const WorkRequest &request, const System::StrType& exec_path, const System::EnvironmentBlock& environment_block) {
  System::StrType stdout_file;
  System::StrType stderr_file;
  System::StrType copy_source;
  System::StrType copy_dest;
  System::Arguments arguments(request.arguments_size());

  for (int i = 0; i < request.arguments_size(); i++) {
    // TODO: Probably some way to copy the entire memory into the vector in one go.
    arguments[i] = request.arguments(i);
  }
  // TODO: Add incremental arg.
  //

  // Create a file to write stderr to.
  std::stringstream fn_stream;
  fn_stream << System::GetWorkingDirectory() << "/stderr_" << request.request_id() << ".log";
  stderr_file = fn_stream.str();

  int exit_code = System::Exec(exec_path, arguments, environment_block,
                               stdout_file, stderr_file);
  std::ifstream source(fn_stream.str(), std::ios::binary);
  std::string stderr_output;
  if (source.fail()) {
    stderr_output = "[worker] Error getting stderr\n";
  } else {
    std::stringstream stderr_stream;
    stderr_stream << source.rdbuf();
    stderr_output = stderr_stream.str();
  }

  std::unique_ptr<WorkResponse> response(new WorkResponse());
  response->set_exit_code(exit_code);
  response->set_request_id(request.request_id());
  response->set_output(stderr_output);
  return response;
}


// Simple process wrapper allowing us to not depend on the shell to run a
// process to perform basic operations like capturing the output or having
// the $pwd used in command line arguments or environment variables
int PW_MAIN(int argc, const CharType* argv[], const CharType* envp[]) {
  System::StrType exec_path;
  System::EnvironmentBlock environment_block;
  // Taking all environment variables from the current process
  // and sending them down to the child process
  for (int i = 0; envp[i] != nullptr; ++i) {
    environment_block.push_back(envp[i]);
  }

  // Have the last values added take precedence over the first.
  // This is simpler than needing to track duplicates and explicitly override them.
  std::reverse(environment_block.begin(), environment_block.end());

  // This will need support for understanding param file argument.
  // As well as parsing other flags generally.

  bool as_worker = false;

  // Processing current process argument list until -- is encountered
  // everthing after gets sent down to the child process
  for (int i = 1; i < argc; ++i) {
    System::StrType arg = argv[i];
    if (arg == PW_SYS_STR("--persistent_worker")) {
      as_worker = true;
    } else if (arg == PW_SYS_STR("--compiler")) {
      if (++i == argc) {
        std::cerr << "--compiler flag missing argument\n";
        return -1;
      }
      exec_path = argv[i];
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

  std::unique_ptr<CodedInputStream> input(new CodedInputStream(new FileInputStream(0)));
  // TODO: Figure out ownership.
  FileOutputStream *out = new FileOutputStream(1);
  std::unique_ptr<CodedOutputStream> output(new CodedOutputStream(out));

  while (true) {
    WorkRequest request;
    if (!ReadRequest(*input, &request)) {
      return 1;
    }

    std::unique_ptr<WorkResponse> response;
    if ((response = HandleRequest(request, exec_path, environment_block)) == nullptr) {
      return 1;
    }
    uint32_t size = response->ByteSize();
    output->WriteVarint32(size);
    response->SerializeWithCachedSizes(output.get());
    if (output->HadError()) {
      std::cerr << "Error serializing response\n";
      return 1;
    }
    out->Flush();
  }

}
