#ifndef HPCL_DECOMP_FPC_PL_PROC_H_
#define HPCL_DECOMP_FPC_PL_PROC_H_

//#include "hpcl_dict.h"
#include "hpcl_comp_pl_data.h"
#include <cassert>
#include <iostream>


//added by kh(022716)
extern struct mc_placement_config g_mc_placement_config;
extern gpgpu_sim *g_the_gpu;


template <class K>
class hpcl_decomp_fpc_pl_proc
{
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output[2];
  int m_pl_index;
  unsigned m_par_id;

public:
  hpcl_decomp_fpc_pl_proc(unsigned par_id);
  ~hpcl_decomp_fpc_pl_proc() {}

  void set_output(hpcl_comp_pl_data* output, int index=0);
  hpcl_comp_pl_data* get_output(int index=0);
  hpcl_comp_pl_data* get_input();
  void reset_output(int index=0);
  void set_pl_index(int pl_index);
  void run();
};


template <class K>
hpcl_decomp_fpc_pl_proc<K>::hpcl_decomp_fpc_pl_proc(unsigned par_id) {
  m_input = new hpcl_comp_pl_data;
  m_output[0] = NULL;
  m_output[1] = NULL;
  m_pl_index = -1;
  m_par_id = par_id;
}

template <class K>
void hpcl_decomp_fpc_pl_proc<K>::set_output(hpcl_comp_pl_data* output, int index) {
  m_output[index] = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_decomp_fpc_pl_proc<K>::get_output(int index) {
  return m_output[index];
}

template <class K>
hpcl_comp_pl_data* hpcl_decomp_fpc_pl_proc<K>::get_input() {
  return m_input;
}

template <class K>
void hpcl_decomp_fpc_pl_proc<K>::reset_output(int index) {
  m_output[index]->clean();
}

template <class K>
void hpcl_decomp_fpc_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_decomp_fpc_pl_proc<K>::run()
{
  mem_fetch* mf = m_input->get_mem_fetch();
  m_output[0]->set_mem_fetch(mf);
  m_input->clean();
}

#endif /* HPCL_DECOMP_PL_PROC_H_ */
