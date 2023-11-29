#include "data_structs/cluster.h"
#include "data_structs/mm_hit.h"
#include "devices/current_device.h"
#include "other/utils.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <stack>
#include <sys/types.h>

class cluster_splitter
{
  using cluster_it = std::vector<cluster<mm_hit>>::iterator;

  struct partitioned_hit
  {
    double toa;
    coord coordinates;
    uint32_t partition_index;

    ushort x() { return coordinates.x(); }

    ushort y() { return coordinates.y(); }

    partitioned_hit(double toa, const coord &coordinates)
      : toa(toa), partition_index(0), coordinates(coordinates)
    {
    }
  };

  struct partition_time_pair
  {
    uint32_t partition_index;
    double toa;

    partition_time_pair(double toa, uint32_t partition_index)
      : toa(toa), partition_index(partition_index)
    {
    }
  };

  class pixel_matrix

  {
    std::array<std::array<std::vector<partitioned_hit>,
                          current_chip::chip_type::size_x() + 2>,
               current_chip::chip_type::size_y() + 2>
        pixel_matrix_;

  public:
    std::vector<partitioned_hit> &at(uint32_t i, uint32_t j)
    {
      return pixel_matrix_[i + 1][j + 1];
    }
  };

  using timestamp_it = std::vector<partitioned_hit>::iterator;

  const std::vector<coord> EIGHT_NEIGHBORS = {{-1, -1}, {-1, 0}, {-1, 1},
                                              {0, -1},  {0, 0},  {0, 1},
                                              {1, -1},  {1, 0},  {1, 1}};
  const double MAX_JOIN_TIME = 200.;
  pixel_matrix pixel_matrix_;
  std::vector<timestamp_it> timestamp_references_;
  std::vector<cluster<mm_hit>> result_clusters_;
  u_int64_t clusters_procesed_ = 0;

  // std::vector<partition_time_pair> partition_time_pairs_;
  std::vector<cluster<mm_hit>> temp_clusters_;

  void store_to_matrix(const cluster<mm_hit> &cluster)
  {
    const uint64_t MAX_SAME_HIT_COUNT = 100;
    for (const auto &hit : cluster.hits())
    {
      pixel_matrix_.at(hit.x(), hit.y())
          .reserve(std::min(MAX_SAME_HIT_COUNT, cluster.hits().size()));
      pixel_matrix_.at(hit.x(), hit.y())
          .emplace_back(hit.toa(), hit.coordinates());
      timestamp_references_.push_back(pixel_matrix_.at(hit.x(), hit.y()).end() -
                                      1);
    }
  }

  void remove_from_matrix()
  {
    for (const auto &cluster : temp_clusters_)
    {
      for (const auto &hit : cluster.hits())
        pixel_matrix_.at(hit.x(), hit.y()).clear();
    }
    timestamp_references_.clear();
    result_clusters_.insert(result_clusters_.end(), temp_clusters_.begin(),
                            temp_clusters_.end());
    temp_clusters_.clear();
  }

  void dfs_tag(timestamp_it node, uint32_t current_partition_index)
  {
    std::stack<timestamp_it> open_nodes;
    open_nodes.push(node);

    node->partition_index = current_partition_index;
    while (!open_nodes.empty())
    {
      auto current_node = open_nodes.top();
      open_nodes.pop();

      for (const auto &neighbor_offset : EIGHT_NEIGHBORS)
      {
        auto &neighbor_data_vector = pixel_matrix_.at(
            current_node->coordinates.x() + neighbor_offset.x(),
            current_node->coordinates.y() + neighbor_offset.y());
        if (neighbor_data_vector.size() > 0)
        {
          for (timestamp_it neighbor_it = neighbor_data_vector.begin();
               neighbor_it != neighbor_data_vector.end(); ++neighbor_it)
          {
            if (neighbor_it->partition_index == 0 &&
                std::abs(neighbor_it->toa - current_node->toa) < MAX_JOIN_TIME)
            {
              open_nodes.push(neighbor_it);
              neighbor_it->partition_index = current_partition_index;
            }
          }
        }
      }
    }
  }

  void label_components(const cluster<mm_hit> &cluster)
  {
    uint32_t current_partition_index = 1;
    for (uint32_t i = 0; i < timestamp_references_.size(); ++i)
    {
      if (timestamp_references_[i]->partition_index == 0)
      {
        dfs_tag(timestamp_references_[i], current_partition_index);
        ++current_partition_index;
      }
    }
  }

  void split_cluster(cluster<mm_hit> &&cluster)
  {
    auto buffered_cluster_count = result_clusters_.size();
    /*for (uint32_t i = 0; i < timestamp_references_.size(); ++i)
    {
      while (partition_time_pairs_.size() <=
             timestamp_references_[i]->partition_index)
        partition_time_pairs_.emplace_back(partition_time_pairs_.size() + 1,
                                           std::numeric_limits<double>::max());
      if (partition_time_pairs_[timestamp_references_[i]->partition_index].toa >
          timestamp_references_[i]->toa)
        partition_time_pairs_[timestamp_references_[i]->partition_index].toa =
            timestamp_references_[i]->toa;
    }*/
    for (uint32_t i = 0; i < timestamp_references_.size(); ++i)
    {
      auto current_cluster_index =
          timestamp_references_[i]->partition_index - 1;
      while (current_cluster_index >= temp_clusters_.size())
        temp_clusters_.emplace_back();
      temp_clusters_[current_cluster_index].add_hit(
          std::move(cluster.hits()[i]));
    }
    if (temp_clusters_.size() > 1)
      std::sort(temp_clusters_.begin(), temp_clusters_.end(),
                [](const auto &left, const auto &right)
                { return left.first_toa() < right.first_toa(); });
    clusters_procesed_ += temp_clusters_.size();
  }

public:
  void process_cluster(cluster<mm_hit> &&cluster)
  {
    if (cluster.hits().size() == 1)
    {
      result_clusters_.push_back(cluster);
      ++clusters_procesed_;
    }
    else
    {
      store_to_matrix(cluster);
      label_components(cluster);
      split_cluster(std::move(cluster));
      remove_from_matrix();
    }
  }

  void process_data(cluster_it first, cluster_it last)
  {
    for (auto it = first; it != last; ++it)
    {
      process_cluster(std::move(*it));
    }
  }

  cluster_splitter() : result_clusters_(){};

  std::vector<cluster<mm_hit>> process_remaining() { return result_clusters_; }

  std::vector<cluster<mm_hit>> &result_clusters() { return result_clusters_; }
};