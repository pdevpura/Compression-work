/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_CUDA_COMP_DSC_H_
#define HPCL_CUDA_COMP_DSC_H_

/*
#include "hpcl_comp_pl_data.h"
#include "hpcl_user_define_stmts.h"
#include <cmath>
#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;
*/
#include <vector>
#include "hpcl_cuda_user_define_stmts.h"
#include "hpcl_cuda_mem_fetch.h"
#include "../option_parser.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;


class hpcl_cuda_comp_dsc {

public:
  hpcl_cuda_comp_dsc();
  ~hpcl_cuda_comp_dsc() {
//    if(m_input)	 	delete m_input;
//    if(m_output)	delete m_output;
  };
/*
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  int m_pl_index;
public:
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();
*/

public:
  void run(hpcl_cuda_mem_fetch* mf);

private:
  void run_ds_comp(hpcl_cuda_mem_fetch* mf, unsigned res, unsigned data_type, std::vector<unsigned char>& cache_data, std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only);
  int m_min_data_segment_size;

/*
private:
  void run_ds_comp_multi_res(mem_fetch* mf, unsigned max_res, unsigned data_type, std::vector<unsigned char>& cache_data,std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only);

private:
  unsigned encoding_bits;

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
*/

};



#endif /* HPCL_COMP_PL_PROC_H_ */
