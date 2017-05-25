/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_CUDA_MEM_FETCH_H_
#define HPCL_CUDA_MEM_FETCH_H_

#include <vector>
#include <cassert>

//added by kh(081516)
#include "memory.h"



class hpcl_cuda_mem_fetch {

public:
  hpcl_cuda_mem_fetch(unsigned pc, unsigned sid, unsigned wid);
  ~hpcl_cuda_mem_fetch();
public:
  enum COMP_DATA_TYPE {
    NO_DATA_TYPE = -1,
    ORG_DATA,
    REMAPPED_DATA_1,	//4B
    REMAPPED_DATA_2,	//4B
    REMAPPED_DATA_3,	//8B
    REMAPPED_DATA_4,	//8B
    REMAPPED_DATA_5,	//2B
    REMAPPED_DATA_6,	//2B
  };

private:
  std::vector<unsigned> m_remap_op_bits;

public:
  void set_remap_op_bits(unsigned index, unsigned remap_op_bits);
  unsigned get_remap_op_bits(unsigned index);


private:
  std::vector<unsigned char> m_real_data;
  unsigned m_comp_data_size;	//best compression result
  unsigned m_comp_res;

public:
  void set_real_data(std::vector<unsigned char>* data);
  unsigned char get_real_data(unsigned data_index);
  unsigned get_real_data_size();
  std::vector<unsigned char>& get_real_data_ptr();
  void copy_real_data(std::vector<unsigned char>& tmp);


private:
  std::vector<std::vector<unsigned char> > m_trans_data;
  std::vector<enum COMP_DATA_TYPE> m_trans_data_type;
public:
  unsigned get_trans_data_size(unsigned index=0);
  std::vector<unsigned char>* config_remapped_data(unsigned index, unsigned data_size);
  unsigned get_trans_data_no();
  std::vector<unsigned char>& get_trans_data_ptr(unsigned index);

public:
  void set_remapped_data_type(unsigned index, enum COMP_DATA_TYPE data_type);
  enum COMP_DATA_TYPE get_remapped_data_type(unsigned index);
  int get_remapped_data_type_index(enum COMP_DATA_TYPE data_type);


public:
//debugging
  void print_data_all(unsigned space_gap);
  void print_data(std::vector<unsigned char>& data, unsigned space_gap);
  void print_real_data(unsigned space_gap);


private:
  std::vector<void*> m_trans_loc_dict_2B;
  std::vector<void*> m_trans_loc_dict_4B;
  std::vector<void*> m_trans_loc_dict_8B;
  std::vector<unsigned> m_comp_trans_data_bits_2B;
  std::vector<unsigned> m_comp_trans_data_bits_4B;
  std::vector<unsigned> m_comp_trans_data_bits_8B;

public:
  void* get_trans_loc_dict(unsigned res, unsigned index=0);
  void set_trans_loc_dict(void* trans_loc_dict, unsigned res, unsigned index=0);
  void set_trans_comp_data_bits(unsigned comp_bits_size, unsigned res, unsigned index=0);
  void* get_loc_dict(unsigned size);
  void set_loc_dict(unsigned size, void* loc_dict);

private:
  void* m_loc_dict_2B;
  void* m_loc_dict_4B;
  void* m_loc_dict_8B;


private:
  int m_comp_data_bits_2B;
  int m_comp_data_bits_4B;
  int m_comp_data_bits_8B;
  int m_comp_data_bits;		//best compression result

public:
  void set_comp_data_bits(unsigned comp_data_bits, int res=-1);
  unsigned get_comp_data_bits(int res=-1);


private:
  enum COMP_DATA_TYPE m_dsc_comp_data_type;
public:
  void set_dsc_comp_data_type(enum COMP_DATA_TYPE type);
  enum COMP_DATA_TYPE get_dsc_comp_data_type();

private:
  std::vector<unsigned> m_approx_op;
public:
  void set_approx_op(unsigned index, unsigned approx_op);
  unsigned get_approx_op(unsigned index);


//added by kh(071216)
public:
  enum COMP_ALGO_TYPE {
    NO_COMP = -1,
    DSM_COMP,
    LWM_COMP,
  };
  void set_comp_algo_type(enum COMP_ALGO_TYPE algo_type);
  enum COMP_ALGO_TYPE get_comp_algo_type();
  void print_dsc_comp_data_type(enum COMP_DATA_TYPE type);
private:
  enum COMP_ALGO_TYPE m_algo_type;

//added by kh(081016)
private:
  unsigned m_id;
  static unsigned next_mf_request_uid;

public:
  unsigned get_request_uid();


//added by kh(081116)
private:
  unsigned m_pc;
public:
  unsigned get_pc();

private:
  unsigned m_wid;
  unsigned m_sid;
public:
  unsigned get_sid();
  unsigned get_wid();

//added by kh(081516)
private:
  mem_addr_t m_addr;
public:
  void set_access_addr(mem_addr_t addr);
  mem_addr_t get_access_addr();


};

//hpcl_cuda_mem_fetch::hpcl_cuda_mem_fetch() {
//
//  int MAX_DATA_REMAP_TYPE_NO = g_hpcl_comp_config.hpcl_data_remap_function.size();
//  m_trans_data.resize(MAX_DATA_REMAP_TYPE_NO);
//  m_trans_data_type.resize(MAX_DATA_REMAP_TYPE_NO, NO_DATA_TYPE);
//
//}
//
//void hpcl_cuda_mem_fetch::set_remap_op_bits(unsigned index, unsigned remap_op_bits) {
//  m_remap_op_bits[index] = remap_op_bits;
//}
//unsigned hpcl_cuda_mem_fetch::get_remap_op_bits(unsigned index) {
//  return m_remap_op_bits[index];
//}
//
//
//void hpcl_cuda_mem_fetch::set_real_data(std::vector<unsigned char>* data) {
//  m_real_data = *data;
//}
//unsigned hpcl_cuda_mem_fetch::get_real_data_size() {
//  return m_real_data.size();
//}
//
//unsigned hpcl_cuda_mem_fetch::get_trans_data_size(unsigned index) {
//  return m_trans_data[index].size();
//}
//
//std::vector<unsigned char>* hpcl_cuda_mem_fetch::config_remapped_data(unsigned index, unsigned data_size) {
//  assert(m_trans_data.size() > index);
//  m_trans_data[index].clear();
//  m_trans_data[index].resize(data_size, 0);
//  return &m_trans_data[index];
//}
//
//unsigned char hpcl_cuda_mem_fetch::get_real_data(unsigned data_index)
//{
//  assert(m_real_data.size() > data_index);
//  return m_real_data[data_index];
//}
//
//void hpcl_cuda_mem_fetch::set_remapped_data_type(unsigned index, enum COMP_DATA_TYPE data_type)
//{
//  assert(m_trans_data_type.size() > index);
//  m_trans_data_type[index] = data_type;
//  //printf("set_remapped_data_type: mf %u, type %d\n", this->get_request_uid(), data_type);
//}
//
//enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE hpcl_cuda_mem_fetch::get_remapped_data_type(unsigned index)
//{
//  assert(m_trans_data_type.size() > index);
//  return m_trans_data_type[index];
//}
//
//int hpcl_cuda_mem_fetch::get_remapped_data_type_index(enum COMP_DATA_TYPE data_type)
//{
//  int ret = -1;
//  for(unsigned i = 0; i < m_trans_data_type.size(); i++) {
//    if(m_trans_data_type[i] == data_type) {
//      ret = i;
//      break;
//    }
//  }
//  return ret;
//}
//
//void hpcl_cuda_mem_fetch::print_real_data(unsigned space_gap)
//{
//  printf("ORG_DATA");
//  print_data(m_real_data, space_gap);
//  printf("\n");
//}
//
//
//void hpcl_cuda_mem_fetch::print_data_all(unsigned space_gap)
//{
//  printf("ORG_DATA");
//  print_data(m_real_data, space_gap);
//  printf("\n");
//
//  for(int i = 0; i < m_trans_data.size(); i++) {
//
//    if(m_trans_data_type[i] == REMAPPED_DATA_1)		printf("REMAPPED_DATA_1");
//    else if(m_trans_data_type[i] == REMAPPED_DATA_2)	printf("REMAPPED_DATA_2");
//    else if(m_trans_data_type[i] == REMAPPED_DATA_3)	printf("REMAPPED_DATA_3");
//    else if(m_trans_data_type[i] == REMAPPED_DATA_4)	printf("REMAPPED_DATA_4");
//    else if(m_trans_data_type[i] == REMAPPED_DATA_5)	printf("REMAPPED_DATA_5");
//    else if(m_trans_data_type[i] == REMAPPED_DATA_6)	printf("REMAPPED_DATA_6");
//    else {
//      printf("m_trans_data_type[%d] = %d\n", i, m_trans_data_type[i]);
//      assert(0);
//    }
//    print_data(m_trans_data[i], space_gap);
//    printf("\n");
//  }
//}
//
//void hpcl_cuda_mem_fetch::print_data(std::vector<unsigned char>& data, unsigned space_gap)
//{
//  int data_size = data.size();
//  printf(" (%d) = ", data_size);
//  for(int i = 0; i < data_size; i++) {
//    if(i % space_gap == 0) printf(" ");
//    printf("%02x", data[i]);
//  }
//  printf("\n");
//}

#endif



