/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "node.grpc.pb.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using node::MsgReq;
using node::Msg;
using node::Node;
using std::chrono::system_clock;
using namespace node;

extern std::vector<std::string> msgtmp;
extern std::vector<std::string> outbox;

class NodeImpl final : public Node::Service {
 public:
  Status NewRound(ServerContext* context, const node::MsgReq* req, node::MsgReq* ans) override;

  Status DumpMessages(ServerContext* context,
   const node::MsgReq* req, ServerWriter<node::Msg>* writer) override;

  Status PutMessages(ServerContext* context, ServerReader<node::Msg>* reader,node::MsgReq* response) override;

  std::vector<std::string> GetMsgArray();

 private:
};


Status NodeImpl::NewRound(ServerContext* context, const node::MsgReq* req,
                    node::MsgReq* ans) {
    ans->set_yes(true);
    // Do nothing for now
    return Status::OK;
}

Status NodeImpl::DumpMessages(ServerContext* context, const node::MsgReq* req, ServerWriter<node::Msg>* writer) {
    if(!outbox.empty()){
        for(auto x : outbox){
            node::Msg g;
            g.set_data(x);
            writer->Write(g);
        }
    }else{
        return Status::error_code();
    }
    return Status::OK;
}

Status NodeImpl::PutMessages(ServerContext* context, ServerReader<node::Msg>* reader,node::MsgReq* response) {
    node::Msg m;

    system_clock::time_point start_time = system_clock::now();
    while (reader->Read(&m)) {
        msgtmp.push_back(m.data());
    }
    system_clock::time_point end_time = system_clock::now();
    response->set_yes(true);
    return Status::OK;
  }

inline void RunServer() {
  std::string server_address("0.0.0.0:50051");
  NodeImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

// int main(int argc, char** argv) {
//   // Expect only arg: --db_path=path/to/route_guide_db.json.
//   RunServer();

//   return 0;
// }
