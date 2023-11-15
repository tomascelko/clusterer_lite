#include "data_flow/dataflow_controller.h"

bool dataflow_controller::done() { return reader_.done(); }

void dataflow_controller::finalize()
{
  auto last_hits = sorter_.process_remaining();
  auto clusters = clusterer_.process_hits(last_hits.begin(), last_hits.end());
  auto final_clusters = clusterer_.process_remaining();
  printer_.process_data(clusters.begin(), clusters.end());
  printer_.process_data(final_clusters.begin(), final_clusters.end());
  printer_.close();
}

std::string create_output_name(const std::string &input_name)
{
  const char suffix_separator = '.';
  std::string output_name = input_name;
  auto last_dot_index = input_name.rfind('.');
  if (last_dot_index != std::string::npos)
    output_name = output_name.substr(0, last_dot_index);
  return output_name + "_clustered";
}

dataflow_controller::dataflow_controller(const node_args &args,
                                         const std::string &data_file,
                                         const std::string &calib_folder)
  : reader_(data_file),
    adapter_(calibration(calib_folder, current_chip::chip_type::size())),
    sorter_(), clusterer_(args),
    printer_(new mm_write_stream(create_output_name(data_file)))
{
}

void dataflow_controller::run()
{
  auto new_hit = reader_.process_hit();
  while (!done())
  {
    auto hits_optional = sorter_.process_hit(adapter_.process_hit(new_hit));
    if (hits_optional.has_value())
    {
      auto clusters = clusterer_.process_hits(hits_optional.value().begin(),
                                              hits_optional.value().end());
      printer_.process_data(clusters.begin(), clusters.end());
    }
    new_hit = reader_.process_hit();
  }
  finalize();
}