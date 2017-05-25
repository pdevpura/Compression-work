/*
 * hpcl_decomp_dsc_pl.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DECOMP_DSC_PL_H_
#define HPCL_DECOMP_DSC_PL_H_

#include "hpcl_decomp_dsc_pl_proc.h"
#include <vector>


extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;

template <class K>
class hpcl_decomp_dsc_pl
{
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  std::vector<hpcl_decomp_dsc_pl_proc<K>*> m_decomp_pl_proc;
  hpcl_comp_pl_data* m_input_data;
  hpcl_comp_pl_data* m_output_data;

  //added by kh(102016)
  int m_current_data_cnt;

public:
  hpcl_decomp_dsc_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_decomp_dsc_pl () {}

  void create();
  //void set_input_data(hpcl_comp_pl_data* input_data);
  //hpcl_comp_pl_data* get_output_data();
  //hpcl_comp_pl_data* get_input();


  void reset_output_data();
  mem_fetch* run(mem_fetch** fb_mf);

  //added by kh(102016)
  void set_input_data(mem_fetch* data);
  int get_current_data_cnt();
  ///


//added by kh(092716)
private:
  unsigned m_busy_flag;
public:
  unsigned get_busy_flag();
  void clear_busy_flag();
  void set_busy_flag();

};

template <class K>
hpcl_decomp_dsc_pl<K>::hpcl_decomp_dsc_pl(unsigned pipeline_no, unsigned id)
{
  m_pipeline_no = pipeline_no;
  m_output_data = new hpcl_comp_pl_data;
  m_id = id;
  m_input_data = NULL;
  m_output_data = NULL;

  //added by kh(092716)
  m_busy_flag = 0;
  ///

  m_current_data_cnt = 0;
}

template <class K>
//void hpcl_decomp_pl<K>::create(unsigned dict_size, enum hpcl_dict_rep_policy policy)
void hpcl_decomp_dsc_pl<K>::create()
{
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_decomp_pl_proc.push_back(new hpcl_decomp_dsc_pl_proc<K>(m_id));
    m_decomp_pl_proc[i]->set_pl_index(i);
  }
  for(unsigned i = 0; i < (m_pipeline_no-1); i++) {
    m_decomp_pl_proc[i]->set_output(m_decomp_pl_proc[i+1]->get_input());
  }

  m_input_data = m_decomp_pl_proc[0]->get_input();
  m_decomp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
}

template <class K>
void hpcl_decomp_dsc_pl<K>::set_input_data(mem_fetch* data) {
  m_input_data->set_mem_fetch(data);
  m_current_data_cnt++;
}
template <class K>
int hpcl_decomp_dsc_pl<K>::get_current_data_cnt() {
  return m_current_data_cnt;
}


/*
template <class K>
void hpcl_decomp_dsc_pl<K>::set_input_data(hpcl_comp_pl_data* input_data) {

  //added by kh(071816)
  m_input_data->copy(input_data);
  ///
}
template <class K>
hpcl_comp_pl_data* hpcl_decomp_dsc_pl<K>::get_output_data() {
  return m_output_data;
}
template <class K>
hpcl_comp_pl_data* hpcl_decomp_dsc_pl<K>::get_input() {
  return m_input_data;
}
*/


template <class K>
void hpcl_decomp_dsc_pl<K>::reset_output_data() {
  m_output_data->clean();
}


template <class K>
unsigned hpcl_decomp_dsc_pl<K>::get_busy_flag() {
  return m_busy_flag;
}

template <class K>
void hpcl_decomp_dsc_pl<K>::clear_busy_flag() {
  m_busy_flag = 0;
}

template <class K>
void hpcl_decomp_dsc_pl<K>::set_busy_flag() {
  m_busy_flag = 1;
}

template <class K>
//mem_fetch* hpcl_decomp_dsc_pl<K>::run(unsigned long long time)
mem_fetch* hpcl_decomp_dsc_pl<K>::run(mem_fetch** fb_mf)
{
  unsigned busy_count = 0;
  mem_fetch *ret=NULL;
  //added by kh(083116)
  for(int i=(m_pipeline_no-1); i>=0; i--)
  {
    if(i==0) // DCompression phase
    {
      //return m_input_data->get_mem_fetch();
      m_decomp_pl_proc[0]->run();

      *fb_mf = NULL;
      hpcl_comp_pl_data* tmp = m_decomp_pl_proc[0]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      if(mf) {

	//added by kh(062516)
	//Inter-packet compression
	std::vector<mem_fetch*>& merged_mfs_to_same_SM = mf->get_merged_mfs_to_same_SM();
	if(merged_mfs_to_same_SM.size() > 0) {
	*fb_mf = merged_mfs_to_same_SM[0];
	assert(fb_mf);
	merged_mfs_to_same_SM.erase(merged_mfs_to_same_SM.begin());
	(*fb_mf)->set_merged_mfs_to_same_SM(merged_mfs_to_same_SM);
	}

	if(m_pipeline_no == 1)
	{
	  ret = mf;
	  //added by kh(102016)
	  m_current_data_cnt--;
	}
	else
	m_decomp_pl_proc[i+1]->get_output()->set_mem_fetch(mf);

	//added by kh(092716)
	busy_count++;
	//printf("\t%llu | decomp | id %u | stage %d | move mf %d to %d\n", (gpu_sim_cycle+gpu_tot_sim_cycle), m_id, i, mf->get_request_uid(), i+1);

      }

      tmp->clean();
    }
    else if( i == (m_pipeline_no -1)) // Output phase
    {
      hpcl_comp_pl_data* tmp = m_decomp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      if(mf) {
	ret = mf;
	//added by kh(102016)
	m_current_data_cnt--;

	//added by kh(092716)
	busy_count++;

	//printf("\t%llu | decomp | id %u | stage %d | move mf %d at last stage\n", (gpu_sim_cycle+gpu_tot_sim_cycle), m_id, i, mf->get_request_uid());
      }
      tmp->clean();
    }
    else
    {
      // Do nothing
      hpcl_comp_pl_data* tmp = m_decomp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      if(mf)
      {
	m_decomp_pl_proc[i+1]->get_output()->set_mem_fetch(mf);

	//added by kh(092716)
	busy_count++;

	//printf("\t%llu | decomp | id %u | stage %d | move mf %d to %d\n", (gpu_sim_cycle+gpu_tot_sim_cycle), m_id, i, mf->get_request_uid(), i+1);

      }
      tmp->clean();
    }
  }

  //added by kh(092716)
  if(busy_count > 0) {
    set_busy_flag();
    //printf("set_busy_flag is called\n");

  } else {
    clear_busy_flag();
    //printf("clear_busy_flag is called\n");

  }
  ///

  //added by kh(041816)
  return ret;
}





#endif /* HPCL_DECOMP_PL_H_ */
