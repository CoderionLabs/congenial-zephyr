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

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "node.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using node::MsgReq;
using node::Msg;
using node::Node;
using namespace node;

// inline Msg MakeMsg(std::string msg) {
//   Msg m;
//   m.set_data(msg);
//   return m;
// }

// inline MsgReq MakeReq(bool req) {
//   MsgReq f;
//   f.set_yes(req);
//   return f;
// }


class NodeClient {
 public:
  std::vector<Msg> data;
  NodeClient(std::shared_ptr<Channel> channel) : stub_(Node::NewStub(channel)) {}
  void NewRound();
  void DumpMessages();
  void PutMessages();
  bool quiet = false;
 private:
  std::unique_ptr<Node::Stub> stub_;
  void print();
  bool GetMsqReq(const MsgReq& msg, MsgReq* recv);
};


void NodeClient::print(std::string str){
    if(quiet){
        std::cout << str << std::endl;
    }
}

//NodeClient::NodeClient(std::shared_ptr<Channel> channel) : 
bool NodeClient::GetMsqReq(const MsgReq& msg, MsgReq* recv) {
    ClientContext context;
    
    Status status = stub_->NewRound(&context, msg, recv);
    if (!status.ok()) {
      print("GetFeature rpc failed.");
      return false;
    }
    return recv->yes();
}
  
void NodeClient::NewRound(){
    MsgReq req;
    req.set_yes(true);
    MsgReq recv;
    GetMsqReq(req, &recv);
}

void NodeClient::DumpMessages() {
    node::MsgReq req;
    req.set_yes(true);
    Msg recv;
    ClientContext context;

    print("Dump Messages Activated");

    std::unique_ptr<ClientReader<Msg> > reader(
        stub_->DumpMessages(&context, req));
    while (reader->Read(&recv)) {
      // Do nothing
      data.push_back(recv);
    }
    Status status = reader->Finish();
    if (status.ok()) {
      print("DumpMessages rpc succeeded.");
    } else {
       print("DumpMessages rpc failed.");
    }
}

void NodeClient::PutMessages() {
    Msg msg;
    MsgReq recv;
    ClientContext context;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> delay_distribution(
        500, 1500);

    std::unique_ptr<ClientWriter<Msg> > writer(stub_->PutMessages(&context, &recv));
    for(auto x : data){
        print("WRITING " << x.data());
        writer->Write(x);
        std::this_thread::sleep_for(std::chrono::milliseconds(
          delay_distribution(generator)));
    }
    
    writer->WritesDone();
    Status status = writer->Finish();
    if (status.ok()) {
        if(recv.yes() == true){
            print("THIS WORKS");
            //std::cout << "THIS WORKS" << std::endl;
        }
    } else {
        print("PutMessages rpc failed");
        //std::cout << "PutMessages rpc failed." << std::endl;
    }
}

// int main(int argc, char** argv) {
//   // Expect only arg: --db_path=path/to/route_guide_db.json.
//   NodeClient guide(
//       grpc::CreateChannel("localhost:50051",
//                           grpc::InsecureChannelCredentials()));

//   std::cout << "-------------- GetMessages --------------" << std::endl;
//   guide.DumpMessages();

//   return 0;
// }

