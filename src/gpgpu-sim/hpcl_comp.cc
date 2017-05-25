/*
 * hpcl_comp.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include "hpcl_comp.h"
#include "../abstract_hardware_model.h"
#include <cassert>

#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

#include <iostream>
#include <string>
#include <stdio.h>


extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;


//added by kh(091916)
#include "../gpgpu-sim/hpcl_approx_mem_space.h"
extern hpcl_approx_mem_space* g_hpcl_approx_mem_space;
///



//#define DATA_OP_DEBUG 1

hpcl_comp::hpcl_comp ()
{
  // TODO Auto-generated constructor stub

}

hpcl_comp::~hpcl_comp ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_comp::create(const struct memory_config * memory_config, class memory_space * global_mem)
{
  m_memory_config = memory_config;
  m_global_mem = global_mem;


  m_symbol_table.clear();


  std::cout << "g_hpcl_comp_config.hpcl_char_symbol_tbl " << g_hpcl_comp_config.hpcl_char_symbol_tbl << std::endl;

  if(g_hpcl_comp_config.hpcl_char_symbol_tbl == 0) {
	  //table1, SM
	  //symbol table
	  m_symbol_table[0x00] = 0x60;
	  m_symbol_table[0x0A] = 0x7B;
	  m_symbol_table[0x0D] = 0x7C;
	  m_symbol_table[0x20] = 0x7D;
	  m_symbol_table[0x2C] = 0x7E;
	  m_symbol_table[0x2E] = 0x7F;

	  m_symbol_table[0x60] = 0x00;
	  m_symbol_table[0x7B] = 0x0A;
	  m_symbol_table[0x7C] = 0x0D;
	  m_symbol_table[0x7D] = 0x20;
	  m_symbol_table[0x7E] = 0x2C;
	  m_symbol_table[0x7F] = 0x2E;
	  ///
  } else if(g_hpcl_comp_config.hpcl_char_symbol_tbl == 1) {

	  //table2, MUM
	  //symbol table
	  m_symbol_table[0x00] = 0x5B;
	  m_symbol_table[0x71] = 0x5C;

	  m_symbol_table[0x5B] = 0x00;
	  m_symbol_table[0x5C] = 0x71;
	  ///

  } else if(g_hpcl_comp_config.hpcl_char_symbol_tbl == 2) {

	  //table3, II
	  //0x22 -> ", 0x3D -> =, 0x3C -> <, 0x3E -> >, 0x2E -> . , 0x20 -> Space
	  m_symbol_table[0x20] = 0x60;
	  m_symbol_table[0x22] = 0x7B;
	  m_symbol_table[0x3D] = 0x7C;
	  m_symbol_table[0x3C] = 0x7D;
	  m_symbol_table[0x3E] = 0x7E;
	  m_symbol_table[0x2E] = 0x7F;

	  m_symbol_table[0xA0] = 0x5B;
	  m_symbol_table[0xC2] = 0x5C;



	  m_symbol_table[0x60] = 0x20;
	  m_symbol_table[0x7B] = 0x22;
	  m_symbol_table[0x7C] = 0x3D;
	  m_symbol_table[0x7D] = 0x3C;
	  m_symbol_table[0x7E] = 0x3E;
	  m_symbol_table[0x7F] = 0x2E;


	  m_symbol_table[0x5B] = 0xA0;
	  m_symbol_table[0x5C] = 0xC2;

	  ///

  } else if(g_hpcl_comp_config.hpcl_char_symbol_tbl == 3) {
		
	  //minimal table
	  m_symbol_table[0x00] = 0x60;
	  m_symbol_table[0x09] = 0x7B;
	  m_symbol_table[0x0A] = 0x7C;
	  m_symbol_table[0x0D] = 0x7D;
	  m_symbol_table[0x20] = 0x7F;
	  m_symbol_table[0x60] = 0x00;
	  m_symbol_table[0x7B] = 0x09;
	  m_symbol_table[0x7C] = 0x0A;
	  m_symbol_table[0x7D] = 0x0D;
	  m_symbol_table[0x7F] = 0x20;

  }

}

bool hpcl_comp::is_data_retrievable(mem_fetch* mf)
{
  if(mf->get_is_write() == false) {
    if(mf->get_access_type() == GLOBAL_ACC_R
    || mf->get_access_type() == TEXTURE_ACC_R)
//deleted by kh(092416)
//let's restrict ours to global memory access
/*
    || mf->get_access_type() == LOCAL_ACC_R
    || mf->get_access_type() == CONST_ACC_R
    || mf->get_access_type() == TEXTURE_ACC_R)
*/
    {
      return true;
    }
  }
  //added by kh(081016)
  else {
    /*
    if(mf->get_access_type() == GLOBAL_ACC_W)
      return true;
    */
  }
  ///

  return false;
}



void hpcl_comp::get_cache_data(void* _mf, void* data, int index)
{
  mem_fetch* mf = (mem_fetch*)_mf;
  new_addr_type access_addr = mf->get_addr();
  new_addr_type blk_addr = m_memory_config->m_L2_config.block_addr(access_addr);

  assert(data != 0);

  if(is_data_retrievable(mf) == true) {

    if(index == -1) {
      for(unsigned i = 0; i < mf->get_data_size(); i++) m_global_mem->read((mem_addr_t)blk_addr+i, 1, data+i);
    } else {
      assert(index*32+32 <= mf->get_data_size());
      int start_addr = (mem_addr_t)blk_addr + index*32;
      for(unsigned i = 0; i < 32; i++)	m_global_mem->read(start_addr+i, 1, data+i);
    }

//    printf("HPCL_COMP | access_addr = 0x%08x, blk_addr = 0x%08x, access_type = %u\n", access_addr, blk_addr, mf->get_access_type());
//    printf("HPCL_COMP | Org_Data (%u) = ", mf->get_data_size());
//    for(int i = 0; i < mf->get_data_size(); i++) {
//      printf("%02x ", *((unsigned char*)(data+i)));
//    }
//    printf("\n");

  }



//printf("LD - PC: %u, Type: %d, Mem_Addr: %016llx, Data[0]: %02x\n", mf->get_pc(), mf->get_access_type(), blk_addr, *((unsigned char*)(data)));
//  } else {
//    //printf("LD - PC: %u, Type: %d, Mem_Addr: %016llx\n", mf->get_pc(), mf->get_access_type(), blk_addr);
//  }

}

void hpcl_comp::print_cache_data(mem_fetch* mf)
{
  return;
/*
  new_addr_type access_addr = mf->get_addr();
  new_addr_type blk_addr = m_memory_config->m_L2_config.block_addr(access_addr);
  unsigned char* cache_data = NULL;
  if(mf->get_access_type() == GLOBAL_ACC_R) {
      cache_data = (unsigned char*)malloc(128*sizeof(unsigned char));
      for(unsigned i = 0; i < 128; i++)	m_global_mem->read((mem_addr_t)blk_addr+i, 1, cache_data+i);

      //printf("gm_addr = 0x%0x\n", blk_addr);
      printf("gm_addr = %0x\n", blk_addr);
      //printf("gm_data = 0x");
      printf("gm_data = ");
      for(int i = 127; i >= 0; i--)	printf("%02x", *(cache_data+i));
      printf("\n");
  }

  if(cache_data) free(cache_data);
*/
}

void hpcl_comp::remap_data_to_type0(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data)
{
  mem_fetch* mf = (mem_fetch*)data;

  unsigned data_size = mf->get_real_data_size();
  remapped_data->resize(data_size, 0);
  //mf->config_trans_data(data_size, type_index);

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Org_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");
  #endif

  //printf("haha1\n");
  //unsigned trans_res = 4;
  //added by kh(071316)
  assert(data_size%remap_res == 0);
  std::vector< std::vector<unsigned char> > bins;
  for(unsigned i = 0; i < remap_res; i++) {
    std::vector<unsigned char> tmp;
    //tmp.resize(data_size/trans_res,0);
    bins.push_back(tmp);
  }

  //printf("haha2\n");
  unsigned index = 0;
  for(unsigned i = 0; i < data_size; i++) {
    index = i%remap_res;
    bins[index].push_back(mf->get_real_data(i));
  }

  //printf("haha3\n");
  unsigned ptr = 0;
  //for(unsigned i = 0; i < bins.size(); i++) {
  for(int i = bins.size()-1; i >= 0; i--) {
    //printf("bins[i].size() %u\n", bins[i].size());
    for(unsigned j = 0; j < bins[i].size(); j++) {
      //printf("ptr %u, bins[i][j] 0x%2x\n", ptr, bins[i][j]);
      //mf->set_trans_data(ptr, bins[i][j], type_index);
      remapped_data->at(ptr) = bins[i][j];
      ptr++;
    }
  }

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Trans_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_trans_data(i));
  }
  printf("\n");
  #endif
}
/*
void hpcl_comp::check_value_similarity(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type0)
{
  mem_fetch* mf = (mem_fetch*) data;
  unsigned data_size = mf->get_real_data_size();
  unsigned group_size = data_size/remap_res;

  assert(remapped_data_type0->size() > group_size);
  unsigned char base_byte_val = remapped_data_type0->at(0);
  bool is_similar = true;
  bool has_diff1 = false;
  for(int i = 1; i < group_size; i++) {
    unsigned char byte_val = remapped_data_type0->at(i);
    unsigned diff = abs(byte_val-base_byte_val);
    if(diff > 1) {
      is_similar = false;
      break;
    } else {
      if(diff == 1) has_diff1 = true;
    }
  }
}
*/

int hpcl_comp::check_csn_pattern(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size)
{
  //Step1: CSN-checking.
  bool is_CSN_pattern = true;
  unsigned char high_nib = remapped_data_type[index] & 0x0f;
  unsigned char low_nib = (remapped_data_type[index] >> 4) & 0x0f;
  if(high_nib == low_nib) {
    //printf("\thigh_nib: %02x low_nib: %02x\n", high_nib, low_nib);
    for(int j = 1; j < subgroup_size; j++) {
      unsigned char hn = remapped_data_type[index+j] & 0x0f;
      unsigned char ln = (remapped_data_type[index+j] >> 4) & 0x0f;
      //printf("\thn: %02x ln: %02x\n", hn, ln);
      if(hn != high_nib || ln != high_nib) {
	  is_CSN_pattern = false;
	  break;
      }
    }
  } else {
    is_CSN_pattern = false;
  }

  ///
  if(is_CSN_pattern == true) {
    //printf("csn_segment: yes\n");
    return 1;
  } else {
    //printf("csn_segment: no\n");
    return 0;
  }
}

int hpcl_comp::complement_data(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size)
{
  //Step2: If subgroup is not CSN pattern, complement nibbles with more than 7
  int complement_use = 0;

  bool is_all_nibble_larger_than_7 = true;
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    if((byte_val & 0x0f) < 0x07)	{
      is_all_nibble_larger_than_7 = false;
      break;
    }

    if(((byte_val >> 4) & 0x0f) < 0x07)	{
      is_all_nibble_larger_than_7 = false;
      break;
    }
  }

  if(is_all_nibble_larger_than_7 == true) {
    complement_use = 0;
  } else {
    for(int j = 0; j < subgroup_size; j++) {
      unsigned char byte_val = remapped_data_type[index+j];
      unsigned char new_byte_val = 0;

      //printf("byte_val1 0x%02x, byte_val2 0x%02x comp %d\n", byte_val & 0x0f, 0x07, ((byte_val & 0x0f) > 0x07));
      if((byte_val & 0x0f) > 0x07) {
	new_byte_val = (~byte_val & 0x0f);
      } else {
	new_byte_val = (byte_val & 0x0f);
      }
      byte_val = byte_val >> 4;
      if((byte_val & 0x0f) > 0x07) {
	new_byte_val = new_byte_val | ((~byte_val & 0x0f) << 4);
      } else {
	new_byte_val = new_byte_val | ((byte_val & 0x0f) << 4 );
      }

      //printf("byte_val 0x%02x, new_byte_val 0x%02x\n", remapped_data_type1_1[index+j], new_byte_val);
      if(remapped_data_type[index+j] != new_byte_val) {
	remapped_data_type[index+j] = new_byte_val;
	complement_use = 1;
      }
    }
  }

  return complement_use;
}

int hpcl_comp::check_float_number(std::vector<unsigned char>& remapped_data_type, int group_size)
{
  //check if the first group (32B for 128B cache block) has CSN pattern
  int is_csn_segment1 = check_csn_pattern(remapped_data_type, 0, group_size/2);
  int is_csn_segment2 = check_csn_pattern(remapped_data_type, group_size/2, group_size/2);

  unsigned char byte_val1 = remapped_data_type[0] & 0x0f;
  unsigned char byte_val2 = remapped_data_type[group_size/2] & 0x0f;

  if(is_csn_segment1 == 1 && is_csn_segment2 == 1 && byte_val1 != 0) {
    if(byte_val1 != byte_val2)	return 1;
    else			return 2;
  } else {
    return 0;
  }
}

int hpcl_comp::compute_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, int& out_max_delta_val, unsigned char& out_min_nib_val)
{
  //Step3: If subgroup is not CSN pattern, compute delta value
  //Compute delta value
  int delta_val_use = 1;
  int MAX_DELTA = 3;

  //Find a minimum value
  unsigned char min_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char min_tmp_nib_val = (remapped_data_type[index] >> 4 & 0x0f);
  if(min_nib_val > min_tmp_nib_val) min_nib_val = min_tmp_nib_val;
  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char high_nib = byte_val & 0x0f;
    unsigned char low_nib = (byte_val >> 4) & 0x0f;
    if(high_nib < min_nib_val) {
      min_nib_val = high_nib;
    }
    if(low_nib < min_nib_val) {
      min_nib_val = low_nib;
    }
  }

  int max_delta_value = -1;
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char high_nib = (remapped_data_type[index+j] & 0x0f);
    unsigned char low_nib = ((remapped_data_type[index+j] >> 4) & 0x0f);

    int delta_val1 = high_nib-min_nib_val;
    int delta_val2 = low_nib-min_nib_val;
    assert(delta_val1 >= 0);
    assert(delta_val2 >= 0);
    if(delta_val1 > MAX_DELTA || delta_val2 > MAX_DELTA) {
      delta_val_use = 0;
      max_delta_value = (delta_val1 > delta_val2)? delta_val1 : delta_val2;
    } else {
      if(max_delta_value < delta_val1)	max_delta_value = delta_val1;
      if(max_delta_value < delta_val2)	max_delta_value = delta_val2;
    }
  }
  if(delta_val_use == 1) {
    if(max_delta_value == 0) {	//all nibbles are same to each other.
      delta_val_use = 0;
    }
  }
  out_max_delta_val = max_delta_value;
  out_min_nib_val = min_nib_val;


  /*
  //There is no CSN pattern in the complement-only mode
  if(delta_val_use == 0 && complement_use == 1 && max_delta_value > 3) {
    complement_use = 0;
  }
  */
  ///
  return delta_val_use;
}

int hpcl_comp::compute_masking(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& out_max_nib_val)
{
  //Step4: If subgroup is not CSN pattern, check masking
  int masking_use = 1;
  unsigned char max_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char max_tmp_nib_val = ((remapped_data_type[index] >> 4) & 0x0f);
  if(max_nib_val < max_tmp_nib_val) max_nib_val = max_tmp_nib_val;
  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char low_nib = byte_val & 0x0f;
    unsigned char high_nib = (byte_val >> 4) & 0x0f;
    if(high_nib > max_nib_val) {
      max_nib_val = high_nib;
    }
    if(low_nib > max_nib_val) {
      max_nib_val = low_nib;
    }
  }
  //printf("max_nib_val 0x%02x\n", max_nib_val);
  for(int j = 0; j < subgroup_size; j++) {
    unsigned char low_nib = (remapped_data_type[index+j] & 0x0f);
    unsigned char high_nib = (remapped_data_type[index+j] >> 4) & 0x0f;

    if(low_nib != 0 && low_nib != max_nib_val) {
      masking_use = 0;
      //printf("max_nib_val 0x%02x, low_nib \n", max_nib_val);
      break;
    }
    if(high_nib != 0 && high_nib != max_nib_val) {
      masking_use = 0;
      break;
    }
  }

  out_max_nib_val = max_nib_val;


  return masking_use;
}

int hpcl_comp::compute_neighbor_delta(std::vector<unsigned char>& remapped_data_type, int index, int subgroup_size, unsigned char& first_nib_val)
{
  //Step5: If subgroup is not CSN pattern, check constant neighboring delta
  int neighbor_const_delta_use = 1;
  unsigned char low_nib_val = (remapped_data_type[index] & 0x0f);
  unsigned char high_nib_val = ((remapped_data_type[index] >> 4) & 0x0f);
  unsigned char diff = (high_nib_val-low_nib_val)&0x0f;
  unsigned char last_nib_val = low_nib_val;

  for(int j = 1; j < subgroup_size; j++) {
    unsigned char byte_val = remapped_data_type[index+j];
    unsigned char low_nib = byte_val & 0x0f;
    unsigned char high_nib = (byte_val >> 4) & 0x0f;

    if(diff != ((last_nib_val - high_nib)&0x0f)) {
      neighbor_const_delta_use = 0;
      break;
    } else {
      last_nib_val = high_nib;
    }

    if(diff != ((last_nib_val - low_nib)&0x0f)) {
      neighbor_const_delta_use = 0;
      break;
    } else {
      last_nib_val = low_nib;
    }
  }

  first_nib_val = low_nib_val;

  return neighbor_const_delta_use;
}

void hpcl_comp::remap_data_to_type1(void* data, unsigned remap_res, std::vector<unsigned char>* remapped_data_type1, unsigned& added_bits_for_remap, std::vector<enum mem_fetch::REMAP_OP_TYPE>& remap_op_types, unsigned& approx_op, unsigned& approx_except)
{
  mem_fetch* mf = (mem_fetch*) data;
  unsigned data_size = mf->get_trans_data_size();

  #ifdef DATA_REMAP_DEBUG
  printf("----- MF %u's data -----\n", mf->get_request_uid());
  printf("Original Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");
  #endif

  //mf->config_trans_data(data_size, type_index);
  remapped_data_type1->resize(data_size, 0);

  std::vector<unsigned char> remapped_data_type0;
  remapped_data_type0.resize(data_size, 0);
  remap_data_to_type0(data, remap_res, &remapped_data_type0);

  #ifdef DATA_REMAP_DEBUG
  printf("Remapped Data (type 0) Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type0[i]);
  }
  printf("\n");
  #endif

  //added by kh(071316)
  unsigned remap_unit_size = data_size / remap_res;

  if(g_hpcl_comp_config.hpcl_data_remap_level == 0) {	//nibble-level remapping

    unsigned data_seg_no = data_size/remap_unit_size;
    for(unsigned i = 0; i < data_seg_no; i++) {
      //DATA_REMAP_DEBUG_PRINT("data segment %u\n", i);
      for(unsigned j = 0; j < (remap_unit_size*2); j=j+2) {
	unsigned index1 = i * remap_unit_size + j % remap_unit_size;
	unsigned index2 = i * remap_unit_size + (j+1) % remap_unit_size;
	//DATA_REMAP_DEBUG_PRINT("\tindex1 %u index2 %u, ", index1, index2);
	//unsigned char trans_data1 = mf->get_trans_data(index1, src_type_index);
	//unsigned char trans_data2 = mf->get_trans_data(index2, src_type_index);
	unsigned char trans_data1 = remapped_data_type0[index1];
	unsigned char trans_data2 = remapped_data_type0[index2];

	if(j < remap_unit_size) {
	  //DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	  trans_data1 = (trans_data1 & 0xf0)>>4;
	  trans_data2 = (trans_data2 & 0xf0);
	  trans_data1 = (trans_data1 | trans_data2);
	  //DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	  //mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
	  remapped_data_type1->at(i*remap_unit_size+j/2) = trans_data1;
	} else {
	  //DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	  trans_data1 = (trans_data1 & 0x0f);
	  trans_data2 = (trans_data2 & 0x0f)<<4;
	  trans_data1 = (trans_data1 | trans_data2);
	  //DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	  //mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
	  remapped_data_type1->at(i*remap_unit_size+j/2) = trans_data1;
	}
      }
    }

  } else if(g_hpcl_comp_config.hpcl_data_remap_level == 1) {	//bit-level remapping

    //Bit-level remapping
    unsigned data_seg_no = data_size/remap_unit_size;
    for(unsigned i = 0; i < data_seg_no; i++) {
      //DATA_REMAP_DEBUG_PRINT("data segment %u\n", i);
      for(unsigned j = 0; j < (remap_unit_size*8); j=j+8) {

	int bit_pos = 7 - j / remap_unit_size;
	int index = i * remap_unit_size + j % remap_unit_size;
	unsigned char remapped_byte = 0;
	for(unsigned k = 0; k < 8; k++) {
	  unsigned byte_index = index + k;
	  unsigned char bit_value = ((remapped_data_type0[byte_index] >> bit_pos) & 0x01);
	  unsigned char prev_remapped_byte = remapped_byte;
	  remapped_byte = (remapped_byte << 1) | bit_value;
	  //DATA_REMAP_DEBUG_PRINT("\tbit_pos %d index %u prev_remapped_byte %02x, remapped_byte %02x bit_value %2x\n", bit_pos, byte_index, prev_remapped_byte, remapped_byte, bit_value);
	}

	//DATA_REMAP_DEBUG_PRINT("\n");
	remapped_data_type1->at(i*remap_unit_size+j/8) = remapped_byte;
      }
    }

  }

  #ifdef DATA_REMAP_DEBUG
  printf("Remapped Data (type 1)  Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type1->at(i));
  }
  printf("\n");
  #endif


  //added by kh(072216)
  //reset remap_op_types
  remap_op_types.clear();

  //added by kh(080416)
  //remap_op_loc.clear();



  if(g_hpcl_comp_config.hpcl_data_remap_op_en == 1)
  {


  unsigned group_size = data_size/remap_res;
  //unsigned subgroup_size = group_size/2;
  unsigned subgroup_size = 8;


  std::vector<unsigned char> remapped_data_type1_1 = *remapped_data_type1;
  std::vector<unsigned char> remapped_data_type1_2 = *remapped_data_type1;
  std::vector<unsigned char> remapped_data_type1_3 = *remapped_data_type1;
  int flipped_subgroup_no = 0;


  added_bits_for_remap = data_size/subgroup_size;	//to indicate whether or not subgroup is modified

  if(g_hpcl_comp_config.hpcl_data_remap_op_range == 1) {
    added_bits_for_remap = group_size/subgroup_size;
    assert(remap_res == 4);
  }

  for(int i = 0; i < data_size/subgroup_size; i++) {

      if(g_hpcl_comp_config.hpcl_data_remap_op_range == 1) {
	if(i >= group_size/subgroup_size)	break;
      }

      int index = i*subgroup_size;

      int csn_pattern = check_csn_pattern(remapped_data_type1_1, index, subgroup_size);
      if(csn_pattern == 1)	continue;

      //The complemented data is stored in remapped_data_type1_1.
      int complement_use = complement_data(remapped_data_type1_1, index, subgroup_size);
      unsigned char min_nib_val = 0;
      int max_delta_value = 0;
      int delta_val_use = compute_delta(remapped_data_type1_1, index, subgroup_size, max_delta_value, min_nib_val);

      //There is no CSN pattern in the complement-only mode
      if(delta_val_use == 0 && complement_use == 1 && max_delta_value > 3) {
	complement_use = 0;
      }
      unsigned char max_nib_val = 0;
      int masking_use = compute_masking(remapped_data_type1_2, index, subgroup_size, max_nib_val);
      unsigned char low_nib_val = 0;
      int neighbor_const_delta_use = compute_neighbor_delta(remapped_data_type1_3, index, subgroup_size, low_nib_val);

      int op_bits = 2;

      if(masking_use == 1) {

	added_bits_for_remap += ((subgroup_size*2)+op_bits+2);	//2: op, 2: # of ops

	unsigned char val = max_nib_val | (max_nib_val << 4);

	#ifdef DATA_OP_DEBUG
	printf("%d | masking use is applied\n", i);
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif

	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;

	#ifdef DATA_OP_DEBUG
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif

	remap_op_types.push_back(mem_fetch::MASK_OP);
	//remap_op_loc.push_back(i);

      }
      #ifdef old  //don't use this due to minimal impact
      else if(neighbor_const_delta_use == 1) {

	added_bits_for_remap += (4+op_bits+2);	//4: delta value, 2: op, 2: # of ops

	/*
	printf("neighbor_const_delta_use is applied\n");
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	*/

	unsigned char val = low_nib_val | (low_nib_val << 4);

	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;
	remap_op_types.push_back(mem_fetch::NEIGHBOR_CONST_DELTA_OP);

	/*
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");

	assert(0);
	*/
      }
      #endif
      else if(complement_use == 1 && delta_val_use == 0) {
	added_bits_for_remap += ((subgroup_size*2)+op_bits+2);

	#ifdef DATA_OP_DEBUG
	printf("%d | complement use only is applied\n", i);
	printf("max_delta_value %d\n", max_delta_value);

	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif

	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = remapped_data_type1_1[index+j];

	remap_op_types.push_back(mem_fetch::COMP_OP);
	//remap_op_loc.push_back(i);

	#ifdef DATA_OP_DEBUG
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	assert(0);
	#endif

      }
      else if(complement_use == 0 && delta_val_use == 1) {
	//added by kh(072616)
	if(max_delta_value == 1) {
	  added_bits_for_remap += ((subgroup_size*2)+op_bits+1+2);	//1: max_delta_value indicator
	  remap_op_types.push_back(mem_fetch::DELTA_UPTO_1_OP);
	} else if(max_delta_value > 1) {
	  added_bits_for_remap += ((subgroup_size*2)*2+op_bits+1+2);	//1: max_delta_value indicator
	  remap_op_types.push_back(mem_fetch::DELTA_UPTO_3_OP);
	}
	///
	//remap_op_loc.push_back(i);


	#ifdef DATA_OP_DEBUG
	printf("%d | delta val use only is applied\n", i);
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif

	unsigned char val = min_nib_val | (min_nib_val << 4);
	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;

	#ifdef DATA_OP_DEBUG
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif

      } else if(complement_use == 1 && delta_val_use == 1) {


	if(max_delta_value == 1) {
	  added_bits_for_remap += ((subgroup_size*2)*2+op_bits+1+2);	//1: max_delta_value indicator
	  remap_op_types.push_back(mem_fetch::COMP_DELTA_UPTO_1_OP);
	} else if(max_delta_value > 1) {
	  added_bits_for_remap += ((subgroup_size*2)*3+op_bits+1+2);	//1: max_delta_value indicator
	  remap_op_types.push_back(mem_fetch::COMP_DELTA_UPTO_3_OP);
	}
	//remap_op_loc.push_back(i);

	//if(max_delta_value > 1) {
	#ifdef DATA_OP_DEBUG
	printf("%d | both complement and delta val use is applied\n", i);
	printf("Before - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif
	//}

	unsigned char val = min_nib_val | (min_nib_val << 4);
	for(int j = 0; j < subgroup_size; j++)	remapped_data_type1->at(index+j) = val;

	//if(max_delta_value > 1) {
	#ifdef DATA_OP_DEBUG
	printf("After - \n");
	for(int j = 0; j < subgroup_size; j++)	printf("%02x ", remapped_data_type1->at(index+j));
	printf("\n");
	#endif
	//}

      }
      else {
	 //no operation is possible
      }
  }
  //}

  #ifdef DATA_REMAP_DEBUG
  printf("New Remapped Data (type 1)  Remap Res %u - ", remap_res);
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", remapped_data_type1->at(i));
  }
  printf("\n");
  #endif

  }


  //unsigned group_size = data_size/remap_res;
  approx_op = 0;	//reset approx_op
  approx_except = 0;	//reset approx_except
  if(g_hpcl_comp_config.hpcl_approx_en == 1 && g_hpcl_comp_config.hpcl_float_approx_range > 0)
  {
    if(g_hpcl_comp_config.hpcl_float_det_type == hpcl_comp_config::HEURISTIC)
    {
      std::vector<unsigned char>& approx_remapped_data_type1 = *remapped_data_type1;
      unsigned group_size = approx_remapped_data_type1.size()/remap_res;
      int is_float_num = check_float_number(approx_remapped_data_type1, group_size);
      if(is_float_num == 1) {

	printf("Float number is checked\n");
	//for(int i = 0; i < group_size; i++) {
	for(int i = 0; i < approx_remapped_data_type1.size(); i++) {
	  printf("%02x", approx_remapped_data_type1[i]);
	  if(i == (group_size-1))	printf(" / ");
	}
	printf("\n");

	for(int i = 1; i <= g_hpcl_comp_config.hpcl_float_approx_range; i++) {
	  int index = approx_remapped_data_type1.size() - (i * group_size/2);
	  bool is_subgroup_zeros = true;
	  for(int j = 0; j < group_size/2; j++) {
	    if(approx_remapped_data_type1[index+j] != 0) {
	      is_subgroup_zeros = false;
	      break;
	    }
	  }
	  if(is_subgroup_zeros == false) {

	    /*
	    printf("Before Approx..\n");
	    for(int j = 0; j < group_size/2; j++)	printf("%02x", approx_remapped_data_type1[index+j]);
	    printf("\n");
	    */

	    for(int j = 0; j < group_size/2; j++)	approx_remapped_data_type1[index+j] = 0;

	    /*
	    printf("After Approx..\n");
	    for(int j = 0; j < group_size/2; j++) printf("%02x", approx_remapped_data_type1[index+j]);
	    printf("\n");
	    */

	    approx_op = 1;	//set approx_op

	  } else {
	    //printf("subgroup zero!!!\n");
	  }
	}

      } else if(is_float_num == 2) {

	/*
	printf("Is this Float number? mf %u\n", mf->get_request_uid());
	for(int i = 0; i < approx_remapped_data_type1.size(); i++) {
	  if(i == group_size)	printf(" / ");
	  printf("%02x", approx_remapped_data_type1[i]);
	}
	printf("\n");

	printf("After the first nibble-level remapping\n");
	mf->print_data(remapped_data_type0, 4);
	///
	*/

	//mf->print_data_all(4);
	approx_except = 1;
      }
    } else if(g_hpcl_comp_config.hpcl_float_det_type == hpcl_comp_config::MEMSPACE) {

	//mem_addr_t addr = mf->get_access_addr();
	unsigned addr = (unsigned) mf->get_addr();

	if(g_hpcl_approx_mem_space->is_in_approximable_mem_space(addr) == true) {

	  std::vector<unsigned char>& approx_remapped_data_type1 = *remapped_data_type1;
	  unsigned approx_group_size = approx_remapped_data_type1.size()/remap_res/4;
	  /*
	  printf("Before \n");
	  for(int j = 0; j < approx_remapped_data_type1.size(); j++) {
	      printf("%02x", approx_remapped_data_type1[j]);
	      if(j != 0 && j % approx_group_size == (approx_group_size-1)) printf(" ");
	  }
	  printf("\n");
	  */
	  for(int i = 2; i <= g_hpcl_comp_config.hpcl_float_approx_range; i = i + 2) {
	    int index = approx_remapped_data_type1.size() - (i/2 * approx_group_size);

	    /*
	    printf("\tindex %d\n", index);

	    printf("\tBefore Approx..\n");
	    printf("\t");
	    for(int j = 0; j < approx_group_size; j++)	printf("%02x", approx_remapped_data_type1[index+j]);
	    printf("\n");
	    */

	    for(int j = 0; j < approx_group_size; j++) {
	      approx_remapped_data_type1[index+j] = 0;
	    }

	    /*
	    printf("\tAfter Approx..\n");
	    printf("\t");
	    for(int j = 0; j < approx_group_size; j++) printf("%02x", approx_remapped_data_type1[index+j]);
	    printf("\n");
	    */

	  }

	  /*
	  printf("After \n");
	  for(int j = 0; j < approx_remapped_data_type1.size(); j++) {
	      printf("%02x", approx_remapped_data_type1[j]);
	      if(j != 0 && j % approx_group_size == (approx_group_size-1)) printf(" ");
	  }
	  printf("\n");
	  */

	  approx_op = 1;	//set approx_op
	  //assert(0);
	}




	  #ifdef old
   	  std::vector<unsigned char>& approx_remapped_data_type1 = *remapped_data_type1;
	  unsigned group_size = approx_remapped_data_type1.size()/remap_res;
	  for(int i = 1; i <= g_hpcl_comp_config.hpcl_float_approx_range; i++) {
	    int index = approx_remapped_data_type1.size() - (i * group_size/2);
	    bool is_subgroup_zeros = true;
	    for(int j = 0; j < group_size/2; j++) {
	      if(approx_remapped_data_type1[index+j] != 0) {
		is_subgroup_zeros = false;
		break;
	      }
	    }
	    if(is_subgroup_zeros == false) {

	      /*
	      printf("Before Approx..\n");
	      for(int j = 0; j < group_size/2; j++)	printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      for(int j = 0; j < group_size/2; j++)	approx_remapped_data_type1[index+j] = 0;

	      /*
	      printf("After Approx..\n");
	      for(int j = 0; j < group_size/2; j++) printf("%02x", approx_remapped_data_type1[index+j]);
	      printf("\n");
	      */

	      approx_op = 1;	//set approx_op


	      //printf("mf %u | approximable float number\n", mf->get_request_uid());


	    } else {
	      //printf("subgroup zero!!!\n");
	    }
	  }
	  #endif


      }
  }
  ///

}






void hpcl_comp::transform_cache_data_type0(void* _mf, unsigned remap_res, unsigned type_index)
{
  mem_fetch* mf = (mem_fetch*)_mf;

  unsigned data_size = mf->get_real_data_size();
  mf->config_trans_data(data_size, type_index);

  //save data in odd addresses first, then data in even addresses
  /*
  unsigned even_addr_data_offset = data_size/2;
  for(unsigned i = 0; i < data_size; i++) {
    unsigned addr = 0;
    if(i % 2 == 0) addr = i/2;
    else	   addr = even_addr_data_offset+i/2;

    mf->set_trans_data(addr, mf->get_real_data(i));
  }
  */

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Org_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");
  #endif

  //printf("haha1\n");
  //unsigned trans_res = 4;
  //added by kh(071316)
  unsigned trans_res = remap_res;
  assert(data_size%trans_res == 0);
  std::vector< std::vector<unsigned char> > bins;
  for(unsigned i = 0; i < trans_res; i++) {
    std::vector<unsigned char> tmp;
    //tmp.resize(data_size/trans_res,0);
    bins.push_back(tmp);
  }

  //printf("haha2\n");
  unsigned index = 0;
  for(unsigned i = 0; i < data_size; i++) {
    index = i%trans_res;
    bins[index].push_back(mf->get_real_data(i));
  }

  //printf("haha3\n");
  unsigned ptr = 0;
  //for(unsigned i = 0; i < bins.size(); i++) {
  for(int i = bins.size()-1; i >= 0; i--) {
    //printf("bins[i].size() %u\n", bins[i].size());
    for(unsigned j = 0; j < bins[i].size(); j++) {
    //printf("ptr %u, bins[i][j] 0x%2x\n", ptr, bins[i][j]);
    mf->set_trans_data(ptr, bins[i][j], type_index);
    ptr++;
    }
  }

  #ifdef TRASFORM_CORRECTNESS_TEST
  printf("transform_cache_data::Trans_Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_trans_data(i));
  }
  printf("\n");
  #endif


  //printf("haha4\n");
  ///

}


void hpcl_comp::transform_cache_data_type1(void* data, unsigned remap_res, unsigned src_type_index, unsigned type_index)
{
  mem_fetch* mf = (mem_fetch*) data;
  unsigned data_size = mf->get_trans_data_size();

  #ifdef DATA_REMAP_DEBUG
  printf("----- MF %u's data -----\n", mf->get_request_uid());
  printf("Original Data - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_real_data(i));
  }
  printf("\n");


  printf("Remapped Data (type 0) - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_trans_data(i));
  }
  printf("\n");
  #endif

  mf->config_trans_data(data_size, type_index);
  //added by kh(071316)
  unsigned remap_unit_size = data_size / remap_res;
  //unsigned data_seg_no = data_size/32;
  unsigned data_seg_no = data_size/remap_unit_size;
  for(unsigned i = 0; i < data_seg_no; i++) {
    DATA_REMAP_DEBUG_PRINT("data segment %u\n", i);
    for(unsigned j = 0; j < (remap_unit_size*2); j=j+2) {
      unsigned index1 = i * remap_unit_size + j % remap_unit_size;
      unsigned index2 = i * remap_unit_size + (j+1) % remap_unit_size;
      DATA_REMAP_DEBUG_PRINT("\tindex1 %u index2 %u, ", index1, index2);
      unsigned char trans_data1 = mf->get_trans_data(index1, src_type_index);
      unsigned char trans_data2 = mf->get_trans_data(index2, src_type_index);
      if(j < remap_unit_size) {
	DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	trans_data1 = (trans_data1 & 0xf0)>>4;
	trans_data2 = (trans_data2 & 0xf0);
	trans_data1 = (trans_data1 | trans_data2);
	DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
      } else {
	DATA_REMAP_DEBUG_PRINT("\ttrans_data1 %02x, trans_data2 %02x, ", trans_data1, trans_data2);
	trans_data1 = (trans_data1 & 0x0f);
	trans_data2 = (trans_data2 & 0x0f)<<4;
	trans_data1 = (trans_data1 | trans_data2);
	DATA_REMAP_DEBUG_PRINT("\tretrans_data %02x\n", trans_data1);
	mf->set_trans_data(i*remap_unit_size+j/2, trans_data1, type_index);
      }
    }
  }

  #ifdef DATA_REMAP_DEBUG
  printf("Remapped Data (type 1) - ");
  for(unsigned i = 0; i < data_size; i++) {
    printf("%02x ", mf->get_trans_data(i, 1));
  }
  printf("\n");
  #endif


  //printf("haha4\n");
  ///

}


void hpcl_comp::remap_nonchar_data(void* data)
{
  mem_fetch* mf = (mem_fetch*) data;

  #ifdef DATA_REMAP_DEBUG
  unsigned data_size = mf->get_real_data_size();
  printf("----- MF %u's data SM %u WID %u PC %u Time %llu-----\n", mf->get_request_uid(), mf->get_tpc(), mf->get_wid(), mf->get_pc(), (gpu_sim_cycle+gpu_tot_sim_cycle));
  printf("Original Data - ");
  std::string val;
  std::vector<int> delta_val;
  std::vector<int> hex_val;
  for(int i = data_size-1; i >= 0; i--) {
    printf("%02x", mf->get_real_data(i));
    char str_hex[20];
    sprintf(str_hex, "%02x", mf->get_real_data(i));
    val += std::string(str_hex);
    if(i%4 == 0) {
      printf(" ");
      //std::cout << "val - " << val << std::endl;
      long i_hex = std::stol (val, 0, 16);
      if(hex_val.size() > 0) {
	delta_val.push_back(i_hex - hex_val[hex_val.size()-1]);
      }
      hex_val.push_back(i_hex);
      //std::cout << "i_hex  - " << i_hex << std::endl;
      val.clear();
    }
  }
  printf("\n");

  printf("Delta Val - ");
  for(int i = 0; i < delta_val.size(); i++) {
    printf("%02x ", delta_val[i]);
  }
  printf("\n");
  #endif

  unsigned remap_func_no = g_hpcl_comp_config.hpcl_data_remap_function.size();
  for(int i = 0; i < remap_func_no; i++) {
    unsigned func_index = g_hpcl_comp_config.hpcl_data_remap_function[i];

    std::vector<unsigned char>* remapped_data = mf->config_remapped_data(i, mf->get_real_data_size());
    unsigned remap_op_bits = 0;
    unsigned approx_op = 0;
    unsigned approx_exception = 0;
    std::vector<enum mem_fetch::REMAP_OP_TYPE> remap_op_types;
    if(func_index == 1) {
	remap_data_to_type0(mf, 4, remapped_data);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_1);
    } else if(func_index == 2) {
	remap_data_to_type1(mf, 4, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_2);
	mf->set_remap_op_bits(i, remap_op_bits);
	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
	mf->set_approx_exception(i, approx_exception);
    } else if(func_index == 3) {
	remap_data_to_type0(mf, 8, remapped_data);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_3);
    } else if(func_index == 4) {
	remap_data_to_type1(mf, 8, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_4);
	mf->set_remap_op_bits(i, remap_op_bits);
	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
	mf->set_approx_exception(i, approx_exception);
    } else if(func_index == 5) {
	remap_data_to_type0(mf, 2, remapped_data);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_5);
    } else if(func_index == 6) {
	remap_data_to_type1(mf, 2, remapped_data, remap_op_bits, remap_op_types, approx_op, approx_exception);
	mf->set_remapped_data_type(i, mem_fetch::REMAPPED_DATA_6);
	mf->set_remap_op_bits(i, remap_op_bits);
	mf->set_remap_op_type(i, remap_op_types);
	mf->set_approx_op(i, approx_op);
	mf->set_approx_exception(i, approx_exception);
    } else {
	printf("Error: func_index %u\n", func_index);
	assert(0);
    }
  }
}

int hpcl_comp::compare_data(void* data1, void* data2)
{
  mem_fetch* mf1 = (mem_fetch*) data1;
  mem_fetch* mf2 = (mem_fetch*) data2;

  unsigned data_size1 = mf1->get_real_data_size();
  unsigned data_size2 = mf2->get_real_data_size();

  if(data_size1 != data_size2)	return -1;

  for(unsigned i = 0; i < data_size1; i++) {
    if(mf1->get_real_data(i) != mf2->get_real_data(i))	return -1;
  }

  return 0;
}


void hpcl_comp::remap_char_data(void *data)
{
  mem_fetch* mf = (mem_fetch*)data;
  std::vector<unsigned char> lownibble_clus, highnibble_clus, extranibble_clus;
  unsigned char lsb = 0;
  for(unsigned i = 0; i < mf->get_real_data_size(); i = i+2) {
		unsigned char byte1 = mf->get_real_data(i);
		unsigned char byte2 = mf->get_real_data(i+1);

		unsigned char low_nibble = (byte1 & 0x0f);
		low_nibble = (low_nibble | ((byte2 & 0x0f) << 4));

		unsigned char high_nibble = 0;
		if(g_hpcl_comp_config.hpcl_char_remap_extrabit_en == 1) {
			high_nibble = (byte1 & 0xf0) >> 4;
			high_nibble = high_nibble >> 1;
			high_nibble = (high_nibble | (((byte2 & 0xf0) >> 1) & 0xf0) );
		} else {
			high_nibble = (byte1 & 0xf0) >> 4;
			high_nibble = (high_nibble | (byte2 & 0xf0));
		}

		lownibble_clus.push_back(low_nibble);
		highnibble_clus.push_back(high_nibble);

		if(g_hpcl_comp_config.hpcl_char_remap_extrabit_en == 1) {
			if(i % 8 == 0) {
				lsb = 0;
			}
			unsigned char high_nibble1 = (byte1 & 0xf0) >> 4;
			unsigned char high_nibble2 = (byte2 & 0xf0) >> 4;
			int bitpos1 = i % 8;
			int bitpos2 = (i+1) % 8;
			lsb = lsb | ((high_nibble1 & 0x01) << bitpos1);
			lsb = lsb | ((high_nibble2 & 0x01) << bitpos2);


			//Only we add extrabit to the high nibble cluster
			//if(((i+1) % 8 == 7) && ((i+1) <= mf->get_real_data_size()/2)) {
			if((i+1) % 8 == 7) {
				extranibble_clus.push_back(lsb);
			}
		}
  }

  //added by kh(022317)
  if(g_hpcl_comp_config.hpcl_char_remap_2bit_en == 1) {

	  std::vector<unsigned char> re_highnible_high2bit;
	  std::vector<unsigned char> re_highnible_low2bit;
	  for(int i = 0; i < highnibble_clus.size(); i = i + 2) {
		  unsigned char high2bit_clus = (highnibble_clus[i] & 0x0C) >> 2;
		  high2bit_clus |= ((highnibble_clus[i] & 0xC0) >> 4);
		  high2bit_clus |= ((highnibble_clus[i+1] & 0x0C) << 2);
		  high2bit_clus |= ((highnibble_clus[i+1] & 0xC0));

		  unsigned char low2bit_clus = (highnibble_clus[i] & 0x03);
		  low2bit_clus |= ((highnibble_clus[i] & 0x30) >> 2);
		  low2bit_clus |= ((highnibble_clus[i+1] & 0x03) << 4);
		  low2bit_clus |= ((highnibble_clus[i+1] & 0x30) << 2);

		  re_highnible_high2bit.push_back(high2bit_clus);
		  re_highnible_low2bit.push_back(low2bit_clus);
	  }

	  highnibble_clus.clear();

	  //deleted by kh(030117)
	  //highnibble_clus = re_highnible_low2bit;
	  //highnibble_clus.insert(highnibble_clus.end(), re_highnible_high2bit.begin(), re_highnible_high2bit.end());
	  //added by kh(030117)
	  highnibble_clus = re_highnible_high2bit;
	  highnibble_clus.insert(highnibble_clus.end(), re_highnible_low2bit.begin(), re_highnible_low2bit.end());
	  ///
  }
  ///


  std::vector<unsigned char>& txt_trans_data = mf->get_txt_trans_data_ptr();
	//added by kh(030117)
	if(g_hpcl_comp_config.hpcl_char_remap_1bit_en == 1) {
		assert(g_hpcl_comp_config.hpcl_char_remap_2bit_en != 1);
		assert(g_hpcl_comp_config.hpcl_char_remap_extrabit_en != 1);
		txt_trans_data.clear();
		//for(unsigned m = 0; m < 8; m++) {			//the mth bits are clustered.

		std::vector<int> remapp_list = {7,6,5,4,3,2,1,0};
		//To make compressible data close each other.
		if(g_hpcl_comp_config.hpcl_char_remap_1bit_swap_en == 1) {
			remapp_list = {7,5,6,4,3,2,1,0};
		}
		///

		//for(int m = 7; m >= 0; m--) {			//the mth bits are clustered.
		for(int m = 0; m < remapp_list.size(); m++) {			//the mth bits are clustered.
			for(unsigned i = 0; i < mf->get_real_data_size(); i = i + 8) {
				unsigned char remapped_byte = 0;
				for(unsigned j = i; j < i+8; j++) {
					unsigned char byte_val = mf->get_real_data(j);
					byte_val = byte_val & (1 << m);
					byte_val = (byte_val >> m);
					remapped_byte |= (byte_val << (j%8));
				}
				txt_trans_data.push_back(remapped_byte);
			}
		}

	} else {
		//deleted by kh(030117)
		//txt_trans_data = lownibble_clus;
		txt_trans_data = highnibble_clus;
		if(g_hpcl_comp_config.hpcl_char_remap_extrabit_en == 1) {
			txt_trans_data.insert(txt_trans_data.end(), extranibble_clus.begin(), extranibble_clus.end());
		}
		//txt_trans_data.insert(txt_trans_data.end(), highnibble_clus.begin(), highnibble_clus.end());
		txt_trans_data.insert(txt_trans_data.end(), lownibble_clus.begin(), lownibble_clus.end());
	}





}


double hpcl_comp::get_char_type_pct(void *data)
{
	mem_fetch* mf = (mem_fetch*)data;

	//printf("get_char_type_pct start!!\n");
	int all_elem_no = mf->get_real_data_size();
	int char_elem_no = 0;
	for(int i = 0; i < all_elem_no; i++) {

		unsigned char byte_data = mf->get_real_data(i);
		bool is_char = true;
		if(byte_data >= 0 && byte_data < 32) {
			//remove null pointer
			if(byte_data != 9 && byte_data != 10 && byte_data != 13) {	//Not tab, line feed, carriage return
				is_char = false;
				//printf("no_char_data = 0x%02x\n", byte_data);
			}
		} else if(byte_data >= 32 && byte_data < 128) {
			if(byte_data == 127)	is_char = false;
		} else {
			if(byte_data != 0xA0 && byte_data != 0xC2)
				is_char = false;
			//printf("no_char_data = 0x%02x\n", byte_data);
		}

		if(is_char == true) {
			char_elem_no++;
		}

	}

	return (double)char_elem_no/(double)all_elem_no*100;
}


void hpcl_comp::convert_char_symbols(void *data)
{
	mem_fetch* mf = (mem_fetch*)data;
	int all_elem_no = mf->get_real_data_size();
	int char_elem_no = 0;
	for(int i = 0; i < all_elem_no; i++) {
		unsigned char byte_data = mf->get_real_data(i);
		std::map<unsigned char, unsigned char>::iterator it = m_symbol_table.find(byte_data);
		if(it != m_symbol_table.end()) {
			mf->set_real_data(i, it->second);
		}
	}
}













