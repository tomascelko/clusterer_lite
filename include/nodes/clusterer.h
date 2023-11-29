#pragma once
#include "../data_structs/cluster.h"
#include "../data_structs/mm_hit.h"
#include "../data_structs/node_args.h"
#include "../devices/current_device.h"
#include "../other/utils.h"
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

template <typename mm_hit> struct unfinished_cluster;
using cluster_list = std::list<unfinished_cluster<mm_hit>>;
using cluster_it = typename cluster_list::iterator;
using cluster_it_list = typename std::list<cluster_it>;

// an auxiliary structure for cluster that is open at a time
template <typename mm_hit> struct unfinished_cluster
{
  cluster<mm_hit> cl;
  // iterators pointing to pixels matrix entries (which are iterators of the
  // hits)
  std::vector<typename cluster_it_list::iterator> pixel_iterators;
  // self reference
  cluster_it self;
  bool selected = false;

  unfinished_cluster() {}

  void select() { selected = true; }

  void unselect() { selected = false; }
};

// a node which implements the pixels list clustering as proposed by P.Manek
class pixel_list_clusterer
{
private:
  std::vector<cluster_it_list> pixel_lists_;
  cluster_list unfinished_clusters_;
  uint32_t unfinished_clusters_count_;
  bool finished_ = false;
  uint64_t processed_hit_count_;
  double current_toa_;
  uint32_t tile_size_;
  const std::vector<coord> EIGHT_NEIGHBORS = {{-1, -1}, {-1, 0}, {-1, 1},
                                              {0, -1},  {0, 0},  {0, 1},
                                              {1, -1},  {1, 0},  {1, 1}};
  const uint32_t WRITE_INTERVAL = 2 << 2;
  using hit_vect_iterator = std::vector<mm_hit>::iterator;
  using optional_clusters =
      std::optional<std::vector<mm_hit, std::allocator<mm_hit>>>;
  uint64_t merge_count_ = 0;
  std::vector<cluster<mm_hit>> result_clusters_;
  uint64_t processed_clusters_ = 0;

protected:
  double cluster_diff_dt =
      200.; // time that marks the max difference of cluster last_toa()

  bool is_old(double last_toa, const cluster<mm_hit> &cl)
  {
    return cl.last_toa() < last_toa - cluster_diff_dt;
  }

  void merge_clusters(unfinished_cluster<mm_hit> &base_cluster,
                      unfinished_cluster<mm_hit> &new_cluster)
  // merging clusters to biggest cluster will however disrupt time orderedness -
  // after merging bigger cluster can lower its first toa which causes time
  // unorderedness, can hovewer improve performance try always merging to left
  // (smaller toa)
  {

    for (auto &pix_it : new_cluster.pixel_iterators) // update iterator
    {
      *pix_it = base_cluster.self;
    }

    base_cluster.cl.hits().reserve(base_cluster.cl.hits().size() +
                                   new_cluster.cl.hits().size());
    base_cluster.cl.hits().insert(
        base_cluster.cl.hits().end(),
        std::make_move_iterator(new_cluster.cl.hits().begin()),
        std::make_move_iterator(
            new_cluster.cl.hits()
                .end())); // TODO try hits in std list and then concat in O(1)

    // merge clusters
    base_cluster.pixel_iterators.reserve(base_cluster.pixel_iterators.size() +
                                         new_cluster.pixel_iterators.size());
    base_cluster.pixel_iterators.insert(
        base_cluster.pixel_iterators.end(),
        std::make_move_iterator(new_cluster.pixel_iterators.begin()),
        std::make_move_iterator(
            new_cluster.pixel_iterators
                .end())); // TODO try hits in std list and then concat in O(1)

    // update first and last toa
    base_cluster.cl.set_first_toa(
        std::min(base_cluster.cl.first_toa(), new_cluster.cl.first_toa()));
    base_cluster.cl.set_last_toa(
        std::max(base_cluster.cl.last_toa(), new_cluster.cl.last_toa()));

    unfinished_clusters_.erase(new_cluster.self);
    --unfinished_clusters_count_;
  }

  std::vector<cluster_it> find_neighboring_clusters(const coord &base_coord,
                                                    double toa,
                                                    cluster_it &oldest_cluster)
  {
    std::vector<cluster_it> uniq_neighbor_cluster_its;
    double min_toa = std::numeric_limits<double>::max();
    for (auto neighbor_offset : EIGHT_NEIGHBORS) // check all neighbor indexes
    {
      if (!base_coord.is_valid_neighbor(neighbor_offset, tile_size_))
        continue;
      // error in computing neighbor offset linearize() - does not account for
      // tile size
      uint32_t neighbor_index = neighbor_offset.linearize_tiled(tile_size_) +
                                base_coord.linearize(tile_size_);

      for (auto &neighbor_cl_it :
           pixel_lists_[neighbor_index]) // iterate over each cluster neighbor
                                         // pixel can be in
      // TODO do reverse iteration and break when finding a match - as there
      // cannot two "already mergable" clusters on a single neighbor pixel
      {
        if (std::abs(toa - neighbor_cl_it->cl.last_toa()) < cluster_diff_dt &&
            !neighbor_cl_it->selected)
        { // which of conditions is more likely to fail?
          neighbor_cl_it->select();
          uniq_neighbor_cluster_its.push_back(neighbor_cl_it);
          if (neighbor_cl_it->cl.first_toa() <
              min_toa) // find biggest cluster for possible merging
          {
            min_toa = neighbor_cl_it->cl.first_toa();
            oldest_cluster = neighbor_cl_it;
          }
          break;
        }
      }
    }
    for (auto &neighbor_cluster_it : uniq_neighbor_cluster_its)
    {
      neighbor_cluster_it
          ->unselect(); // unselect to prepare clusters for next neighbor search
    }

    return uniq_neighbor_cluster_its;
  }

public:
  std::string name() { return "clusterer"; }

  std::vector<cluster_it> get_all_unfinished_clusters()
  {
    std::vector<cluster_it> unfinished_its;
    for (auto it = unfinished_clusters_.begin();
         it != unfinished_clusters_.end(); ++it)
    {
      unfinished_its.push_back(it);
    }
    return unfinished_its;
  }

  void add_new_hit(mm_hit &&hit, cluster_it &cluster_iterator)
  {
    // update cluster itself, assumes the cluster exists
    auto &target_pixel_list =
        pixel_lists_[hit.coordinates().linearize(tile_size_)];
    target_pixel_list.push_front(cluster_iterator);
    cluster_iterator->pixel_iterators.push_back(
        target_pixel_list
            .begin()); // beware, we need to push in the same order,
                       // as we push hits in addhit and in merging
    cluster_iterator->cl.add_hit(std::move(hit));
    // update the pixel list
  }

  std::vector<cluster<mm_hit>>
  get_old_clusters(std::vector<cluster<mm_hit>> &old_clusters,
                   double hit_toa = 0)
  {
    // old clusters should be at the end of the list
    while (unfinished_clusters_count_ > 0 &&
           (is_old(hit_toa, unfinished_clusters_.back().cl) || finished_))
    {
      unfinished_cluster<mm_hit> &current = unfinished_clusters_.back();
      auto &current_hits = current.cl.hits();
      for (uint32_t i = 0; i < current.pixel_iterators.size();
           i++) // update iterator
      {
        auto &pixel_list_row =
            pixel_lists_[current_hits[i].coordinates().linearize(tile_size_)];
        pixel_list_row.erase(current.pixel_iterators[i]);
      }
      current_toa_ = unfinished_clusters_.back().cl.first_toa();
      old_clusters.emplace_back(std::move(unfinished_clusters_.back().cl));
      unfinished_clusters_.pop_back();
      --unfinished_clusters_count_;
    }
    return old_clusters;
  }

  void process_hit(mm_hit &&hit)
  {
    cluster_it target_cluster = unfinished_clusters_.end();
    const auto neighboring_clusters =
        find_neighboring_clusters(hit.coordinates(), hit.toa(), target_cluster);
    switch (neighboring_clusters.size())
    {
    case 0:
      // pixel does not belong to any cluster, create a new one
      unfinished_clusters_.emplace_front(unfinished_cluster<mm_hit>{});
      processed_clusters_++;
      unfinished_clusters_.begin()->self = unfinished_clusters_.begin();
      ++unfinished_clusters_count_;
      target_cluster = unfinished_clusters_.begin();
      break;
    case 1:
      // target cluster is already selected by find neighboring clusters
      break;
    default:
      // we found the largest cluster and move hits from smaller clusters to the
      // biggest one
      ++merge_count_;
      for (auto &neighboring_cluster_it : neighboring_clusters)
      {
        // merge clusters and then add pixel to it
        if (neighboring_cluster_it != target_cluster)
        {
          merge_clusters(*target_cluster, *neighboring_cluster_it);
          processed_clusters_--;
        }
      }
      break;
    }
    add_new_hit(std::move(hit), target_cluster);
    ++processed_hit_count_;
  }

  void process_hits(hit_vect_iterator first, hit_vect_iterator last)
  {
    std::vector<cluster<mm_hit>> old_clusters;
    for (auto hit_it = first; hit_it != last; ++hit_it)
    {
      double current_toa = hit_it->toa();
      process_hit(std::move(*hit_it));
      if (processed_hit_count_ % WRITE_INTERVAL == 0)
        get_old_clusters(old_clusters, current_toa);
    }
    result_clusters_.insert(result_clusters_.end(), old_clusters.begin(),
                            old_clusters.end());
  }

  std::vector<cluster<mm_hit>> process_remaining()
  {
    finished_ = true;
    std::vector<cluster<mm_hit>> old_clusters;
    get_old_clusters(old_clusters);
    std::cout << "Merge happened " << merge_count_ << " times" << std::endl;
    std::cout << "Total hits processed " << processed_hit_count_ << std::endl;
    std::cout << "Processed clusters " << processed_clusters_ << std::endl;
    return old_clusters;
  }

  pixel_list_clusterer(const node_args &args)
    : pixel_lists_(current_chip::chip_type::size_x() *
                   current_chip::chip_type::size_y() /
                   (args.get_arg<int>(name(), "tile_size") *
                    args.get_arg<int>(name(), "tile_size"))),
      tile_size_(args.get_arg<int>(name(), "tile_size")),
      unfinished_clusters_count_(0), processed_hit_count_(0), current_toa_(0),
      cluster_diff_dt(args.get_arg<double>(name(), "max_dt")),
      result_clusters_()
  {
    return;
  }

  std::vector<cluster<mm_hit>> &result_clusters() { return result_clusters_; }

  double current_toa() { return current_toa_; }

  void reset()
  {
    pixel_lists_.clear();
    unfinished_clusters_.clear();
    unfinished_clusters_count_ = 0;
    finished_ = false;
  }

  void set_tile(uint32_t tile_size)
  {

    tile_size_ = tile_size;
    reset();
    pixel_lists_ = std::vector<cluster_it_list>(
        current_chip::chip_type::size_x() * current_chip::chip_type::size_y() /
        (tile_size * tile_size));
  }

  void close() {}

  virtual ~pixel_list_clusterer() = default;
};