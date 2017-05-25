/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_LWM_PL_PROC_H_
#define HPCL_COMP_LWM_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_dict.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>


//#define DUMMY_COMP_TEST 0

//#define DEBUG_LWM 1

//added by kh(030816)
#include "hpcl_comp_anal.h"
extern hpcl_comp_anal* g_hpcl_comp_anal;
///

#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;


//added by kh(041516)
#include "hpcl_user_define_stmts.h"
///

template<class K>
class hpcl_comp_lwm_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  int m_pl_index;

public:
  hpcl_comp_lwm_pl_proc();
  ~hpcl_comp_lwm_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    if(m_loc_dict)	delete m_loc_dict;
  };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  hpcl_dict<K>* m_loc_dict;		//1B word
  //unsigned simul_local_comp(hpcl_dict<K>& loc_dict, vector<unsigned char>& cache_data);
public:
  void run();

//added by kh(042216)
public:
  static void decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list);
  static void decompose_data(K word, std::set<unsigned char>& word_list);
  static void decompose_data(K word, std::vector<unsigned char>& word1B_list);
  static void print_word_list(std::vector<K>& word_list);

#ifdef WORD1B_ANALYSIS
private:
  hpcl_dict<unsigned char>* m_loc_dict_for_uncomp_data;

  double m_word1B_zeros_rate_in_uncomp_data;
  double m_word1B_nonzeros_rate_in_uncomp_data;
  double m_word1B_uni_val_rate_in_uncomp_data;
  double m_word1B_nonzeros_no;

public:
  void get_word1B_redund_stat_in_uncomp_data(double& word1B_zeros_rate, double& word1B_nonzeros_rate, double& word1B_uni_val_rate, double& word1B_nonzeros_no)
  {
    word1B_zeros_rate = m_word1B_zeros_rate_in_uncomp_data;
    word1B_nonzeros_rate = m_word1B_nonzeros_rate_in_uncomp_data;
    word1B_uni_val_rate = m_word1B_uni_val_rate_in_uncomp_data;
    word1B_nonzeros_no = m_word1B_nonzeros_no;
  }
#endif
///

//added by kh(070216)
private:
  unsigned compute_data_ptr_bits(mem_fetch* mf);
  //void init_comp_for_trans_data(mem_fetch* mf);
  unsigned run_lwm_algo(mem_fetch* mf, unsigned data_type, std::vector<unsigned char>& cache_data, hpcl_dict<K>* loc_dict, std::vector<double>& comp_ds_bits, std::vector<unsigned>& comp_ds_match_word_no, std::vector<int>& no_comp_ds_ratio, unsigned& compressed_bits_only);
  void run_comp_for_remapped_data(mem_fetch* mf, unsigned remap_type);

  int run_dynamic_dict_in_data_segment(hpcl_dict<K>* trans_loc_dict, unsigned data_ptr_bits, std::vector<unsigned>& matchword_no, std::vector<double>& comp_ds_bits);
  int run_part_comp_in_data_segment(std::vector<int>& trans_no_comp_ratio, std::vector<double>& comp_ds_bits);
  void run_comp_for_org_data(mem_fetch* mf);


//added by kh(073016)
private:
  int m_type;
public:
  void set_pl_type(int type);
  int get_pl_type();
  enum
  {
    NONE=0,
    DUMMY,
    COMP,
    GET_OUTPUT,
  };
///







};

template <class K>
hpcl_comp_lwm_pl_proc<K>::hpcl_comp_lwm_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_loc_dict = new hpcl_dict<K>(128, HPCL_LFU);
  m_loc_dict->clear();

  //added by kh(073016)
  m_type = NONE;
  ///
#ifdef WORD1B_ANALYSIS
  //added by kh(042216)
  m_loc_dict_for_uncomp_data = new hpcl_dict<unsigned char>(128, HPCL_LFU);
  m_loc_dict_for_uncomp_data->clear();
  ///
#endif
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_lwm_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_lwm_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_lwm_pl_proc<K>::get_input() {
  return m_input;
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::reset_output() {
  m_output->clean();
}

/*
template<class K>
unsigned hpcl_comp_lwm_pl_proc<K>::run_data_segment_comp(std::vector<unsigned char>& data, std::vector<unsigned>& ds_enc_status)
{
}
*/




template<class K>
int hpcl_comp_lwm_pl_proc<K>::run_dynamic_dict_in_data_segment(hpcl_dict<K>* trans_loc_dict, unsigned data_ptr_bits, std::vector<unsigned>& matchword_no, std::vector<double>& comp_bits)
{
  int ret_comp_bits = -1;

  int trans_last_max_word_index = trans_loc_dict->get_last_max_word_loc();
  if(trans_last_max_word_index >= 0) {

    unsigned new_data_ptr_bits = (unsigned) ceil(log2(trans_last_max_word_index+1));
    if(new_data_ptr_bits == 0)	new_data_ptr_bits = 1;
    assert(new_data_ptr_bits <= data_ptr_bits);
    //printf("trans_data_ptr_bits : %u, trans_new_data_ptr_bits : %u\n", data_ptr_bits, new_data_ptr_bits);
    unsigned all_matching_no = 0;
    unsigned new_comp_bit_size = 0;
    for(unsigned i = 0; i < matchword_no.size(); i++) {
	comp_bits[i] = comp_bits[i] - (data_ptr_bits-new_data_ptr_bits)*matchword_no[i];
	new_comp_bit_size += comp_bits[i];
	all_matching_no += matchword_no[i];
    }
    unsigned trans_comp_header = 3;	//comp_header : 3 bits for index bit size: 000 --> 1bit, 101 --> 6bit

    //printf("DICT_SIZE_ADJ | mf %u res %u trans_comp_bit_size : %u, trans_new_comp_bit_size : %u, matching_word_no : %u\n", mf->get_request_uid(), sizeof(K), trans_comp_bit_size, (new_comp_bit_size+trans_comp_header), all_matching_no);
    ret_comp_bits = (new_comp_bit_size+trans_comp_header);
  }

  return ret_comp_bits;
}


template<class K>
int hpcl_comp_lwm_pl_proc<K>::run_part_comp_in_data_segment(std::vector<int>& trans_no_comp_ratio, std::vector<double>& comp_ds_bits)
{
  int ret_comp_bits = -1;

  //added by kh(061216)
  //Partial Compression
  //if(g_hpcl_comp_config.hpcl_partial_comp_en == 1) {
    unsigned trans_comp_bit_size_by_partial_comp = 0;
    unsigned trans_no_comp_ds_no = 0;
    for(unsigned i = 0; i < trans_no_comp_ratio.size(); i++) {
      if(trans_no_comp_ratio[i] == 1) {
	comp_ds_bits[i] = comp_ds_bits[i] - 32/sizeof(K);
  	trans_no_comp_ds_no++;
      }
      trans_comp_bit_size_by_partial_comp += comp_ds_bits[i];
    }
    unsigned trans_comp_header = 4;	//comp_header : 4 bits each bit status of compression of each ds

    //printf("PART_COMP | mf %u res %u trans_comp_bit_size %u, trans_comp_bit_size_by_partial_comp %u, trans_no_comp_ds_no %u\n", mf->get_request_uid(), sizeof(K), trans_comp_bit_size, (trans_comp_bit_size_by_partial_comp+trans_comp_header), trans_no_comp_ds_no);
    ret_comp_bits = (trans_comp_bit_size_by_partial_comp+trans_comp_header);
  //}
}

template<class K>
unsigned hpcl_comp_lwm_pl_proc<K>::run_lwm_algo(mem_fetch* mf, unsigned data_type, std::vector<unsigned char>& cache_data, hpcl_dict<K>* loc_dict, std::vector<double>& comp_ds_bits, std::vector<unsigned>& comp_ds_match_word_no, std::vector<int>& no_comp_ds_ratio, unsigned& comp_bits_only)
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
  //added by kh(090116)
  comp_bits_only = 0;

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
	//added by kh(090116)
	comp_bits_only += data_ptr_bits;
	///

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
	//added by kh(090116)
	comp_bits_only += (sizeof(K)*8);
	///

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
      /*
      if(cur_data_segment_size == 32)		comp_ds_bits[0] = comp_bit_size;
      else if(cur_data_segment_size == 64) 	comp_ds_bits[1] = comp_bit_size-comp_ds_bits[0];
      else if(cur_data_segment_size == 96)	comp_ds_bits[2] = comp_bit_size-comp_ds_bits[0]-comp_ds_bits[1];
      else if(cur_data_segment_size == 128)	comp_ds_bits[3] = comp_bit_size-comp_ds_bits[0]-comp_ds_bits[1]-comp_ds_bits[2];
      */
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
      /*
      if(cur_data_segment_head/data_segment_size == 0) {
	if(is_word_compressed == true)	no_comp_ds_ratio[0] = 0;
      } else if(cur_data_segment_head/data_segment_size == 1) {
	if(is_word_compressed == true)	no_comp_ds_ratio[1] = 0;
      } else if(cur_data_segment_head/data_segment_size == 2) {
	if(is_word_compressed == true)	no_comp_ds_ratio[2] = 0;
      } else if(cur_data_segment_head/data_segment_size == 3) {
	if(is_word_compressed == true)	no_comp_ds_ratio[3] = 0;
      }
      */
      if(is_word_compressed == true) {
	unsigned ds_index = cur_data_segment_head/data_segment_size;
	no_comp_ds_ratio[ds_index] = 0;
      }


      ///

      //added by kh(061216)
      //save number of matching words for each data segment
      /*
      if(cur_data_segment_size == 32) {
	comp_ds_match_word_no[0] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 64) {
	comp_ds_match_word_no[1] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 96) {
	comp_ds_match_word_no[2] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 128) {
	comp_ds_match_word_no[3] = matching_word_no;
	matching_word_no = 0;
      }
      */
      //

      if(cur_data_segment_size % data_segment_size == 0)
      {
	unsigned ds_index = cur_data_segment_size/data_segment_size - 1;
	comp_ds_match_word_no[ds_index] = matching_word_no;
	matching_word_no = 0;
      }

  }
  LWM_COMP_DEBUG_PRINT("\tStep2 - header+comp_data : %u\n", comp_bit_size);


  LWM_COMP_DEBUG_PRINT("LWM: mf %u comp_bits %u comp_bits_only %u\n", mf->get_request_uid(), comp_bit_size, comp_bits_only);
  return comp_bit_size;
}


template<class K>
void hpcl_comp_lwm_pl_proc<K>::run_comp_for_remapped_data(mem_fetch* mf, unsigned remap_type)
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

  unsigned comp_data_bits_only = 0;
  unsigned comp_bit_size = run_lwm_algo(mf, remap_type+1, cache_data, loc_dict, comp_ds_bits, comp_ds_matching_word_no, no_comp_ds_ratio, comp_data_bits_only);
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

  //added by kh(061016)
  /*
  for(unsigned i = 0; i < cache_data.size()/32; i++) {
    loc_dict->set_comp_data_segment_ratio(i, (32*8-comp_ds_bits[i])/(32*8));
    loc_dict->set_no_comp_data_segment_ratio(i, no_comp_ds_ratio[i]);
  }
  */

  mf->set_trans_comp_data_bits(comp_bit_size, sizeof(K), remap_type);

#ifdef old_hyrid
  //added by kh(070416)
  if(remap_type == 0) 	   mf->set_comp_ds_bits(comp_ds_bits, sizeof(K), mem_fetch::REMAPPED_DATA_1);
  else if(remap_type == 1) mf->set_comp_ds_bits(comp_ds_bits, sizeof(K), mem_fetch::REMAPPED_DATA_2);
  else if(remap_type == 2) mf->set_comp_ds_bits(comp_ds_bits, sizeof(K), mem_fetch::REMAPPED_DATA_3);
  else if(remap_type == 3) mf->set_comp_ds_bits(comp_ds_bits, sizeof(K), mem_fetch::REMAPPED_DATA_4);
  else	assert(0);
  ///
#endif

  LWM_COMP_DEBUG_PRINT("LWM: mf %u data_type %u res %u comp_bit_size %u comp_bits_only %u\n", mf->get_request_uid(), remap_type+1, sizeof(K), comp_bit_size, comp_data_bits_only);

  //added by kh(090116)
  /*
  unsigned ed_bits = comp_data_bits_only;
  unsigned es_bits = comp_bit_size - comp_data_bits_only;
  mf->set_lwm_comp_ed_byte(ceil(ed_bits/8.0), sizeof(K));
  mf->set_lwm_comp_es_byte(ceil(es_bits/8.0), sizeof(K));
  */
  ///


}

template<class K>
void hpcl_comp_lwm_pl_proc<K>::run_comp_for_org_data(mem_fetch* mf)
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

  unsigned comp_data_bits_only = 0;
  //printf("mf %u starts lwm_comp\n", mf->get_request_uid());
  unsigned comp_bit_size = run_lwm_algo(mf, 0, cache_data, loc_dict, comp_ds_bits, comp_ds_matching_word_no, no_comp_ds_ratio, comp_data_bits_only);
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

  //added by kh(061016)
  //temporarily deleted
  /*
  for(unsigned i = 0; i < cache_data.size()/32; i++) {
    loc_dict->set_comp_data_segment_ratio(i, (32*8-comp_ds_bits[i])/(32*8));
    loc_dict->set_no_comp_data_segment_ratio(i, no_comp_ds_ratio[i]);
  }
  */

  mf->set_comp_data_bits(comp_bit_size, sizeof(K));

  //added by kh(070416)
  mf->set_comp_ds_bits(comp_ds_bits, sizeof(K), mem_fetch::ORG_DATA);

  LWM_COMP_DEBUG_PRINT("LWM: mf %u data_type %u res %u comp_bit_size %u comp_bits_only %u\n", mf->get_request_uid(), 0, sizeof(K), comp_bit_size, comp_data_bits_only);

  //added by kh(090116)
  unsigned ed_bits = comp_data_bits_only;
  unsigned es_bits = comp_bit_size - comp_data_bits_only;
  mf->set_lwm_comp_ed_byte(ceil(ed_bits/8.0), sizeof(K));
  mf->set_lwm_comp_es_byte(ceil(es_bits/8.0), sizeof(K));
  ///

}


template<class K>
void hpcl_comp_lwm_pl_proc<K>::run()
{
  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

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

  /*
  run_comp_for_remapped_data(mf, 0);
  run_comp_for_remapped_data(mf, 1);
  run_comp_for_remapped_data(mf, 2);
  run_comp_for_remapped_data(mf, 3);
  */

}


#ifdef delete_code
template<class K>
void hpcl_comp_lwm_pl_proc<K>::run()
{
  #ifdef DUMMY_COMP_TEST
  m_output->set_mem_fetch(m_input->get_mem_fetch());
  #else

  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

  //If mf is not type for compression,
  if(mf->get_real_data_size() == 0)	return;



  std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
  std::vector<K> word_list;
  decompose_data(cache_data, word_list);
#ifdef old
  //added by kh(060616)
  //support for compressing the transformed data
  std::vector<unsigned char>& trans_cache_data = mf->get_trans_data_ptr();
  std::vector<K> trans_word_list;
  //if(sizeof(K) == 2) {
  decompose_data(trans_cache_data, trans_word_list);
  //}
  ///
#endif

  //LWM simulation
  //encodeing format: encoding flag(1 bit), encoded word(index_size or word_size)
  unsigned comp_bit_size = 0;
  //deleted by kh(060916)
  //unsigned data_ptr_bits = (unsigned) ceil(log2(cache_data.size()/sizeof(K)));

  #ifdef DEBUG_LWM
  printf("org_data = 0x");
  for(unsigned i = 0; i < cache_data.size(); i++) {
    printf("%02x", cache_data[i]);
  }
  printf("\n");
  #endif

  //m_loc_dict;
  hpcl_dict<K>* loc_dict = (hpcl_dict<K>*) mf->get_loc_dict(sizeof(K));
  if(loc_dict == NULL) {
    loc_dict = new hpcl_dict<K>(128, HPCL_LFU);		//MAX word #: 128
    mf->set_loc_dict(sizeof(K), loc_dict);
  }
  loc_dict->clear();

#ifdef old
  //added by kh(060616)
  hpcl_dict<K>* trans_loc_dict = NULL;
  //if(sizeof(K) == 2) {
  trans_loc_dict = (hpcl_dict<K>*) mf->get_trans_loc_dict(sizeof(K));
  if(trans_loc_dict == NULL) {
    trans_loc_dict = new hpcl_dict<K>(128, HPCL_LFU);		//MAX word #: 128
    mf->set_trans_loc_dict(trans_loc_dict, sizeof(K));
  }
  trans_loc_dict->clear();
  //}
  ///
#endif

  #ifdef WORD1B_ANALYSIS
  std::vector<std::vector<unsigned char> > uncompdata;
  #endif

  //added by kh(060916)
  double max_dict_size = (double) mf->get_real_data_size();
#ifdef old
  if(sizeof(K) == 2) {
    if(g_hpcl_comp_config.hpcl_dict_window_mode_2B == hpcl_comp_config::DICT_WINDOW_100) {
      //do nothing
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_2B == hpcl_comp_config::DICT_WINDOW_50) {
      max_dict_size = max_dict_size*0.5;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_2B == hpcl_comp_config::DICT_WINDOW_25) {
      max_dict_size = max_dict_size*0.25;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_2B == hpcl_comp_config::DICT_WINDOW_12_5) {
      max_dict_size = max_dict_size*0.125;
    }
  } else if(sizeof(K) == 4) {
    if(g_hpcl_comp_config.hpcl_dict_window_mode_4B == hpcl_comp_config::DICT_WINDOW_100) {
      //do nothing
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_4B == hpcl_comp_config::DICT_WINDOW_50) {
      max_dict_size = max_dict_size*0.5;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_4B == hpcl_comp_config::DICT_WINDOW_25) {
      max_dict_size = max_dict_size*0.25;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_4B == hpcl_comp_config::DICT_WINDOW_12_5) {
      max_dict_size = max_dict_size*0.125;
    }
  } else if(sizeof(K) == 8) {
    if(g_hpcl_comp_config.hpcl_dict_window_mode_8B == hpcl_comp_config::DICT_WINDOW_100) {
      //do nothing
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_8B == hpcl_comp_config::DICT_WINDOW_50) {
      max_dict_size = max_dict_size*0.5;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_8B == hpcl_comp_config::DICT_WINDOW_25) {
      max_dict_size = max_dict_size*0.25;
    } else if(g_hpcl_comp_config.hpcl_dict_window_mode_8B == hpcl_comp_config::DICT_WINDOW_12_5) {
      max_dict_size = max_dict_size*0.125;
    }
  } else assert(0);
#endif

  unsigned data_ptr_bits = (unsigned) ceil(log2(max_dict_size/sizeof(K)));
  ///

  //added by kh(061016)
  std::vector<double> comp_data_segment_bits;
  comp_data_segment_bits.resize(cache_data.size()/32, -1);
  std::vector<int> no_comp_data_segment_ratio;
  no_comp_data_segment_ratio.resize(cache_data.size()/32, 1);
  ///

  //std::cout << "cache_data.size() " << cache_data.size() << " data_ptr_bits " << data_ptr_bits << std::endl;
  //added by kh(061216)
  std::vector<unsigned> comp_data_segment_matching_word_no;
  comp_data_segment_matching_word_no.resize(cache_data.size()/32, 0);
  unsigned matching_word_no = 0;
  ///

  for(unsigned i = 0; i < word_list.size(); i++) {

      bool is_word_compressed = false;

      //added by kh(051916)
      comp_bit_size += 1;		//flag(1 bit)
      ///

      K word = word_list[i];

      int word_index = loc_dict->search_word(word);
      if(word_index >= 0) {
	comp_bit_size += data_ptr_bits;

	#ifdef DEBUG_LWM
	std::cout << "\t";
	m_loc_dict->print_word(word);
	std::cout << " is found!" << std::endl;
	#endif

	//added by kh(042116)
	loc_dict->update_dict(word, 0);
	///

	#ifdef COMP_WORD_ANALYSIS
	//added by kh(042216)
	mf->push_compword(sizeof(K), word, mem_fetch::COMPRESSED);
	///
	#endif

	//added by kh(061016)
	//store the max last word index
	int word_loc = loc_dict->get_word_loc(word_index);
	//assert(word_loc >= 0);
	if(loc_dict->get_last_max_word_loc() < word_loc) {
	  loc_dict->set_last_max_word_loc(word_loc);
	}
	///

	//added by kh(061016)
	is_word_compressed = true;
	///

	//added by kh(061216)
	matching_word_no++;
	///

	/*
	std::cout << "\t";
	loc_dict->print_word(word);
	std::cout << " is found! : word_loc " << word_loc << std::endl;
	*/

      } else {
	comp_bit_size += (sizeof(K)*8);

	//deleted by kh(060916)
	//loc_dict->update_dict(word, 0);

	//added by kh(060916)
	//Update words within the dictionary window to the dictionary only
	double dict_size = sizeof(K)*(i+1);
	if(dict_size <= max_dict_size) {
	  int index = loc_dict->update_dict(word, 0);
	  loc_dict->set_word_loc(index, i);

	  /*
	  std::cout << "\t";
	  loc_dict->print_word(word);
	  std::cout << "dict index " << index << " word_loc " << i << std::endl;
	  */
	}
	///

	#ifdef DEBUG_LWM
	std::cout << "\t";
	m_loc_dict->print_word(word);
	std::cout << " is not found!" << std::endl;
	#endif

	#ifdef COMP_WORD_ANALYSIS
	//added by kh(042216)
	mf->push_compword(sizeof(K), word, mem_fetch::UNCOMPRESSED);
	///
	#endif

	#ifdef WORD1B_ANALYSIS
	std::set<unsigned char> word1B_list;
	hpcl_comp_lwm_pl_proc<K>::decompose_data(word, word1B_list);

	std::set<unsigned char>::iterator it;
	for(it = word1B_list.begin(); it != word1B_list.end(); ++it) {
	  m_loc_dict_for_uncomp_data->update_dict(*it, 0);
	}

	std::vector<unsigned char> tmp;
	hpcl_comp_lwm_pl_proc<K>::decompose_data(word, tmp);
	uncompdata.push_back(tmp);
	#endif
	/*
	std::cout << "\t";
	loc_dict->print_word(word);
	std::cout << " is not found! : word_loc " << i << std::endl;
	*/

      }

      unsigned cur_data_segment_size = (i+1)*sizeof(K);
      if(cur_data_segment_size == 32)		comp_data_segment_bits[0] = comp_bit_size;
      else if(cur_data_segment_size == 64) 	comp_data_segment_bits[1] = comp_bit_size-comp_data_segment_bits[0];
      else if(cur_data_segment_size == 96)	comp_data_segment_bits[2] = comp_bit_size-comp_data_segment_bits[0]-comp_data_segment_bits[1];
      else if(cur_data_segment_size == 128)	comp_data_segment_bits[3] = comp_bit_size-comp_data_segment_bits[0]-comp_data_segment_bits[1]-comp_data_segment_bits[2];

      unsigned cur_data_segment_head = i*sizeof(K);
      if(cur_data_segment_head/32 == 0) {
	if(is_word_compressed == true)	no_comp_data_segment_ratio[0] = 0;
      } else if(cur_data_segment_head/32 == 1) {
	if(is_word_compressed == true)	no_comp_data_segment_ratio[1] = 0;
      } else if(cur_data_segment_head/32 == 2) {
	if(is_word_compressed == true)	no_comp_data_segment_ratio[2] = 0;
      } else if(cur_data_segment_head/32 == 3) {
	if(is_word_compressed == true)	no_comp_data_segment_ratio[3] = 0;
      }

      //added by kh(061216)
      //save number of matching words for each data segment
      if(cur_data_segment_size == 32) {
	comp_data_segment_matching_word_no[0] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 64) {
	comp_data_segment_matching_word_no[1] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 96) {
	comp_data_segment_matching_word_no[2] = matching_word_no;
	matching_word_no = 0;
      } else if(cur_data_segment_size == 128) {
	comp_data_segment_matching_word_no[3] = matching_word_no;
	matching_word_no = 0;
      }
      ///
  }

  //std::cout << "\t last_max_word_loc " << loc_dict->get_last_max_word_loc() << std::endl;

//  bool dynamic_dict_size_enabled = false;
//  if(sizeof(K) == 2) {
//    if(g_hpcl_comp_config.hpcl_dict_window_mode_2B == hpcl_comp_config::DICT_WINDOW_DYNAMIC) {
//      dynamic_dict_size_enabled = true;
//    }
//  } else if(sizeof(K) == 4) {
//    if(g_hpcl_comp_config.hpcl_dict_window_mode_4B == hpcl_comp_config::DICT_WINDOW_DYNAMIC) {
//      dynamic_dict_size_enabled = true;
//    }
//  } else if(sizeof(K) == 8) {
//    if(g_hpcl_comp_config.hpcl_dict_window_mode_8B == hpcl_comp_config::DICT_WINDOW_DYNAMIC) {
//      dynamic_dict_size_enabled = true;
//    }
//  }

  //added by kh(061216)
  unsigned comp_header = 0;

  //added by kh(061216)
  //Dictionary Size Adjustment
  if(g_hpcl_comp_config.hpcl_dict_size_opt_en == 1) {
    int last_max_word_index = loc_dict->get_last_max_word_loc();
    if(last_max_word_index >= 0) {
      //double dict_window_size = ()*sizeof(K);
      unsigned new_data_ptr_bits = (unsigned) ceil(log2(last_max_word_index+1));
      if(new_data_ptr_bits == 0)	new_data_ptr_bits = 1;


      assert(new_data_ptr_bits <= data_ptr_bits);
      //printf("data_ptr_bits : %u, new_data_ptr_bits : %u\n", data_ptr_bits, new_data_ptr_bits);
      unsigned all_matching_no = 0;
      unsigned new_comp_bit_size = 0;
      for(unsigned i = 0; i < comp_data_segment_matching_word_no.size(); i++) {
	comp_data_segment_bits[i] = comp_data_segment_bits[i] - (data_ptr_bits-new_data_ptr_bits)*comp_data_segment_matching_word_no[i];
	new_comp_bit_size += comp_data_segment_bits[i];
	all_matching_no += comp_data_segment_matching_word_no[i];
      }

      comp_header += 3;	//comp_header : 3 bits for index bit size: 000 --> 1bit, 101 --> 6bit

      //printf("DICT_SIZE_ADJ | mf %u res %u comp_bit_size : %u, new_comp_bit_size : %u, matching_word_no : %u\n", mf->get_request_uid(), sizeof(K), comp_bit_size, (new_comp_bit_size+comp_header), all_matching_no);
      comp_bit_size = (new_comp_bit_size+comp_header);
    }
  }
  ///

  //added by kh(061216)
  //Partial Compression
  if(g_hpcl_comp_config.hpcl_partial_comp_en == 1) {
    unsigned comp_bit_size_by_partial_comp = 0;
    unsigned no_comp_ds_no = 0;
    for(unsigned i = 0; i < no_comp_data_segment_ratio.size(); i++) {
      if(no_comp_data_segment_ratio[i] == 1) {
	comp_data_segment_bits[i] = comp_data_segment_bits[i] - 32/sizeof(K);
	no_comp_ds_no++;
      }
      comp_bit_size_by_partial_comp += comp_data_segment_bits[i];
    }
    comp_header += 4;	//comp_header : 4 bits each bit status of compression of each ds

    //printf("PART_COMP | mf %u res %u comp_bit_size %u, comp_bit_size_by_partial_comp %u, no_comp_ds_no %u\n", mf->get_request_uid(), sizeof(K), comp_bit_size, (comp_bit_size_by_partial_comp+comp_header), no_comp_ds_no);
    ///
    comp_bit_size = (comp_bit_size_by_partial_comp+comp_header);


  }

  //added by kh(061016)
  for(unsigned i = 0; i < cache_data.size()/32; i++) {
    loc_dict->set_comp_data_segment_ratio(i, (32*8-comp_data_segment_bits[i])/(32*8));
    loc_dict->set_no_comp_data_segment_ratio(i, no_comp_data_segment_ratio[i]);
  }
  ///

  unsigned comp_byte_size = ceil((double)comp_bit_size/8);
  mf->set_comp_data_size(comp_byte_size);
  //added by kh(062416)
  mf->set_comp_data_bits(comp_bit_size, sizeof(K));
  ///


  #ifdef DEBUG_LWM
  if(comp_byte_size < mf->get_real_data_size()) {
    printf("access_type %u org_data = 0x", mf->get_access_type());
    for(unsigned i = 0; i < cache_data.size(); i++) {
      printf("%02x", cache_data[i]);
    }
    printf("\n");
    printf("org_data_size = %d, comp_data_size = %d\n", mf->get_real_data_size(), comp_byte_size);
    m_loc_dict->print();
  }
  #endif

  //added by kh(042116)
  //m_loc_dict->print();
  double zeros_rate = 0;
  double nonzeros_rate = 0;
  double uni_val_rate = 0;
  loc_dict->get_word_redun_type_dist(zeros_rate, nonzeros_rate, uni_val_rate);
  if(sizeof(K) == 2) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_UNI_VAL_RATE, uni_val_rate);
  } else if(sizeof(K) == 4) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_UNI_VAL_RATE, uni_val_rate);
  } else if(sizeof(K) == 8) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_UNI_VAL_RATE, uni_val_rate);
  }
  ///

  //deleted by kh(051816)
  //delete a local dictionary
  //loc_dict->clear();



  #ifdef WORD1B_ANALYSIS
  //added by kh(042216)
  m_word1B_zeros_rate_in_uncomp_data = 0;
  m_word1B_nonzeros_rate_in_uncomp_data = 0;
  m_word1B_uni_val_rate_in_uncomp_data = 0;

  //loc_dict->print();
  m_loc_dict_for_uncomp_data->get_word_redun_type_dist(m_word1B_zeros_rate_in_uncomp_data, m_word1B_nonzeros_rate_in_uncomp_data, m_word1B_uni_val_rate_in_uncomp_data);
  //printf("word1B_rate_mf_%u = %f, %f, %f\n", mf->get_request_uid(), m_word1B_zeros_rate_in_uncomp_data, m_word1B_nonzeros_rate_in_uncomp_data, m_word1B_uni_val_rate_in_uncomp_data);


  for(unsigned i = 0; i < uncompdata.size(); i++)
  {
    for(unsigned j = 0; j < uncompdata[i].size(); j++)
    {
      int index = m_loc_dict_for_uncomp_data->search_word(uncompdata[i][j]);
      if(m_loc_dict_for_uncomp_data->get_freq(index) > 1) {
	mf->push_uncompword(sizeof(K), i, uncompdata[i][j], mem_fetch::INTRA_COMPRESSED);
      } else if(m_loc_dict_for_uncomp_data->get_freq(index) == 1) {
	mf->push_uncompword(sizeof(K), i, uncompdata[i][j], mem_fetch::UNCOMPRESSED);
      } else assert(0);
    }
  }

  m_word1B_nonzeros_no = m_loc_dict_for_uncomp_data->get_word_no_with_multi_freq();


  m_loc_dict_for_uncomp_data->clear();
  ///
  #endif


   /*
   printf("org_data = 0x");
   for(unsigned i = 0; i < cache_data.size(); i++) {
     printf("%02x", cache_data[i]);
   }
   printf("\n");
   */
   //printf("org_data_size = %d, comp_data_size = %d\n", mf->get_real_data_size(), comp_byte_size);
   //m_loc_dict->print();

  //Data Remapping Support
  run_comp_for_remapped_data(mf, 0);







#ifdef old
  //added by kh(061016)
  std::vector<double> trans_comp_data_segment_bits;
  trans_comp_data_segment_bits.resize(cache_data.size()/32, -1);
  std::vector<int> trans_no_comp_data_segment_ratio;
  trans_no_comp_data_segment_ratio.resize(cache_data.size()/32, 1);
  ///

  //added by kh(061216)
  std::vector<unsigned> trans_comp_data_segment_matching_word_no;
  trans_comp_data_segment_matching_word_no.resize(cache_data.size()/32, 0);
  unsigned trans_matching_word_no = 0;
  ///

  unsigned trans_comp_bit_size = 0;
  for(unsigned i = 0; i < trans_word_list.size(); i++) {

      //added by kh(061016)
      bool trans_is_word_compressed = false;

      trans_comp_bit_size += 1;		//flag(1 bit)
      K word = trans_word_list[i];
      int word_index = trans_loc_dict->search_word(word);
      if(word_index >= 0) {
	trans_comp_bit_size += data_ptr_bits;
	trans_loc_dict->update_dict(word, 0);

	//added by kh(061016)
	//store the max last word index
	int trans_word_loc = trans_loc_dict->get_word_loc(word_index);
	//assert(trans_word_loc >= 0);
	if(trans_loc_dict->get_last_max_word_loc() < trans_word_loc) {
	  trans_loc_dict->set_last_max_word_loc(trans_word_loc);
	}
	///

	//#ifdef DEBUG_LWM
	/*
	std::cout << "\t trans--> ";
	trans_loc_dict->print_word(word);
	std::cout << " is found at loc " << trans_word_loc << std::endl;
	std::cout << "\t max_word_loc " << trans_loc_dict->get_last_max_word_loc() << std::endl;
	*/


	//added by kh(061016)
	trans_is_word_compressed = true;
	///

	//added by kh(061216)
	trans_matching_word_no++;
	///


      } else {

	//#ifdef DEBUG_LWM
	/*
	std::cout << "\t trans-->";
	trans_loc_dict->print_word(word);
	std::cout << " is not found! word_loc " << i << std::endl;
	*/
	//#endif

	trans_comp_bit_size += (sizeof(K)*8);

	double dict_size = sizeof(K)*(i+1);
	if(dict_size <= max_dict_size) {
	  int index = trans_loc_dict->update_dict(word, 0);
	  trans_loc_dict->set_word_loc(index, i);
	  /*
	  std::cout << "\t ";
	  trans_loc_dict->print_word(word);
	  std::cout << " trans_dict index " << index << " trans_word_loc " << i << std::endl;
	  */
	}

      }

      //added by kh(061016)
      unsigned cur_data_segment_size = (i+1)*sizeof(K);
      if(cur_data_segment_size == 32)		trans_comp_data_segment_bits[0] = trans_comp_bit_size;
      else if(cur_data_segment_size == 64) 	trans_comp_data_segment_bits[1] = trans_comp_bit_size-trans_comp_data_segment_bits[0];
      else if(cur_data_segment_size == 96)	trans_comp_data_segment_bits[2] = trans_comp_bit_size-trans_comp_data_segment_bits[0]-trans_comp_data_segment_bits[1];
      else if(cur_data_segment_size == 128)	trans_comp_data_segment_bits[3] = trans_comp_bit_size-trans_comp_data_segment_bits[0]-trans_comp_data_segment_bits[1]-trans_comp_data_segment_bits[2];

      unsigned cur_data_segment_head = i*sizeof(K);
      if(cur_data_segment_head/32 == 0) {
	if(trans_is_word_compressed == true)	trans_no_comp_data_segment_ratio[0] = 0;
      } else if(cur_data_segment_head/32 == 1) {
	if(trans_is_word_compressed == true)	trans_no_comp_data_segment_ratio[1] = 0;
      } else if(cur_data_segment_head/32 == 2) {
	if(trans_is_word_compressed == true)	trans_no_comp_data_segment_ratio[2] = 0;
      } else if(cur_data_segment_head/32 == 3) {
	if(trans_is_word_compressed == true)	trans_no_comp_data_segment_ratio[3] = 0;
      }
      ///

      //added by kh(061216)
      //save number of matching words for each data segment
      if(cur_data_segment_size == 32) {
	trans_comp_data_segment_matching_word_no[0] = trans_matching_word_no;
	trans_matching_word_no = 0;
      } else if(cur_data_segment_size == 64) {
	trans_comp_data_segment_matching_word_no[1] = trans_matching_word_no;
	trans_matching_word_no = 0;
      } else if(cur_data_segment_size == 96) {
	trans_comp_data_segment_matching_word_no[2] = trans_matching_word_no;
	trans_matching_word_no = 0;
      } else if(cur_data_segment_size == 128) {
	trans_comp_data_segment_matching_word_no[3] = trans_matching_word_no;
	trans_matching_word_no = 0;
      }
      ///


  }

  //added by kh(061216)
  unsigned trans_comp_header = 0;

  //added by kh(061216)
  //Dictionary Size Adjustment
  if(dynamic_dict_size_enabled == true) {
    int trans_last_max_word_index = trans_loc_dict->get_last_max_word_loc();
    if(trans_last_max_word_index >= 0) {
      //double dict_window_size = ()*sizeof(K);
      unsigned new_data_ptr_bits = (unsigned) ceil(log2(trans_last_max_word_index+1));
      if(new_data_ptr_bits == 0)	new_data_ptr_bits = 1;

      assert(new_data_ptr_bits <= data_ptr_bits);
      //printf("trans_data_ptr_bits : %u, trans_new_data_ptr_bits : %u\n", data_ptr_bits, new_data_ptr_bits);
      unsigned all_matching_no = 0;
      unsigned new_comp_bit_size = 0;
      for(unsigned i = 0; i < trans_comp_data_segment_matching_word_no.size(); i++) {
	  trans_comp_data_segment_bits[i] = trans_comp_data_segment_bits[i] - (data_ptr_bits-new_data_ptr_bits)*trans_comp_data_segment_matching_word_no[i];
	new_comp_bit_size += trans_comp_data_segment_bits[i];
	all_matching_no += trans_comp_data_segment_matching_word_no[i];
      }
      trans_comp_header += 3;	//comp_header : 3 bits for index bit size: 000 --> 1bit, 101 --> 6bit

      //printf("DICT_SIZE_ADJ | mf %u res %u trans_comp_bit_size : %u, trans_new_comp_bit_size : %u, matching_word_no : %u\n", mf->get_request_uid(), sizeof(K), trans_comp_bit_size, (new_comp_bit_size+trans_comp_header), all_matching_no);
      trans_comp_bit_size = (new_comp_bit_size+trans_comp_header);
    }
  }
  ///

  //added by kh(061216)
  //Partial Compression
  if(g_hpcl_comp_config.hpcl_partial_comp_en == 1) {
    unsigned trans_comp_bit_size_by_partial_comp = 0;
    unsigned trans_no_comp_ds_no = 0;
    for(unsigned i = 0; i < trans_no_comp_data_segment_ratio.size(); i++) {
      if(trans_no_comp_data_segment_ratio[i] == 1) {
	trans_comp_data_segment_bits[i] = trans_comp_data_segment_bits[i] - 32/sizeof(K);
	trans_no_comp_ds_no++;
      }
      trans_comp_bit_size_by_partial_comp += trans_comp_data_segment_bits[i];
    }
    trans_comp_header += 4;	//comp_header : 4 bits each bit status of compression of each ds

    //printf("PART_COMP | mf %u res %u trans_comp_bit_size %u, trans_comp_bit_size_by_partial_comp %u, trans_no_comp_ds_no %u\n", mf->get_request_uid(), sizeof(K), trans_comp_bit_size, (trans_comp_bit_size_by_partial_comp+trans_comp_header), trans_no_comp_ds_no);
    trans_comp_bit_size = (trans_comp_bit_size_by_partial_comp+trans_comp_header);
  }
  ///

  //printf("trans_comp_data_segment_bits: %3.2f %3.2f %3.2f %3.2f\n", trans_comp_data_segment_bits[0], trans_comp_data_segment_bits[1], trans_comp_data_segment_bits[2], trans_comp_data_segment_bits[3]);
  //printf("trans_no_comp_data_segment_ratio: %d %d %d %d\n", trans_no_comp_data_segment_ratio[0], trans_no_comp_data_segment_ratio[1], trans_no_comp_data_segment_ratio[2], trans_no_comp_data_segment_ratio[3]);

  //added by kh(061016)
  for(unsigned i = 0; i < cache_data.size()/32; i++) {
    trans_loc_dict->set_comp_data_segment_ratio(i, (32*8-trans_comp_data_segment_bits[i])/(32*8));
    trans_loc_dict->set_no_comp_data_segment_ratio(i, trans_no_comp_data_segment_ratio[i]);
  }
  /*
  if(trans_comp_data_segment_bits[0] >= 0)	trans_loc_dict->set_comp_data_ratio_segment(0, trans_comp_data_segment_bits[0]/(32*8));
  if(trans_comp_data_segment_bits[1] >= 0)	trans_loc_dict->set_comp_data_ratio_segment(1, trans_comp_data_segment_bits[1]/(32*8));
  if(trans_comp_data_segment_bits[2] >= 0)	trans_loc_dict->set_comp_data_ratio_segment(2, trans_comp_data_segment_bits[2]/(32*8));
  if(trans_comp_data_segment_bits[3] >= 0)	trans_loc_dict->set_comp_data_ratio_segment(3, trans_comp_data_segment_bits[3]/(32*8));
  */
  ///

  //unsigned trans_comp_byte_size = ceil((double)trans_comp_bit_size/8);
  //mf->set_trans_comp_data_size(trans_comp_byte_size, sizeof(K));
  mf->set_trans_comp_data_bits(trans_comp_bit_size, sizeof(K));
  //}
  ///

#endif

  #endif
}
#endif

template <class K>
void hpcl_comp_lwm_pl_proc<K>::decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list)
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
void hpcl_comp_lwm_pl_proc<K>::decompose_data(K word, std::set<unsigned char>& word1B_list)
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
void hpcl_comp_lwm_pl_proc<K>::decompose_data(K word, std::vector<unsigned char>& word1B_list)
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
void hpcl_comp_lwm_pl_proc<K>::print_word_list(std::vector<K>& word_list)
{
  for(unsigned i = 0; i < word_list.size(); i++) {
    printf("\tword%u -- ", i);
    hpcl_dict_elem<K>::print_word_data(word_list[i]);
    printf("\n");
  }
}

#endif /* HPCL_COMP_PL_PROC_H_ */
