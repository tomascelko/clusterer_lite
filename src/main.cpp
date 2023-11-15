
#include "data_flow/dataflow_controller.h"
#include "data_structs/node_args.h"
int main(int argc, char **argv)
{
  node_args args;
  dataflow_controller controller(args, "../../../clusterer_data/lead/deg90_2.txt", "../../../clusterer_data/calib/F4-W00076");
  controller.run();
}