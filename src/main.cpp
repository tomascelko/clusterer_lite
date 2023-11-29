
#include "data_flow/dataflow_controller.h"
#include "data_structs/calibration.h"
#include "data_structs/node_args.h"
#include "devices/current_device.h"
#include "nodes/data_reader.h"
#include "nodes/raw_data_reader.h"
#include "other/utils.h"
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>

bool try_buffer_clustering()
{
  /* Example usage for buffer processing, feel free to custimize as needed:
  // Processing buffer from memory (and returning clusters using callback)
  const std::string data_file = ... optional
  const std::sting calib_folder = ... optional

  //1. prepare the char buffer, for testing you can use read_to_buffer as shown
  below char *buffer; below uint64_t file_size =
  io_utils::read_to_buffer(data_file, buffer);

  //2. load calibration from file
  calibration calib = calibration(calib_folder,
  current_chip::chip_type::size());

  //or create your own calibration matrices and load them to the calibration,
  //ensure that they have correct shape size_y() x size_x()
  // the shape should be the same as in a.txt file


  std::vector<std::vector<double>> a_values{};
  std::vector<std::vector<double>> b_values{};
  std::vector<std::vector<double>> c_values{};
  std::vector<std::vector<double>> t_values{};

  calibration calib = calibration(a_values, b_values, c_values, t_values);


  //3. use correct dataflow template arguments:
  // - raw_char_buffer - as we are processing data from char buffer
  // - raw_data_reader - the object capable of reading from this buffer

  std::vector<cluster<mm_hit>> clusters;
  dataflow_controller<raw_data_reader, raw_char_buffer> controller(
      calib,
      [&clusters](auto begin, auto end)
      {
        // Do some processing of clusters if needed and finally store them
        // elsewhere;
        for (auto it = begin; it != end; ++it)
          clusters.push_back(*it);
      });

  // Note: all .run_pixel_list_clustering() calls have analogical
  // .run_temporal_split_clustering() variant which uses different clustering
  // algorithm - it will be optimized very soon*/
  return false;
}

int main(int argc, char **argv)
{
  if (try_buffer_clustering())
    return 0;
  const uint16_t expected_arg_count = 3;
  if (argc != expected_arg_count)
  {

    std::cout << "Error, passed " << argc - 1
              << " arguments, but 2 arguments and 1 option is expected ([-t or "
                 "-b] [data file] [calibration folder])"
              << std::endl;
    return 0;
  }
  std::vector<std::string> args(argv + 1, argv + argc);
  /*{
      "-b",
      "/home/celko/Work/repos/clusterer_data/pion/pions_180GeV_deg_0.raw",
      //  "/home/celko/Work/repos/clusterer_data/pion/pions_180GeV_deg_0.txt",

      "/home/celko/Work/repos/clusterer_data/calib/I4-W00076",

  };*/
  const std::string processing_option = args[0];
  const std::string data_file = args[1];
  const std::string calib_folder = args[2];

  if (args[0] == "-t")
  {
    // choose approperiate dataflow_controller arguments
    // reading from text stream
    dataflow_controller<data_reader, std::ifstream> controller(calib_folder);
    controller.run_pixel_list_clustering(data_file);
  }
  else if (args[0] == "-b")
  {
    // reading from binary stream
    dataflow_controller<raw_data_reader, std::ifstream> controller(
        calib_folder);
    controller.run_pixel_list_clustering(data_file);
  }
  else
  {
    std::cout << "Invalid file option passed '" << args[0]
              << "', expected -t or -b" << std::endl;
  }

  std::cout << "Finished processing" << std::endl;
}