#include "nodeclient.hpp"
#include "node.grpc.pb.h"

using namespace node;


int main(int argc, char** argv) {
  // Expect only arg: --db_path=path/to/route_guide_db.json.
  NodeClient guide(
      grpc::CreateChannel("localhost:50051",
                          grpc::InsecureChannelCredentials()));

  std::cout << "-------------- GetMessages --------------" << std::endl;
  Msg x;
  x.set_data("Powerhouse chicken");
  guide.data.push_back(x);
  guide.data.push_back(x);
  guide.data.push_back(x);
  
  guide.PutMessages();

  return 0;
}
