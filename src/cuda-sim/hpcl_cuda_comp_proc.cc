/*
 * hpcl_cuda_comp.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <cassert>

#include "hpcl_cuda_comp_proc.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

//added by kh(081516)
#include "../gpgpu-sim/hpcl_approx_mem_space.h"
extern hpcl_approx_mem_space* g_hpcl_approx_mem_space;
///


hpcl_cuda_comp_proc::hpcl_cuda_comp_proc ()
{
  // TODO Auto-generated constructor stub
}

hpcl_cuda_comp_proc::~hpcl_cuda_comp_proc ()
{
  // TODO Auto-generated destructor stub
}

void hpcl_cuda_comp_proc::select_best_compressor(std::vector<unsigned>& comp_size, unsigned& min_comp_res, unsigned& min_comp_size) {

  min_comp_res = 2;
  min_comp_size = comp_size[0];
  for(int i = 1; i < comp_size.size(); i++) {
    if(comp_size[i] < min_comp_size) {
      min_comp_size = comp_size[i];
      if(i == 1)	min_comp_res = 4;
      else if(i == 2)	min_comp_res = 8;
      else		assert(0);
    }
  }
}

bool hpcl_cuda_comp_proc::has_zero_approx_data(std::vector<unsigned char>& data)
{
  for(int i = 0; i < data.size()/4; i++) {
    int index = i*4;
    bool is_zero = true;
    for(int j = 0 ; j < 4; j++) {
      //To check whether or not the approximated word is zero
      if(j == 3) {
	if(!(data[index+j] == 0 || data[index+j] == 128)) {
	   is_zero = false;	break;
	}
      } else {
	if(data[index+j] != 0) {
	   is_zero = false;	break;
	}
      }
      ///
    }
    if(is_zero == true) {

      /*
      printf("zero found!!!!\n");
      for(int j = 0; j < 4; j++) {
	printf("%02x", data[index+j]);
      }
      */
      //assert(0);

      return true;
    }
  }

  return false;
}


void hpcl_cuda_comp_proc::run(hpcl_cuda_mem_fetch* mf, std::vector<unsigned char>& output_data, enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE& type )
{
  //added by kh(082016)
  //To simulate approximation fast
  if(mf) {

      assert(g_hpcl_comp_config.hpcl_float_det_type == hpcl_comp_config::MEMSPACE);

  	mem_addr_t addr = mf->get_access_addr();
  	if(g_hpcl_approx_mem_space->is_in_approximable_mem_space(addr) == true) {

  	  output_data.clear();
	//if(data_type == hpcl_cuda_mem_fetch::REMAPPED_DATA_2) {
	 // if(approx_op[0] == 1) {
	  //std::vector<unsigned char> tmp_data = mf->get_real_data_ptr();

  	  std::vector<unsigned char> tmp_data;
  	  mf->copy_real_data(tmp_data);
	  m_hpcl_cuda_comp.approximate_data(tmp_data, hpcl_cuda_mem_fetch::REMAPPED_DATA_2);

	  #ifdef old
	  //added by kh(091416)
	  //When the data has no zero-approximated data, the approximated one is passed
	  if(has_zero_approx_data(tmp_data) == false) {
	    printf("mf %u has no zero data\n", mf->get_request_uid());
	    output_data = tmp_data;
	    /*
	    printf("tmp_data - ");
	    for(int i = 0; i < tmp_data.size(); i++) {
	      printf("%02x ", tmp_data[i]);
	    }
	    printf("\n");

	    printf("tmp_data1 - ");
	    std::vector<unsigned char> tmp_data1 = mf->get_real_data_ptr();
	    for(int i = 0; i < tmp_data1.size(); i++) {
	      printf("%02x ", tmp_data1[i]);
	    }
	    printf("\n");
	    assert(0);
	    */
	  } else {
	    printf("mf %u has zero data\n", mf->get_request_uid());
	    if(mf->get_request_uid() == 762372) {
	      printf("tmp_data1 - ");
	      std::vector<unsigned char> tmp_data1 = mf->get_real_data_ptr();
	      for(int i = 0; i < tmp_data1.size(); i++) {
		printf("%02x ", tmp_data1[i]);
	      }
	      printf("\n");
	    }
	  }
	  #endif

	  output_data = tmp_data;
	  type = hpcl_cuda_mem_fetch::REMAPPED_DATA_2;
	  //}
	//}
  	}
      //}
  }
  ///


#ifdef old_slow_version
  //Step1: Data Remapping
  if(mf) {

    unsigned data_size = mf->get_real_data_size();
    /*
    printf("Org_Data(mf : %u, size: %u, pc: %u, sid: %u, wid: %u) = ", mf->get_request_uid(), data_size, mf->get_pc(), mf->get_sid(), mf->get_wid());
    for(int i = 0; i < data_size; i++) {
      printf("%02x", mf->get_real_data(i));
    }
    printf("\n");
    */
    //printf("mf %u remapping starts.\n", mf->get_request_uid());


    m_hpcl_cuda_comp.remap_data(mf);

    /*
    printf("mf %u remapping ends.\n", mf->get_request_uid());
    */

    unsigned remap_func_no = g_hpcl_comp_config.hpcl_data_remap_function.size();
    //deleted by kh(080316)
    //assert(remap_func_no >= 2);
    //unsigned approx_op1 = mf->get_approx_op(0);
    //unsigned approx_op2 = mf->get_approx_op(1);
    //added by kh(080316)
    std::vector<unsigned> approx_op(remap_func_no,0);
    for(int i = 0; i < remap_func_no; i++) {
      approx_op[i] = mf->get_approx_op(i);
      //printf("approx_op %u.\n", approx_op[i]);
    }

    m_hpcl_cuda_comp_dsc.run(mf);
    m_hpcl_comp_lwm_pl_2B.run(mf);
    m_hpcl_comp_lwm_pl_4B.run(mf);
    m_hpcl_comp_lwm_pl_8B.run(mf);

    hpcl_cuda_mem_fetch* ret_mf = mf;
    if(ret_mf) {


      std::vector<unsigned> comp_bits;
      comp_bits.clear();
      comp_bits.push_back(ret_mf->get_comp_data_bits(2));
      comp_bits.push_back(ret_mf->get_comp_data_bits(4));
      comp_bits.push_back(ret_mf->get_comp_data_bits(8));


      unsigned min_dsm_comp_bits = ret_mf->get_comp_data_bits();
      //unsigned min_dsm_comp_res = ret_mf->get_dsc_comp_res();
      enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE min_data_type = ret_mf->get_dsc_comp_data_type();

      unsigned min_lwm_comp_bits = 0;
      unsigned min_lwm_comp_res = 0;
      select_best_compressor(comp_bits, min_lwm_comp_res, min_lwm_comp_bits);

      assert(min_lwm_comp_bits > 0);
      assert(min_dsm_comp_bits > 0);

      //printf("min_dsm_comp_bits %u min_lwm_comp_bits %u min_lwm_comp_res %u\n", min_dsm_comp_bits, min_lwm_comp_bits, min_lwm_comp_res);


      if(min_dsm_comp_bits <= min_lwm_comp_bits) {

	//printf("DSM_COMP Wins\n");

	ret_mf->set_comp_data_bits(min_dsm_comp_bits);
	//ret_mf->set_comp_res(min_dsm_comp_res);
	ret_mf->set_comp_algo_type(hpcl_cuda_mem_fetch::DSM_COMP);
	ret_mf->set_dsc_comp_data_type(min_data_type);

	/* ---------- DATA APPROXIMATION START ---------- */

	//approx_op1 has priority.
	hpcl_cuda_mem_fetch::COMP_DATA_TYPE data_type = mf->get_dsc_comp_data_type();
	output_data.clear();
	if(data_type == hpcl_cuda_mem_fetch::REMAPPED_DATA_2) {
	  if(approx_op[0] == 1) {
	    std::vector<unsigned char> tmp_data = mf->get_real_data_ptr();
	    m_hpcl_cuda_comp.approximate_data(tmp_data, hpcl_cuda_mem_fetch::REMAPPED_DATA_2);
	    output_data = tmp_data;
	    type = hpcl_cuda_mem_fetch::REMAPPED_DATA_2;
	  }
	} else if(data_type == hpcl_cuda_mem_fetch::REMAPPED_DATA_4) {
	  if(approx_op[1] == 1) {
	    std::vector<unsigned char> tmp_data = mf->get_real_data_ptr();
	    m_hpcl_cuda_comp.approximate_data(tmp_data, hpcl_cuda_mem_fetch::REMAPPED_DATA_4);
	    output_data = tmp_data;
	    type = hpcl_cuda_mem_fetch::REMAPPED_DATA_4;
	  }
	}

	/* ---------- DATA APPROXIMATION END ---------- */

      } else {


	//printf("LWM_COMP Wins\n");


	ret_mf->set_comp_data_bits(min_lwm_comp_bits);
	//ret_mf->set_comp_res(min_lwm_comp_res);
	ret_mf->set_comp_algo_type(hpcl_cuda_mem_fetch::LWM_COMP);
	ret_mf->set_dsc_comp_data_type(hpcl_cuda_mem_fetch::ORG_DATA);
	//since dsm is not used.
	//ret_mf->set_dsc_comp_res(0);

      }

      if(ret_mf->get_comp_data_bits() > ret_mf->get_real_data_size()*8) {
	ret_mf->set_comp_data_bits(ret_mf->get_real_data_size()*8+1);
	ret_mf->set_comp_algo_type(hpcl_cuda_mem_fetch::NO_COMP);
      }

      /*
      printf("Final: Select ");
      ret_mf->print_dsc_comp_data_type(ret_mf->get_dsc_comp_data_type());
      if(ret_mf->get_comp_algo_type() == hpcl_cuda_mem_fetch::DSM_COMP) {
	    printf("DSM_COMP\n");
      } else if(ret_mf->get_comp_algo_type() == hpcl_cuda_mem_fetch::LWM_COMP) {
	    printf("LWM_COMP\n");
      } else {
	    printf("NO_COMP\n");
      }
      */
    }
  }
#endif














#ifdef test
    //approx_op1 has priority.
    hpcl_cuda_mem_fetch::COMP_DATA_TYPE data_type = mf->get_dsc_comp_data_type();

    output_data.clear();
    if(data_type == hpcl_cuda_mem_fetch::REMAPPED_DATA_2) {
      if(approx_op1 == 1) {
	std::vector<unsigned char> tmp_data = mf->get_real_data_ptr();
	m_hpcl_cuda_comp.approximate_data(tmp_data, hpcl_cuda_mem_fetch::REMAPPED_DATA_2);
	output_data = tmp_data;
	type = hpcl_cuda_mem_fetch::REMAPPED_DATA_2;
      }
    } else if(data_type == hpcl_cuda_mem_fetch::REMAPPED_DATA_4) {
      if(approx_op2 == 1) {
	std::vector<unsigned char> tmp_data = mf->get_real_data_ptr();
	m_hpcl_cuda_comp.approximate_data(tmp_data, hpcl_cuda_mem_fetch::REMAPPED_DATA_4);
	output_data = tmp_data;
	type = hpcl_cuda_mem_fetch::REMAPPED_DATA_4;
      }
    }
#endif






}
