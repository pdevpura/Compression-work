/*
 * hpcl_comp.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_CUDA_COMP_H_
#define HPCL_CUDA_COMP_H_

/*
#include "gpu-sim.h"
#include "../cuda-sim/memory.h"
#include "hpcl_stat.h"
*/

#include <vector>
#include "hpcl_cuda_mem_fetch.h"

class hpcl_cuda_comp
{
public:
  hpcl_cuda_comp ();
  virtual
  ~hpcl_cuda_comp ();

  void remap_data_to_type0(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type0);
  void remap_data_to_type1(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type1, unsigned& added_bits_for_remap, unsigned& approx_op);
  void remap_data(void* data);
  int compare_data(void* mf1, void* mf2);

  void approximate_data(std::vector<unsigned char>& remapped_data, hpcl_cuda_mem_fetch::COMP_DATA_TYPE type);


  int check_csn_pattern(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size);
  int complement_data(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size);
  int compute_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, int& out_max_delta_val, unsigned char& out_min_nib_val);
  int compute_masking(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& out_max_nib_val);
  int compute_neighbor_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& first_nib_val);
  int check_float_number(std::vector<unsigned char>& remapped_data_type, int group_size);

};

#endif /* HPCL_COMP_H_ */
