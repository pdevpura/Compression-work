/*
 * hpcl_comp_anal.cc
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#include "hpcl_approx_mem_space.h"
#include <cassert>

//added by kh(081516)
#include "../option_parser.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

//added by kh(092316)
#include <algorithm>    // std::find

hpcl_approx_mem_space::hpcl_approx_mem_space ()
{
  // TODO Auto-generated constructor stub
}

hpcl_approx_mem_space::~hpcl_approx_mem_space ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_approx_mem_space::create()
{
  //This is set for memory space that memcpy does not capture.
  class mem_space_stat tmp;
  m_mem_space_stat.push_back(tmp);

}

void hpcl_approx_mem_space::add_sample(enum sample_type type, double val1, double val2, int id)
{
  if(type == CUDA_ALLOC_SPACE_START_ADDR) {

      //m_cuda_alloc_space_start_addr.push_back((unsigned)val1);
      //added by kh(092316)
      std::vector<unsigned>::iterator it;
      it = find (m_cuda_alloc_space_start_addr.begin(), m_cuda_alloc_space_start_addr.end(), (unsigned)val1);
      if (it == m_cuda_alloc_space_start_addr.end()) {
	m_cuda_alloc_space_start_addr.push_back((unsigned)val1);
      }
      ///

  }
  else if(type == CUDA_ALLOC_SPACE_SIZE) {

      //m_cuda_alloc_space_size.push_back((unsigned)val1);
      //added by kh(092316)
      std::vector<unsigned>::iterator it;
      it = find (m_cuda_alloc_space_size.begin(), m_cuda_alloc_space_size.end(), (unsigned)val1);
      if (it == m_cuda_alloc_space_size.end()) {
	  m_cuda_alloc_space_size.push_back((unsigned)val1);
      }

  }
  else if(type == CUDA_COPY_SPACE_START_ADDR) {

      //m_cuda_copy_space_start_addr.push_back((unsigned)val1);
      //added by kh(092316)
      std::vector<unsigned>::iterator it;
      it = find (m_cuda_copy_space_start_addr.begin(), m_cuda_copy_space_start_addr.end(), (unsigned)val1);
      if (it == m_cuda_copy_space_start_addr.end()) {
	  m_cuda_copy_space_start_addr.push_back((unsigned)val1);
      }
  }
  else if(type == L1_CACHE_MISS) {

    //val1: memory address accessed by ld inst
    //val2: ld inst pc
    bool is_found = false;
    for(unsigned i = 1; i < m_mem_space_stat.size(); i++) {
      if(m_mem_space_stat[i].is_in_mem_space((unsigned)val1) == true) {
	m_mem_space_stat[i].add_L1_miss_cnt((unsigned)val2);
	m_mem_space_stat[i].add_LD_access_type ((unsigned)val2, id);
	is_found = true; break;
      }
    }

    if(is_found == false) {
      m_mem_space_stat[0].add_L1_miss_cnt((unsigned)val2);
      m_mem_space_stat[0].add_LD_access_type ((unsigned)val2, id);
    }

  }
  else assert(0);
}

void hpcl_approx_mem_space::add_mem_space(unsigned start_addr, unsigned end_addr)
{
  //deleted by kh(081616)
  /*
  class mem_space_stat tmp(start_addr, end_addr);
  m_mem_space_stat.push_back(tmp);
 */

  //added by kh(081616)
  bool same_space_found = false;
  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    if(m_mem_space_stat[i].get_start_addr() == start_addr
    && m_mem_space_stat[i].get_end_addr() == end_addr) {
      same_space_found = true; break;
    }
  }

  if(same_space_found == false) {
    class mem_space_stat tmp(start_addr, end_addr);
    m_mem_space_stat.push_back(tmp);
  }
  ///

}


/*
void hpcl_approx_mem_space::add_sample(enum sample_type type, unsigned long long val, int id)
{
  if(type == CUDA_ALLOC_SPACE_START_ADDR)	m_cuda_alloc_space_start_addr.push_back(val);
  else if(type == CUDA_ALLOC_SPACE_SIZE)	m_cuda_alloc_space_size.push_back(val);
  else if(type == CUDA_COPY_SPACE_START_ADDR)	m_cuda_copy_space_start_addr.push_back(val);
  else assert(0);
}
*/

void hpcl_approx_mem_space::display(std::ostream & os) const
{
  os << "====== hpcl_approx_mem_space ======" << std::endl;

  /*
  os << "gpu_tot_cuda_alloc_space_start_addr = [ ";
  for(unsigned i = 0; i < m_cuda_alloc_space_start_addr.size(); i++) {
    printf("0X%08llx ", m_cuda_alloc_space_start_addr[i]);
  }
  os << " ] " << std::endl;

  os << "gpu_tot_cuda_alloc_space_size = [ ";
  for(unsigned i = 0; i < m_cuda_alloc_space_size.size(); i++) {
    printf("%llu ", m_cuda_alloc_space_size[i]);
  }
  os << " ] " << std::endl;
  */

  os << "gpu_tot_cuda_copy_space_start_addr = [ ";
  for(unsigned i = 0; i < m_cuda_copy_space_start_addr.size(); i++) {
    printf("0X%08llx ", m_cuda_copy_space_start_addr[i]);
  }
  os << " ] " << std::endl;

  /*
  os << "--------- gpu_tot_cuda_mem_space_analysis start ---------" << std::endl;
  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    ((class mem_space_stat)m_mem_space_stat[i]).print();
  }
  os << "--------- gpu_tot_cuda_mem_space_analysis end ---------" << std::endl;
 */
  os << "===============================" << std::endl;
}

void hpcl_approx_mem_space::update_overall_stat()
{
  //get L1 miss no
  unsigned long long all_l1_miss_no = 0;
  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    all_l1_miss_no += m_mem_space_stat[i].get_L1_miss_no();
  }

  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    m_mem_space_stat[i].set_all_L1_miss_no(all_l1_miss_no);
  }
  ///

}

void hpcl_approx_mem_space::clear()
{

}


bool hpcl_approx_mem_space::is_in_approximable_mem_space(unsigned addr) {

  if(g_hpcl_comp_config.hpcl_approx_memcpy_index.size() == 0) {

    for(unsigned i = 1; i < m_mem_space_stat.size(); i++) {
      if(m_mem_space_stat[i].is_in_mem_space(addr) == true) return true;
    }

  } else {	//added by kh(081616)

      for(unsigned i = 1; i < m_mem_space_stat.size(); i++) {
	std::map<unsigned,unsigned>::iterator it = g_hpcl_comp_config.hpcl_approx_memcpy_index.find(i);
	if (it != g_hpcl_comp_config.hpcl_approx_memcpy_index.end()) {
	  if(m_mem_space_stat[i].is_in_mem_space(addr) == true) return true;
	}
      }
  }

  return false;
}




