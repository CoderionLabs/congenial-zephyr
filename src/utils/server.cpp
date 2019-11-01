#include "nodeserver.hpp"
#include <vector>

using namespace node;


std::vector<std::string> msgtmp;
std::vector<std::string> outbox;


int main(int argc, char** argv) {
  // Expect only arg: --db_path=path/to/route_guide_db.json.
  RunServer();

  return 0;
}
