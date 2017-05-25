/*
 * hpcl_comp.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_H_
#define HPCL_COMP_H_

#include "gpu-sim.h"
#include "../cuda-sim/memory.h"

#include "hpcl_stat.h"

//added by kh(091916)
#include "../cuda-sim/hpcl_cuda_comp.h"
///

//added by kh(021817)
#include <map>
///

class hpcl_comp
{
public:
  hpcl_comp ();
  virtual
  ~hpcl_comp ();

private:
  const struct memory_config *m_memory_config;
  class memory_space *m_global_mem;

public:
  void create(const struct memory_config * memory_config, class memory_space * global_mem);
  void get_cache_data(void* mf, void* data, int index=-1);
  void print_cache_data(mem_fetch* mf);

  //added by kh(031716)
  unsigned get_mem_no() {
    return m_memory_config->m_n_mem;
  }
  unsigned get_sub_partition_no_per_memory_channel() {
    return m_memory_config->m_n_sub_partition_per_memory_channel;
  }
  ///

  //added by kh(050216)
  new_addr_type get_cache_block_addr(new_addr_type access_addr) {
    return m_memory_config->m_L2_config.block_addr(access_addr);
  }
  ///

  //added by kh(060616)
  void transform_cache_data_type0(void* mf, unsigned remap_res, unsigned type_index);

  //added by kh(070216)
  void transform_cache_data_type1(void* mf, unsigned remap_res, unsigned src_type_index, unsigned type_index);


  bool is_data_retrievable(mem_fetch* mf);

  //added by kh(071316)
  void remap_data_to_type0(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type0);
  void remap_data_to_type1(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type1, unsigned& added_bits_for_remap, std::vector<enum mem_fetch::REMAP_OP_TYPE>& remap_op_types, unsigned& approx_op, unsigned& approx_except);
  ///


  void remap_nonchar_data(void* data);

  //added by kh(071516)
  int compare_data(void* mf1, void* mf2);


  //added by kh(072616)
  int check_csn_pattern(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size);
  int complement_data(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size);
  int compute_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, int& out_max_delta_val, unsigned char& out_min_nib_val);
  int compute_masking(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& out_max_nib_val);
  int compute_neighbor_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& first_nib_val);

  int check_float_number(std::vector<unsigned char>& remapped_data_type, int group_size);

  //added by kh(091916)
private:
  hpcl_cuda_comp m_hpcl_cuda_comp;


  //added by kh(02817)
private:
  std::map<unsigned char, unsigned char> m_symbol_table;

public:
  //preprocessing takes place every single byte.
  //The high-nibbles for every byte are clustered,  while the low-nibbles are clustered.
  void remap_char_data(void *data);
  double get_char_type_pct(void *data);
  void convert_char_symbols(void *data);

  ///


};

#endif /* HPCL_COMP_H_ */
