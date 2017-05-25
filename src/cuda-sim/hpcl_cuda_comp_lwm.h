/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_CUDA_COMP_LWM_H_
#define HPCL_CUDA_COMP_LWM_H_

#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <set>

#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

#include "hpcl_cuda_user_define_stmts.h"
#include "hpcl_cuda_mem_fetch.h"
#include "hpcl_dict.h"

template<class K>
class hpcl_cuda_comp_lwm {
public:
  hpcl_cuda_comp_lwm();
  ~hpcl_cuda_comp_lwm() {};

public:
  void run(hpcl_cuda_mem_fetch* mf);

public:
  static void decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list);
  static void decompose_data(K word, std::set<unsigned char>& word_list);
  static void decompose_data(K word, std::vector<unsigned char>& word1B_list);
  static void print_word_list(std::vector<K>& word_list);

private:
  unsigned compute_data_ptr_bits(hpcl_cuda_mem_fetch* mf);
  unsigned run_lwm_algo(hpcl_cuda_mem_fetch* mf, unsigned data_type, std::vector<unsigned char>& cache_data, hpcl_dict<K>* loc_dict, std::vector<double>& comp_ds_bits, std::vector<unsigned>& comp_ds_match_word_no, std::vector<int>& no_comp_ds_ratio);
  void run_comp_for_remapped_data(hpcl_cuda_mem_fetch* mf, unsigned remap_type);

  int run_dynamic_dict_in_data_segment(hpcl_dict<K>* trans_loc_dict, unsigned data_ptr_bits, std::vector<unsigned>& matchword_no, std::vector<double>& comp_ds_bits);
  int run_part_comp_in_data_segment(std::vector<int>& trans_no_comp_ratio, std::vector<double>& comp_ds_bits);
  void run_comp_for_org_data(hpcl_cuda_mem_fetch* mf);

};

template <class K>
hpcl_cuda_comp_lwm<K>::hpcl_cuda_comp_lwm() {
}

template<class K>
unsigned hpcl_cuda_comp_lwm<K>::run_lwm_algo(hpcl_cuda_mem_fetch* mf, unsigned data_type, std::vector<unsigned char>& cache_data, hpcl_dict<K>* loc_dict, std::vector<double>& comp_ds_bits, std::vector<unsigned>& comp_ds_match_word_no, std::vector<int>& no_comp_ds_ratio)
{
  std::vector<K> trans_word_list;
  decompose_data(cache_data, trans_word_list);

  unsigned cache_data_size = cache_data.size();
  unsigned data_segment_size = g_hpcl_comp_config.hpcl_dsc_min_ds_size;

  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_MATCHING)	assert(data_segment_size >= sizeof(K));


  comp_ds_bits.resize(cache_data_size/data_segment_size, -1);
  no_comp_ds_ratio.resize(cache_data_size/data_segment_size, 1);
  comp_ds_match_word_no.resize(cache_data_size/data_segment_size, 0);

  unsigned matching_word_no = 0;
  unsigned data_ptr_bits = (unsigned) ceil(log2((double)cache_data_size/sizeof(K)));
  unsigned comp_bit_size = 0;

  //added by kh(070516)
  comp_bit_size = 1;	//1-bit indicates compressed or not.
  comp_bit_size += 3; 	//3-bit data remapping type
  comp_bit_size += 2;	//Resolution

  LWM_COMP_DEBUG_PRINT("run_lwm_algo : mf %u, data_type %u\n", mf->get_request_uid(), data_type);

  LWM_COMP_DEBUG_PRINT("\tStep1 - header : %u\n", comp_bit_size);
  for(unsigned i = 0; i < trans_word_list.size(); i++) {

      //added by kh(061016)
      bool is_word_compressed = false;

      comp_bit_size += 1;		//flag(1 bit)
      K word = trans_word_list[i];
      int word_index = loc_dict->search_word(word);
      if(word_index >= 0) {
	  comp_bit_size += data_ptr_bits;
	loc_dict->update_dict(word, 0);

	//added by kh(061016)
	//store the max last word index
	int trans_word_loc = loc_dict->get_word_loc(word_index);
	//assert(trans_word_loc >= 0);
	if(loc_dict->get_last_max_word_loc() < trans_word_loc) {
	    loc_dict->set_last_max_word_loc(trans_word_loc);
	}
	///

	//added by kh(061016)
	is_word_compressed = true;
	///

	//added by kh(061216)
	matching_word_no++;
	///

	LWM_COMP_DEBUG_PRINT("\tword %u is compressed!\n", i);

      } else {

	comp_bit_size += (sizeof(K)*8);
	//double dict_size = sizeof(K)*(i+1);
	//if(dict_size <= max_dict_size) {
	int index = loc_dict->update_dict(word, 0);
	loc_dict->set_word_loc(index, i);
	  /*
	  std::cout << "\t ";
	  trans_loc_dict->print_word(word);
	  std::cout << " trans_dict index " << index << " trans_word_loc " << i << std::endl;
	  */
	//}

	LWM_COMP_DEBUG_PRINT("\tword %u is not compressed!\n", i);
      }

      //added by kh(061016)
      unsigned cur_data_segment_size = (i+1)*sizeof(K);
      //added by kh(070416)
      if(cur_data_segment_size % data_segment_size == 0)
      {
	unsigned ds_index = (cur_data_segment_size/data_segment_size - 1);
	unsigned prev_ds_comp_bits = 0;
	for(unsigned j = 0; j < ds_index; j++) {
	  prev_ds_comp_bits += comp_ds_bits[j];
	}
	assert((comp_bit_size-prev_ds_comp_bits) > 0);
	comp_ds_bits[ds_index] = (comp_bit_size - prev_ds_comp_bits);
      }
      ///


      unsigned cur_data_segment_head = i*sizeof(K);
      if(is_word_compressed == true) {
	unsigned ds_index = cur_data_segment_head/data_segment_size;
	no_comp_ds_ratio[ds_index] = 0;
      }

      if(cur_data_segment_size % data_segment_size == 0)
      {
	unsigned ds_index = cur_data_segment_size/data_segment_size - 1;
	comp_ds_match_word_no[ds_index] = matching_word_no;
	matching_word_no = 0;
      }

  }
  LWM_COMP_DEBUG_PRINT("\tStep2 - header+comp_data : %u\n", comp_bit_size);


  LWM_COMP_DEBUG_PRINT("LWM: mf %u comp_bits %u\n", mf->get_request_uid(), comp_bit_size);
  return comp_bit_size;
}


template<class K>
void hpcl_cuda_comp_lwm<K>::run_comp_for_remapped_data(hpcl_cuda_mem_fetch* mf, unsigned remap_type)
{
  //init dict
  hpcl_dict<K>* loc_dict = NULL;
  loc_dict = (hpcl_dict<K>*) mf->get_trans_loc_dict(sizeof(K), remap_type);
  if(loc_dict == NULL) {
      loc_dict = new hpcl_dict<K>(128, HPCL_LFU);		//MAX word #: 128
    mf->set_trans_loc_dict(loc_dict, sizeof(K), remap_type);
  }
  loc_dict->clear();

  //support for compressing the transformed data
  std::vector<unsigned char>& cache_data = mf->get_trans_data_ptr(remap_type);
  std::vector<double> comp_ds_bits;
  std::vector<int> no_comp_ds_ratio;
  std::vector<unsigned> comp_ds_matching_word_no;

  unsigned comp_bit_size = run_lwm_algo(mf, remap_type+1, cache_data, loc_dict, comp_ds_bits, comp_ds_matching_word_no, no_comp_ds_ratio);
  unsigned data_ptr_bits = (unsigned) ceil(log2((double)cache_data.size()/sizeof(K)));
  //(hpcl_dict<K>* trans_loc_dict, unsigned data_ptr_bits, std::vector<unsigned>& matchword_no, std::vector<unsigned>& comp_bits)
  if(g_hpcl_comp_config.hpcl_dict_size_opt_en == 1) {
    int ret = run_dynamic_dict_in_data_segment(loc_dict, data_ptr_bits, comp_ds_matching_word_no, comp_ds_bits);
    if(ret > 0) {
      comp_bit_size = ret;
    }
  }

  if(g_hpcl_comp_config.hpcl_partial_comp_en == 1) {
    int ret = run_part_comp_in_data_segment(no_comp_ds_ratio, comp_ds_bits);
    if(ret > 0) {
      comp_bit_size = ret;
    }
  }

  mf->set_trans_comp_data_bits(comp_bit_size, sizeof(K), remap_type);

  LWM_COMP_DEBUG_PRINT("LWM: mf %u data_type %u res %u comp_bit_size %u\n", mf->get_request_uid(), remap_type+1, sizeof(K), comp_bit_size);

}

template<class K>
void hpcl_cuda_comp_lwm<K>::run_comp_for_org_data(hpcl_cuda_mem_fetch* mf)
{
  //init dict
  hpcl_dict<K>* loc_dict = NULL;
  loc_dict = (hpcl_dict<K>*) mf->get_loc_dict(sizeof(K));
  if(loc_dict == NULL) {
    loc_dict = new hpcl_dict<K>(128, HPCL_LFU);		//MAX word #: 128
    mf->set_loc_dict(sizeof(K), loc_dict);
  }
  loc_dict->clear();

  //support for compressing the transformed data
  std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
  std::vector<double> comp_ds_bits;
  std::vector<int> no_comp_ds_ratio;
  std::vector<unsigned> comp_ds_matching_word_no;

  //printf("mf %u starts lwm_comp\n", mf->get_request_uid());
  unsigned comp_bit_size = run_lwm_algo(mf, 0, cache_data, loc_dict, comp_ds_bits, comp_ds_matching_word_no, no_comp_ds_ratio);
  unsigned data_ptr_bits = (unsigned) ceil(log2((double)cache_data.size()/sizeof(K)));
  //(hpcl_dict<K>* trans_loc_dict, unsigned data_ptr_bits, std::vector<unsigned>& matchword_no, std::vector<unsigned>& comp_bits)
  if(g_hpcl_comp_config.hpcl_dict_size_opt_en == 1) {
    int ret = run_dynamic_dict_in_data_segment(loc_dict, data_ptr_bits, comp_ds_matching_word_no, comp_ds_bits);
    if(ret > 0) {
      comp_bit_size = ret;
    }
  }

  if(g_hpcl_comp_config.hpcl_partial_comp_en == 1) {
    int ret = run_part_comp_in_data_segment(no_comp_ds_ratio, comp_ds_bits);
    if(ret > 0) {
      comp_bit_size = ret;
    }
  }

  mf->set_comp_data_bits(comp_bit_size, sizeof(K));

  LWM_COMP_DEBUG_PRINT("LWM: mf %u data_type %u res %u comp_bit_size %u\n", mf->get_request_uid(), 0, sizeof(K), comp_bit_size);
}


template<class K>
void hpcl_cuda_comp_lwm<K>::run(hpcl_cuda_mem_fetch* mf)
{
  //If mf is not type for compression,
  if(mf->get_real_data_size() == 0)	return;

  //Compression for org data
  run_comp_for_org_data(mf);

  //Data Remapping Support
  if(g_hpcl_comp_config.hpcl_comp_algo != hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL) {
    for(int i = 0; i < mf->get_trans_data_no(); i++) {
      run_comp_for_remapped_data(mf, i);
    }
  }
}

template <class K>
void hpcl_cuda_comp_lwm<K>::decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list)
{
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    //printf("decomposed %u\n", i);
    for(int j = sizeof(K)-1; j >= 0; j--) {
      K tmp = cache_data[i+j];
      tmp = (tmp << (8*j));
      word_candi += tmp;
      //printf("\t%x %x %x\n", cache_data[i+j], tmp, word_candi);
    }
    word_list.push_back(word_candi);
    //printf("\tword -- ");
    //hpcl_dict_elem<K>::print_word_data(word_candi);
    //printf("\n");
  }

  //printf("word_list_size %u, test %u\n",word_list.size(), cache_data.size()/sizeof(K));

  assert(word_list.size() == cache_data.size()/sizeof(K));
}

template <class K>
void hpcl_cuda_comp_lwm<K>::decompose_data(K word, std::set<unsigned char>& word1B_list)
{
  //std::cout << "decompose_data ----" << std::endl;
  //hpcl_dict_elem<K>::print_word_data(word);
  for(int j = 0; j < sizeof(K); j++) {
    unsigned char word1B = (word >> (8*j)) & 0xff;
    word1B_list.insert(word1B);
    //printf("\tword1B = %02x\n", word1B);
  }
}

template <class K>
void hpcl_cuda_comp_lwm<K>::decompose_data(K word, std::vector<unsigned char>& word1B_list)
{
  //std::cout << "decompose_data ----" << std::endl;
  //hpcl_dict_elem<K>::print_word_data(word);
  for(int j = 0; j < sizeof(K); j++) {
    unsigned char word1B = (word >> (8*j)) & 0xff;
    word1B_list.push_back(word1B);
    //printf("\tword1B = %02x\n", word1B);
  }
}


template <class K>
void hpcl_cuda_comp_lwm<K>::print_word_list(std::vector<K>& word_list)
{
  for(unsigned i = 0; i < word_list.size(); i++) {
    printf("\tword%u -- ", i);
    hpcl_dict_elem<K>::print_word_data(word_list[i]);
    printf("\n");
  }
}

#endif /* HPCL_COMP_PL_PROC_H_ */
