#pragma once
#include "../devices/current_device.h"
#include "../nodes/node_package.h"
#include "../other/mm_stream.h"
#include <variant>

class dataflow_controller
{
  using reader_type = data_reader<std::ifstream>;
  using adapter_type = burda_to_mm_hit_adapter;
  using sorter_type = hit_sorter<mm_hit>;
  using clusterer_type = pixel_list_clusterer;
  using printer_type = data_printer<cluster<mm_hit>, mm_write_stream>;

  reader_type reader_;
  adapter_type adapter_;
  sorter_type sorter_;
  clusterer_type clusterer_;
  printer_type printer_;

  bool done();
  void finalize();

public:
  dataflow_controller(const node_args &args, const std::string &data_file,
                      const std::string &calib_folder);
  void run();
};