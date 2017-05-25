/*
 * hpcl_dict.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DICT_H_
#define HPCL_DICT_H_

#include <vector>
#include <map>
#include "hpcl_dict_elem.h"
#include <cassert>
#include <cmath>

enum hpcl_dict_rep_policy {
  HPCL_INVALID = 0,
  HPCL_LRU,
  HPCL_LFU
};

template<class K>
class hpcl_dict {
/*
public:
	hpcl_dict();
	virtual ~hpcl_dict();
*/
public:
  hpcl_dict(unsigned int dict_size, enum hpcl_dict_rep_policy policy);
  ~hpcl_dict() {
    //printf("delete hpcl_dict %u\n", sizeof(K));
    //added by kh(051916)
    for(unsigned i = 0; i < m_dict.size(); i++) {
      delete m_dict[i];
    }
    m_dict.clear();
  };

//compression
private:
  unsigned int m_MAX_DICT_SIZE;			//default: 256
  //unsigned int m_dict_word_size;		//default: 2B
  enum hpcl_dict_rep_policy m_rep_policy;
  std::vector<hpcl_dict_elem<K>*> m_dict;
  //std::map<K, hpcl_dict_elem<K>*> m_dict;

public:
  void init(unsigned int dict_size, enum hpcl_dict_rep_policy policy);
  int search_word(K word);
  K get_word(unsigned index);
  bool get_valid_flag(unsigned index);

  //deleted by kh(051816)
  //void update_dict(K word, unsigned long long use_time, int index=-1);
  //void update_elem_by_LRU(K word, unsigned long long time);
  //void update_elem_by_LFU(K word, unsigned long long time);
  ///

  //added by kh(051816)
  int update_dict(K word, unsigned long long use_time, int index=-1);
  int update_elem_by_LRU(K word, unsigned long long time);
  int update_elem_by_LFU(K word, unsigned long long time);
  ///

  void print();
  int get_index_bit_size();

//added by kh(022416)
  int get_freq(unsigned index);
  K search_victim_by_LFU();
  int get_free_entry();
///

  //added by kh(030716)
  void erase_dict(K word);

  //added by kh(030816)
  void validate_word(int index, K word);
  void hold_word(int index, K word);
  void release_word(int index, K word);

  //added by kh(031716)
public:
  void clear();
  ///

  //added by kh(031916)
  void print_word(K word);


  //added by kh(042116)
  void get_word_redun_type_dist(double& zeros_rate, double& nonzeros_rate, double& uni_val_rate);
  //double get_zeros_rep_rate();
  //double get_nonzeros_rep_rate();
  //double get_uni_val_rate();
  ///

  unsigned get_word_no_with_multi_freq();


  //added by kh(051816)
  unsigned get_valid_word_no();
  void compare_dict(hpcl_dict<K>* dict, unsigned& same_word_no);
  ///


  //added by kh(053116)
  //for recursive compression window
  void set_found_flag(unsigned index);
  bool get_found_flag(unsigned index);
  ///

  //added by kh(060416)
  void clear_found_flag(unsigned index);
  unsigned get_word_no();
  ///

  //added by kh(061016)
private:
  int m_last_max_word_loc;
public:
  void set_last_max_word_loc(int last_max_word_index);
  int get_last_max_word_loc();

private:
  double m_comp_data_segment0_ratio;	//first data segment(32B)
  double m_comp_data_segment1_ratio;	//first data segment(32B)
  double m_comp_data_segment2_ratio;	//first data segment(32B)
  double m_comp_data_segment3_ratio;	//first data segment(32B)
public:
  double get_comp_data_segment_ratio(unsigned id);
  void set_comp_data_segment_ratio(unsigned id, double comp_data_ratio_segment);


private:
  int m_no_comp_data_segment0_ratio;	//first data segment(32B)
  int m_no_comp_data_segment1_ratio;	//first data segment(32B)
  int m_no_comp_data_segment2_ratio;	//first data segment(32B)
  int m_no_comp_data_segment3_ratio;	//first data segment(32B)
public:
  int get_no_comp_data_segment_ratio(unsigned id);
  void set_no_comp_data_segment_ratio(unsigned id, int comp_data_ratio_segment);


  //added by kh(061216)
  //store word's location in an original data
public:
  void set_word_loc(unsigned index, int word_loc);
  int get_word_loc(unsigned index);
  ///


  //added by kh(071216)
public:
  unsigned get_redund_nibble_no();






};

template<class K>
hpcl_dict<K>::hpcl_dict(unsigned int dict_size, enum hpcl_dict_rep_policy policy)
{
  init(dict_size, policy);
}

template<class K>
void hpcl_dict<K>::init(unsigned int dict_size, enum hpcl_dict_rep_policy policy)
{
  m_MAX_DICT_SIZE = dict_size;
  m_rep_policy = policy;

  //added by kh(030216)
  for(int i = 0; i < m_MAX_DICT_SIZE; i++) {
    m_dict.push_back(new hpcl_dict_elem<K>(0, 0));
  }
  ///

  //added by kh(061016)
  m_last_max_word_loc = -1;
  m_comp_data_segment0_ratio = -1;
  m_comp_data_segment1_ratio = -1;
  m_comp_data_segment2_ratio = -1;
  m_comp_data_segment3_ratio = -1;

  m_no_comp_data_segment0_ratio = -1;
  m_no_comp_data_segment1_ratio = -1;
  m_no_comp_data_segment2_ratio = -1;
  m_no_comp_data_segment3_ratio = -1;
  ///

}

template<class K>
int hpcl_dict<K>::search_word(K word)
{
  int ret = -1;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    //if(m_dict[i]->get_valid() == true && m_dict[i]->search(word) == true) {
    if(m_dict[i]->search(word) == true) {
      ret = (int) i; break;
    }
  }
  return ret;
}
template<class K>
bool hpcl_dict<K>::get_valid_flag(unsigned index) {
  return m_dict[index]->get_valid();
}

template<class K>
K hpcl_dict<K>::get_word(unsigned index)
{
  assert(m_dict.size() > index);
  //assert(m_dict[index]->get_valid() == true);
  return m_dict[index]->get_word();
}

template<class K>
void hpcl_dict<K>::erase_dict(K word)
{
  int index = search_word(word);
  assert(index>=0);
  m_dict[index]->clear_valid();
}

template<class K>
void hpcl_dict<K>::validate_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->set_valid();
}

template<class K>
void hpcl_dict<K>::hold_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->set_hold_flag();
}

template<class K>
void hpcl_dict<K>::release_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->clear_hold_flag();
}

template<class K>
int hpcl_dict<K>::update_dict(K word, unsigned long long use_time, int index)
{
  if(index >= 0) {
    //store word in the specified index
    //std::cout << "update_dict word " << word << " word_index " << index << " m_dict.size() " << m_dict.size() << std::endl;
    m_dict[index]->init(word, use_time);
    //m_dict[index]->set_valid();

    //added by kh(051816)
    return index;

  } else {
    bool found = false;
    for(unsigned i = 0; i < m_dict.size(); i++) {
      //if(m_dict[i]->get_valid() == true && m_dict[i]->search(word) == true) {
      if(m_dict[i]->search(word) == true) {
	m_dict[i]->update_last_use_time(use_time);
	m_dict[i]->update_freq();
	found = true;

	//added by kh(051816)
	return i;
	//deleted by kh(051816)
	//break;
      }
    }

    if(found == false) {
      if(m_dict.size() < m_MAX_DICT_SIZE) {
	hpcl_dict_elem<K> * new_elem = new hpcl_dict_elem<K>(word, use_time);
	m_dict.push_back(new_elem);
	//added by kh(051816)
	return (m_dict.size()-1);
	//assert(0);
      } else {

	//added by kh(061216)
	int ret = 0;

	if(m_rep_policy == HPCL_LRU)	      	ret = update_elem_by_LRU(word, use_time);
	else if(m_rep_policy == HPCL_LFU)  	ret = update_elem_by_LFU(word, use_time);
	else {
	    printf("m_rep_policy %d, m_dict.size() %u, m_MAX_DICT_SIZE %u\n", m_rep_policy, m_dict.size(), m_MAX_DICT_SIZE);
	    assert(0);
	}

	return ret;
      }
    }
  }



}

template<class K>
int hpcl_dict<K>::update_elem_by_LRU(K word, unsigned long long time)
{
  unsigned long long last_use_time = m_dict[0]->get_last_use_time();
  unsigned oldest_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_last_use_time() < last_use_time) {
      oldest_index = i;
    }
  }

  /*
  printf("index %u, ", oldest_index);
  printf("old_word ");
  m_dict[oldest_index]->print_word();
  */
  m_dict[oldest_index]->init(word, time);
  //m_dict[oldest_index]->set_valid();
  /*
  printf(" --> new_word ");
  m_dict[oldest_index]->print_word();
  printf("\n");
  */

  //added by kh(051816)
  return oldest_index;
}

template<class K>
int hpcl_dict<K>::update_elem_by_LFU(K word, unsigned long long time)
{
  unsigned long long freq = m_dict[0]->get_freq();
  unsigned least_used_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() < freq && m_dict[i]->get_hold_flag() == false) {
      least_used_index = i;
    }
  }
  /*
  printf("index %u, ", least_used_index);
  printf("old_word ");
  m_dict[least_used_index]->print_word();
  */
  m_dict[least_used_index]->init(word, time);
  //m_dict[least_used_index]->set_valid();
  /*
  printf(" --> new_word ");
  m_dict[least_used_index]->print_word();
  printf("\n");
  */

  //added by kh(051816)
  return least_used_index;
}

template<class K>
void hpcl_dict<K>::print()
{
  for(int i = 0; i < m_dict.size(); i++) {
    printf("index = %d ", i);
    m_dict[i]->print();
    printf("\n");
  }
}

template<class K>
int hpcl_dict<K>::get_index_bit_size() {
  return (int)log2(m_MAX_DICT_SIZE);
}

template<class K>
int hpcl_dict<K>::get_freq(unsigned index) {
  return m_dict[index]->get_freq();
}

template<class K>
K hpcl_dict<K>::search_victim_by_LFU()
{
  unsigned long long freq = m_dict[0]->get_freq();
  K victim_word = m_dict[0]->get_word();
  unsigned least_used_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() < freq) {
      least_used_index = i;
      victim_word = m_dict[i]->get_word();
    }
  }
  return victim_word;
}

template<class K>
int hpcl_dict<K>::get_free_entry()
{
  int ret = -1;
  if(m_dict.size() < m_MAX_DICT_SIZE) {
    ret = m_dict.size();
  }
  return ret;
}

//added by kh(031716)
template<class K>
void hpcl_dict<K>::clear()
{
  for(int i = 0; i < m_dict.size(); i++) {
    delete m_dict[i];
  }
  m_dict.clear();
}
///

template<class K>
void hpcl_dict<K>::print_word(K word)
{
  if(sizeof(K) == sizeof(unsigned char)) {
    printf("%02x", word);
  } else if(sizeof(K) == sizeof(unsigned short)) {
    printf("%04x", word);
  } else if(sizeof(K) == sizeof(unsigned int)) {
    printf("%08x", word);
  } else if(sizeof(K) == sizeof(unsigned long long)) {
    printf("%016llx", word);
  }
}

template<class K>
void hpcl_dict<K>::get_word_redun_type_dist(double& zeros_rate, double& nonzeros_rate, double& uni_val_rate) {
  unsigned all_freq = 0;
  zeros_rate = 0;
  nonzeros_rate = 0;
  uni_val_rate = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
      all_freq += m_dict[i]->get_freq();
      if(m_dict[i]->get_word() == 0 && m_dict[i]->get_freq() > 1) {
	  zeros_rate += m_dict[i]->get_freq();
      } else if(m_dict[i]->get_word() != 0 && m_dict[i]->get_freq() > 1) {
	  nonzeros_rate += m_dict[i]->get_freq();
      } else if(m_dict[i]->get_freq() == 1) {
	  uni_val_rate += m_dict[i]->get_freq();
      } else	assert(0);
  }

  if(all_freq > 0) {	//to avoid -nan
    zeros_rate = zeros_rate/all_freq;
    nonzeros_rate = nonzeros_rate/all_freq;
    uni_val_rate = uni_val_rate/all_freq;
  }
}

template<class K>
unsigned hpcl_dict<K>::get_word_no_with_multi_freq()
{
  unsigned ret = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() > 1) ret++;
  }
  return ret;
}


template<class K>
unsigned hpcl_dict<K>::get_valid_word_no()
{
  unsigned ret = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    //assert(m_dict[i]->get_freq() >= 1);
    if(m_dict[i]->get_valid() == true) {
      ret++;
    }
  }
  return ret;
}

template<class K>
void hpcl_dict<K>::compare_dict(hpcl_dict<K>* dict, unsigned& same_word_no)
{
  same_word_no = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    //assert(m_dict[i]->get_freq() >= 1);
    K word = m_dict[i]->get_word();
    int index = dict->search_word(word);
    //deleted by kh(053116)
    //if(index >= 0) {
    //added by kh(053116)
    if(index >= 0 && dict->get_found_flag(index) == false) {
      same_word_no++;

      //added by kh(053116)
      dict->set_found_flag(index);
    }
  }
}

template<class K>
void hpcl_dict<K>::set_found_flag(unsigned index)
{
  //assert(index >= 0 && index < m_dict.size());
  m_dict[index]->set_found_flag();
}

template<class K>
bool hpcl_dict<K>::get_found_flag(unsigned index)
{
  //assert(index >= 0 && index < m_dict.size());
  return m_dict[index]->get_found_flag();
}


template<class K>
void hpcl_dict<K>::clear_found_flag(unsigned index)
{
  //assert(index >= 0 && index < m_dict.size());
  m_dict[index]->clear_found_flag();
}

template<class K>
unsigned hpcl_dict<K>::get_word_no()
{
  return m_dict.size();
}

template<class K>
void hpcl_dict<K>::set_last_max_word_loc(int last_max_word_loc) {
  m_last_max_word_loc = last_max_word_loc;
}

template<class K>
int hpcl_dict<K>::get_last_max_word_loc() {
  return m_last_max_word_loc;
}

template<class K>
double hpcl_dict<K>::get_comp_data_segment_ratio(unsigned id) {
  if(id == 0)		return m_comp_data_segment0_ratio;
  else if(id == 1)	return m_comp_data_segment1_ratio;
  else if(id == 2)	return m_comp_data_segment2_ratio;
  else if(id == 3)	return m_comp_data_segment3_ratio;
  else assert(0);

  return -1;
}

template<class K>
void hpcl_dict<K>::set_comp_data_segment_ratio(unsigned id, double comp_data_ratio_segment) {
  if(id == 0)		m_comp_data_segment0_ratio = comp_data_ratio_segment;
  else if(id == 1)	m_comp_data_segment1_ratio = comp_data_ratio_segment;
  else if(id == 2)	m_comp_data_segment2_ratio = comp_data_ratio_segment;
  else if(id == 3)	m_comp_data_segment3_ratio = comp_data_ratio_segment;
  else	assert(0);
}

template<class K>
int hpcl_dict<K>::get_no_comp_data_segment_ratio(unsigned id) {
  if(id == 0)		return m_no_comp_data_segment0_ratio;
  else if(id == 1)	return m_no_comp_data_segment1_ratio;
  else if(id == 2)	return m_no_comp_data_segment2_ratio;
  else if(id == 3)	return m_no_comp_data_segment3_ratio;
  else	assert(0);
}

template<class K>
void hpcl_dict<K>::set_no_comp_data_segment_ratio(unsigned id, int comp_data_ratio_segment) {
  if(id == 0)		m_no_comp_data_segment0_ratio = comp_data_ratio_segment;
  else if(id == 1)	m_no_comp_data_segment1_ratio = comp_data_ratio_segment;
  else if(id == 2)	m_no_comp_data_segment2_ratio = comp_data_ratio_segment;
  else if(id == 3)	m_no_comp_data_segment3_ratio = comp_data_ratio_segment;
  else 	assert(0);
}

template<class K>
void hpcl_dict<K>::set_word_loc(unsigned index, int word_loc)
{
  m_dict[index]->set_word_loc(word_loc);
}

template<class K>
int hpcl_dict<K>::get_word_loc(unsigned index)
{
  return m_dict[index]->get_word_loc();
}

template<class K>
unsigned hpcl_dict<K>::get_redund_nibble_no()
{
  std::map<unsigned, unsigned> nibble_cnt;
  for(int i = 0; i < 16; i++) {
    nibble_cnt[i] = 0;
  }
  for(int i = 0; i < m_dict.size(); i++) {
    K word = m_dict[i]->get_word();
    for(int j = 0; j < sizeof(K); j++) {
      unsigned nibble = word & 0x0f;
      word = (word>>4);
      nibble_cnt[nibble]++;
    }
  }

  unsigned ret = 0;
  for(int i = 0; i < 16; i++) {
    if(nibble_cnt[i] > 1)	ret += nibble_cnt[i];
  }
  return ret;
}


#endif /* HPCL_DICT_H_ */
