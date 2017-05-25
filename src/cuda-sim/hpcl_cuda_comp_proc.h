/*
 * hpcl_comp.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_CUDA_COMP_PROC_H_
#define HPCL_CUDA_COMP_PROC_H_

#include "hpcl_cuda_comp.h"
#include "hpcl_cuda_comp_dsc.h"
#include "hpcl_cuda_mem_fetch.h"

#include "hpcl_cuda_comp_lwm.h"

class hpcl_cuda_comp_proc
{
public:
  hpcl_cuda_comp_proc ();
  virtual
  ~hpcl_cuda_comp_proc ();

private:

  hpcl_cuda_comp m_hpcl_cuda_comp;
  hpcl_cuda_comp_dsc m_hpcl_cuda_comp_dsc;
  hpcl_cuda_comp_lwm<unsigned short> m_hpcl_comp_lwm_pl_2B;
  hpcl_cuda_comp_lwm<unsigned int> m_hpcl_comp_lwm_pl_4B;
  hpcl_cuda_comp_lwm<unsigned long long> m_hpcl_comp_lwm_pl_8B;

public:
  void run(hpcl_cuda_mem_fetch* mf, std::vector<unsigned char>& output_data, enum hpcl_cuda_mem_fetch::COMP_DATA_TYPE& type);
  void select_best_compressor(std::vector<unsigned>& comp_size, unsigned& min_comp_res, unsigned& min_comp_size);

//added by kh(091416)
  bool has_zero_approx_data(std::vector<unsigned char>& data);


};

#endif /* HPCL_COMP_H_ */
