
#include "data_flow/dataflow_controller.h"
#include "data_structs/node_args.h"

int main(int argc, char **argv)
{
  const uint16_t expected_arg_count = 3;
  if (argc != expected_arg_count)
  {
    std::cout << "Error, passed " << argc - 1
              << " arguments, but 2 are expected." << std::endl;
    return 0;
  }
  node_args args;
  dataflow_controller controller(args, argv[1], argv[2]);
  std::cout << "Started processing..." << std::endl;
  controller.run();
  std::cout << "Finished processing" << std::endl;
}