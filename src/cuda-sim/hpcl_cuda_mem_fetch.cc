/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include <vector>
#include <cassert>
#include <cstdio>

#include "hpcl_cuda_mem_fetch.h"

#include "../option_parser.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

#include "hpcl_cuda_comp_lwm.h"

unsigned hpcl_cuda_mem_fetch::next_mf_request_uid=1;


hpcl_cuda_mem_fetch::hpcl_cuda_mem_fetch(unsigned pc, unsigned sid, unsigned wid) {

  m_id = next_mf_request_uid++;

  int MAX_DATA_REMAP_TYPE_NO = g_hpcl_comp_config.hpcl_data_remap_function.size();
  m_trans_data.resize(MAX_DATA_REMAP_TYPE_NO);
  m_trans_data_type.resize(MAX_DATA_REMAP_TYPE_NO, NO_DATA_TYPE);
  m_remap_op_bits.resize(MAX_DATA_REMAP_TYPE_NO, 0);


  m_trans_loc_dict_2B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
  m_trans_loc_dict_4B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
  m_trans_loc_dict_8B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
  m_comp_trans_data_bits_2B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
  m_comp_trans_data_bits_4B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
  m_comp_trans_data_bits_8B.resize(MAX_DATA_REMAP_TYPE_NO, 0);


  m_loc_dict_2B = NULL;
  m_loc_dict_4B = NULL;
  m_loc_dict_8B = NULL;

  m_comp_data_bits_2B = -1;
  m_comp_data_bits_4B = -1;
  m_comp_data_bits_8B = -1;

  m_approx_op.resize(MAX_DATA_REMAP_TYPE_NO, 0);

  //added by kh(081116)
  m_pc = pc;
  m_sid = sid;
  m_wid = wid;
  ///
}

hpcl_cuda_mem_fetch::~hpcl_cuda_mem_fetch() {

  hpcl_dict<unsigned short>* loc_dict_2B = (hpcl_dict<unsigned short>*) m_loc_dict_2B;
  hpcl_dict<unsigned int>* loc_dict_4B = (hpcl_dict<unsigned int>*) m_loc_dict_4B;
  hpcl_dict<unsigned long long>* loc_dict_8B = (hpcl_dict<unsigned long long>*) m_loc_dict_8B;
  if(loc_dict_2B)	delete loc_dict_2B;
  if(loc_dict_4B)	delete loc_dict_4B;
  if(loc_dict_8B)	delete loc_dict_8B;

  for(int i = 0; i < m_trans_loc_dict_2B.size(); i++) {
    hpcl_dict<unsigned short>* _loc_dict_2B = (hpcl_dict<unsigned short>*) m_trans_loc_dict_2B[i];
    if(_loc_dict_2B)	delete _loc_dict_2B;
  }
  for(int i = 0; i < m_trans_loc_dict_4B.size(); i++) {
    hpcl_dict<unsigned int>* _loc_dict_4B = (hpcl_dict<unsigned int>*) m_trans_loc_dict_4B[i];
    if(_loc_dict_4B)	delete _loc_dict_4B;
  }
  for(int i = 0; i < m_trans_loc_dict_8B.size(); i++) {
    hpcl_dict<unsigned long long>* _loc_dict_8B = (hpcl_dict<unsigned long long>*) m_trans_loc_dict_8B[i];
    if(_loc_dict_8B)	delete _loc_dict_8B;
  }

}








void hpcl_cuda_mem_fetch::set_remap_op_bits(unsigned index, unsigned remap_op_bits) {
  m_remap_op_bits[index] = remap_op_bits;
}
unsigned hpcl_cuda_mem_fetch::get_remap_op_bits(unsigned index) {
  return m_remap_op_bits[index];
}


void hpcl_cuda_mem_fetch::set_real_data(std::vector<unsigned char>* data) {
  m_real_data = *data;
}
unsigned hpcl_cuda_mem_fetch::get_real_data_size() {
  return m_real_data.size();
}

unsigned hpcl_cuda_mem_fetch::get_trans_data_size(unsigned index) {
  return m_trans_data[index].size();
}

std::vector<unsigned char>* hpcl_cuda_mem_fetch::config_remapped_data(unsigned index, unsigned data_size) {
  assert(m_trans_data.size() > index);
  m_trans_data[index].clear();
  m_trans_data[index].resize(data_size, 0);
  return &m_trans_data[index];
}

unsigned char hpcl_cuda_mem_fetch::get_real_data(unsigned data_index)
{
  assert(m_real_data.size() > data_index);
  return m_real_data[data_index];
}

void hpcl_cuda_mem_fetch::set_remapped_data_type(unsigned index, enum COMP_DATA_TYPE data_type)
{
  assert(m_trans_data_type.size() > index);
  m_trans_data_type[index] = data_type;
  //printf("set_remapped_data_type: mf %u, type %d\n", this->get_request_uid(), data_type);
}

enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE hpcl_cuda_mem_fetch::get_remapped_data_type(unsigned index)
{
  assert(m_trans_data_type.size() > index);
  return m_trans_data_type[index];
}

int hpcl_cuda_mem_fetch::get_remapped_data_type_index(enum COMP_DATA_TYPE data_type)
{
  int ret = -1;
  for(unsigned i = 0; i < m_trans_data_type.size(); i++) {
    if(m_trans_data_type[i] == data_type) {
      ret = i;
      break;
    }
  }
  return ret;
}

void hpcl_cuda_mem_fetch::print_real_data(unsigned space_gap)
{
  printf("ORG_DATA");
  print_data(m_real_data, space_gap);
  printf("\n");
}


void hpcl_cuda_mem_fetch::print_data_all(unsigned space_gap)
{
  printf("ORG_DATA");
  print_data(m_real_data, space_gap);
  printf("\n");

  for(int i = 0; i < m_trans_data.size(); i++) {

    if(m_trans_data_type[i] == REMAPPED_DATA_1)		printf("REMAPPED_DATA_1");
    else if(m_trans_data_type[i] == REMAPPED_DATA_2)	printf("REMAPPED_DATA_2");
    else if(m_trans_data_type[i] == REMAPPED_DATA_3)	printf("REMAPPED_DATA_3");
    else if(m_trans_data_type[i] == REMAPPED_DATA_4)	printf("REMAPPED_DATA_4");
    else if(m_trans_data_type[i] == REMAPPED_DATA_5)	printf("REMAPPED_DATA_5");
    else if(m_trans_data_type[i] == REMAPPED_DATA_6)	printf("REMAPPED_DATA_6");
    else {
      printf("m_trans_data_type[%d] = %d\n", i, m_trans_data_type[i]);
      assert(0);
    }
    print_data(m_trans_data[i], space_gap);
    printf("\n");
  }
}

void hpcl_cuda_mem_fetch::print_data(std::vector<unsigned char>& data, unsigned space_gap)
{
  int data_size = data.size();
  printf(" (%d) = ", data_size);
  for(int i = 0; i < data_size; i++) {
    if(i % space_gap == 0) printf(" ");
    printf("%02x", data[i]);
  }
  printf("\n");
}


unsigned hpcl_cuda_mem_fetch::get_trans_data_no()
{
  return m_trans_data.size();
}

std::vector<unsigned char>& hpcl_cuda_mem_fetch::get_real_data_ptr()
{
  return m_real_data;
}

void hpcl_cuda_mem_fetch::copy_real_data(std::vector<unsigned char>& tmp)
{
  tmp = m_real_data;
}


std::vector<unsigned char>& hpcl_cuda_mem_fetch::get_trans_data_ptr(unsigned index)
{
  return m_trans_data[index];
}


void* hpcl_cuda_mem_fetch::get_trans_loc_dict(unsigned res, unsigned index)
{
  if(res == 2)		return m_trans_loc_dict_2B[index];
  else if(res == 4)	return m_trans_loc_dict_4B[index];
  else if(res == 8)	return m_trans_loc_dict_8B[index];
  else	assert(0);

  return NULL;
}
void hpcl_cuda_mem_fetch::set_trans_loc_dict(void* trans_loc_dict, unsigned res, unsigned index)	{
  if(res == 2) 		m_trans_loc_dict_2B[index] = trans_loc_dict;
  else if(res == 4) 	m_trans_loc_dict_4B[index] = trans_loc_dict;
  else if(res == 8) 	m_trans_loc_dict_8B[index] = trans_loc_dict;
  else	assert(0);
}

void hpcl_cuda_mem_fetch::set_trans_comp_data_bits(unsigned comp_bits_size, unsigned res, unsigned index) {
  if(res == 2)		m_comp_trans_data_bits_2B[index] = comp_bits_size;
  else if(res == 4)	m_comp_trans_data_bits_4B[index] = comp_bits_size;
  else if(res == 8)	m_comp_trans_data_bits_8B[index] = comp_bits_size;
  else	assert(0);
}

void* hpcl_cuda_mem_fetch::get_loc_dict(unsigned size) {
  if(size == sizeof(unsigned short))	return m_loc_dict_2B;
  else if(size == sizeof(unsigned int))	return m_loc_dict_4B;
  else if(size == sizeof(unsigned long long))	return m_loc_dict_8B;
  else					assert(0);
  return NULL;
}

void hpcl_cuda_mem_fetch::set_loc_dict(unsigned size, void* loc_dict) {
  if(size == sizeof(unsigned short))	m_loc_dict_2B = loc_dict;
  else if(size == sizeof(unsigned int))	m_loc_dict_4B = loc_dict;
  else if(size == sizeof(unsigned long long))	m_loc_dict_8B = loc_dict;
  else					assert(0);
}

void hpcl_cuda_mem_fetch::set_comp_data_bits(unsigned comp_data_bits, int res) {
  if(res == -1) {
    m_comp_data_bits = comp_data_bits;
  } else if(res == 2) {
    m_comp_data_bits_2B = comp_data_bits;
  } else if(res == 4) {
    m_comp_data_bits_4B = comp_data_bits;
  } else if(res == 8) {
    m_comp_data_bits_8B = comp_data_bits;
  } else assert(0);
}
unsigned hpcl_cuda_mem_fetch::get_comp_data_bits(int res) {
  //std::cout << "mf " << this->get_request_uid() << " " << this->get_is_write() << " " << this->get_access_type() << std::endl;
  if(res == -1) {
    assert(m_comp_data_bits > 0);
    return (unsigned) m_comp_data_bits;
  }
  else {
    if(res == 2) {
      assert(m_comp_data_bits_2B > 0);
      return (unsigned) m_comp_data_bits_2B;
    }
    else if(res == 4) {
      assert(m_comp_data_bits_4B > 0);
      return m_comp_data_bits_4B;
    }
    else if(res == 8) {
      assert(m_comp_data_bits_8B > 0);
      return m_comp_data_bits_8B;
    }
    else assert(0);
  }
  assert(0);
  return 0;
}

void hpcl_cuda_mem_fetch::set_dsc_comp_data_type(enum COMP_DATA_TYPE type) {
  m_dsc_comp_data_type = type;
}
enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE hpcl_cuda_mem_fetch::get_dsc_comp_data_type() {
  return m_dsc_comp_data_type;
}

//added by kh(072816)
void hpcl_cuda_mem_fetch::set_approx_op(unsigned index, unsigned approx_op)
{
  m_approx_op[index] = approx_op;
}
unsigned hpcl_cuda_mem_fetch::get_approx_op(unsigned index)
{
  return m_approx_op[index];
}
///

void hpcl_cuda_mem_fetch::set_comp_algo_type(enum COMP_ALGO_TYPE algo_type)
{
  m_algo_type = algo_type;
}
enum hpcl_cuda_mem_fetch::COMP_ALGO_TYPE hpcl_cuda_mem_fetch::get_comp_algo_type()
{
  return m_algo_type;
}
void hpcl_cuda_mem_fetch::print_dsc_comp_data_type(enum COMP_DATA_TYPE type) {
  if(type == ORG_DATA)	printf("DataType : ORG_DATA\n");
  else			printf("DataType : REMAPPED_DATA_%d\n", type);
}


unsigned hpcl_cuda_mem_fetch::get_request_uid() {
  return m_id;
}


unsigned hpcl_cuda_mem_fetch::get_pc() {
  return m_pc;
}

unsigned hpcl_cuda_mem_fetch::get_sid() {
  return m_sid;
}
unsigned hpcl_cuda_mem_fetch::get_wid() {
  return m_wid;
}


void hpcl_cuda_mem_fetch::set_access_addr(mem_addr_t addr) {
  m_addr = addr;
}
mem_addr_t hpcl_cuda_mem_fetch::get_access_addr() {
  return m_addr;
}






