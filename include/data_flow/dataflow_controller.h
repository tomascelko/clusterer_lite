#pragma once
#include "../devices/current_device.h"
#include "../nodes/node_package.h"
#include "../other/mm_stream.h"
#include "data_structs/burda_hit.h"
#include "data_structs/node_args.h"
#include "nodes/raw_data_reader.h"
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <variant>

template <template <typename> class reader_type, typename buffer_type>
class dataflow_controller
{

  /*using reader_type = // if buffer_type = data_reader<std::ifstream>;
      raw_data_reader<buffer_type>;*/

  enum class runtime_configuration
  {
    USE_HDD_IO,
    NO_HDD_IO
  };
  using adapter_type = burda_to_mm_hit_adapter;
  using sorter_type = hit_sorter<mm_hit>;
  using clusterer_type = pixel_list_clusterer;
  using mm_printer_type = data_printer<cluster<mm_hit>, mm_write_stream>;
  using burda_binary_printer_type = data_printer<burda_hit, std::ofstream>;
  using cluster_splitter_type = cluster_splitter;
  using temporal_clusterer_type = temporal_clusterer;
  using result_callback_type =
      std::function<void(std::vector<cluster<mm_hit>>::const_iterator,
                         std::vector<cluster<mm_hit>>::const_iterator)>;
  std::unique_ptr<reader_type<buffer_type>> reader_;
  std::unique_ptr<adapter_type> adapter_;
  std::unique_ptr<sorter_type> sorter_;
  std::unique_ptr<clusterer_type> clusterer_;
  std::unique_ptr<mm_printer_type> printer_;
  std::unique_ptr<burda_binary_printer_type> raw_printer_;
  std::unique_ptr<cluster_splitter_type> cluster_splitter_;
  std::unique_ptr<temporal_clusterer_type> temp_clusterer_;

  result_callback_type result_callback_;
  runtime_configuration runtime_config_;
  const uint16_t MIN_BUFFER_SIZE = 128;

  bool done() { return reader_->done(); }

  void finalize()
  {
    auto last_hits = sorter_->process_remaining();
    clusterer_->process_hits(last_hits.begin(), last_hits.end());
    auto final_clusters = clusterer_->process_remaining();
    if (runtime_config_ == runtime_configuration::USE_HDD_IO)
    {
      printer_->process_data(final_clusters.begin(), final_clusters.end());
      printer_->close();
    }
    else
    {
      result_callback_(final_clusters.cbegin(), final_clusters.cend());
    }
  }

  void two_step_finalize()
  {
    auto last_hits = sorter_->process_remaining();
    temp_clusterer_->process_hits(last_hits.begin(), last_hits.end());
    auto final_clusters = temp_clusterer_->process_remaining();
    cluster_splitter_->process_data(final_clusters.begin(),
                                    final_clusters.end());
    auto final_splitted_clusters = cluster_splitter_->process_remaining();
    if (runtime_config_ == runtime_configuration::USE_HDD_IO)
    {
      printer_->process_data(final_splitted_clusters.begin(),
                             final_splitted_clusters.end());
      printer_->close();
    }
    else
    {
      result_callback_(final_splitted_clusters.cbegin(),
                       final_splitted_clusters.cend());
    }
  }

  std::string create_clustered_output_name(const std::string &input_name)
  {
    const char suffix_separator = '.';
    std::string output_name = input_name;
    auto last_dot_index = input_name.rfind('.');
    if (last_dot_index != std::string::npos)
      output_name = output_name.substr(0, last_dot_index);
    return output_name + "_clustered";
  }

public:
  template <typename T = buffer_type,
            typename std::enable_if_t<std::is_same<T, raw_char_buffer>::value,
                                      int> = 0>
  dataflow_controller(const calibration &calib, result_callback_type &&callback,
                      const node_args &args = node_args())
    : adapter_(std::make_unique<adapter_type>(calib)),
      sorter_(std::make_unique<sorter_type>()),
      clusterer_(std::make_unique<clusterer_type>(args)),
      temp_clusterer_(std::make_unique<temporal_clusterer_type>()),
      cluster_splitter_(std::make_unique<cluster_splitter_type>()),
      result_callback_(callback),
      runtime_config_(runtime_configuration::NO_HDD_IO)

  {
  }

  template <
      typename T = buffer_type,
      typename std::enable_if_t<std::is_same<T, std::ifstream>::value, int> = 0>
  dataflow_controller(const std::string &calib_folder,
                      const node_args &args = node_args())

    : adapter_(std::make_unique<adapter_type>(
          calibration(calib_folder, current_chip::chip_type::size()))),
      sorter_(std::make_unique<sorter_type>()),
      clusterer_(std::make_unique<clusterer_type>(args)),
      temp_clusterer_(std::make_unique<temporal_clusterer_type>()),
      cluster_splitter_(std::make_unique<cluster_splitter_type>()),
      runtime_config_(runtime_configuration::USE_HDD_IO)

  {
  }

  template <typename T = buffer_type>
  typename std::enable_if_t<std::is_same<T, raw_char_buffer>::value>
  run_pixel_list_clustering(char *data_pointer, uint64_t size)
  {
    reader_ = std::move(
        std::make_unique<reader_type<buffer_type>>(data_pointer, size));

    auto new_hit = reader_->process_hit();
    while (!done())
    {
      sorter_->process_hit(adapter_->process_hit(new_hit));
      if (sorter_->result_hits().size() > MIN_BUFFER_SIZE)
      {
        clusterer_->process_hits(sorter_->result_hits().begin(),
                                 sorter_->result_hits().end());
        sorter_->result_hits().clear();
      }
      if (clusterer_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        result_callback_(clusterer_->result_clusters().cbegin(),
                         clusterer_->result_clusters().cend());
        clusterer_->result_clusters().clear();
      }
      new_hit = reader_->process_hit();
    }
    finalize();
  }

  template <typename T = buffer_type>
  typename std::enable_if_t<std::is_same<T, std::ifstream>::value>
  run_pixel_list_clustering(const std::string &data_file)
  {
    reader_ = std::move(std::make_unique<reader_type<buffer_type>>(data_file));
    printer_ = std::move(std::make_unique<mm_printer_type>(
        new mm_write_stream(create_clustered_output_name(data_file))));

    auto new_hit = reader_->process_hit();
    while (!done())
    {
      sorter_->process_hit(adapter_->process_hit(new_hit));
      if (sorter_->result_hits().size() > MIN_BUFFER_SIZE)
      {
        clusterer_->process_hits(sorter_->result_hits().begin(),
                                 sorter_->result_hits().end());
        sorter_->result_hits().clear();
      }
      if (clusterer_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        printer_->process_data(clusterer_->result_clusters().begin(),
                               clusterer_->result_clusters().end());
        clusterer_->result_clusters().clear();
      }
      new_hit = reader_->process_hit();
    }
    finalize();
  }

  template <typename T = buffer_type>
  typename std::enable_if_t<std::is_same<T, raw_char_buffer>::value>
  run_temporal_split_clustering(char *data_pointer, uint64_t size)
  {
    reader_ = std::move(
        std::make_unique<reader_type<buffer_type>>(data_pointer, size));

    auto new_hit = reader_->process_hit();
    while (!done())
    {
      sorter_->process_hit(adapter_->process_hit(new_hit));
      if (sorter_->result_hits().size() > MIN_BUFFER_SIZE)
      {
        temp_clusterer_->process_hits(sorter_->result_hits().begin(),
                                      sorter_->result_hits().end());
        sorter_->result_hits().clear();
      }

      if (temp_clusterer_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        cluster_splitter_->process_data(
            temp_clusterer_->result_clusters().begin(),
            temp_clusterer_->result_clusters().end());
        temp_clusterer_->result_clusters().clear();
      }

      if (cluster_splitter_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        result_callback_(cluster_splitter_->result_clusters().cbegin(),
                         cluster_splitter_->result_clusters().cend());
        cluster_splitter_->result_clusters().clear();
      }
      new_hit = reader_->process_hit();
    }
    two_step_finalize();
  }

  template <typename T = buffer_type>
  typename std::enable_if_t<std::is_same<T, std::ifstream>::value>
  run_temporal_split_clustering(const std::string &data_file)
  {
    reader_ = std::move(std::make_unique<reader_type<buffer_type>>(data_file));
    printer_ = std::move(std::make_unique<mm_printer_type>(
        new mm_write_stream(create_clustered_output_name(data_file))));

    auto new_hit = reader_->process_hit();

    while (!done())
    {
      sorter_->process_hit(adapter_->process_hit(new_hit));
      if (sorter_->result_hits().size() > MIN_BUFFER_SIZE)
      {
        temp_clusterer_->process_hits(sorter_->result_hits().begin(),
                                      sorter_->result_hits().end());
        sorter_->result_hits().clear();
      }

      if (temp_clusterer_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        cluster_splitter_->process_data(
            temp_clusterer_->result_clusters().begin(),
            temp_clusterer_->result_clusters().end());
        temp_clusterer_->result_clusters().clear();
      }

      if (cluster_splitter_->result_clusters().size() > MIN_BUFFER_SIZE)
      {
        printer_->process_data(cluster_splitter_->result_clusters().begin(),
                               cluster_splitter_->result_clusters().end());
        cluster_splitter_->result_clusters().clear();
      }
      new_hit = reader_->process_hit();
    }
    two_step_finalize();
  }
};
