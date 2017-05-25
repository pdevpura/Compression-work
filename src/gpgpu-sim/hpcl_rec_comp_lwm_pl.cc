/*
 * NetworkStat.cpp
 *
 *  Created on: Sep 19, 2015
 *      Author: mumichang
 */

#include "hpcl_rec_comp_lwm_pl.h"
#include "hpcl_user_define_stmts.h"
#include <iostream>
#include <cassert>

extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
//

hpcl_rec_comp_lwm_pl::hpcl_rec_comp_lwm_pl() {
  // TODO Auto-generated constructor stub
  m_input = NULL;
  m_output = NULL;
}


hpcl_rec_comp_lwm_pl::~hpcl_rec_comp_lwm_pl() {
  // TODO Auto-generated destructor stub
}

void hpcl_rec_comp_lwm_pl::push_input(mem_fetch* mf)
{
  m_input = mf;
}


void hpcl_rec_comp_lwm_pl::create(unsigned pipeline_stage_no, unsigned max_inter_comp_size, hpcl_rec_comp_buffer* rec_comp_buffer, unsigned id, hpcl_comp_buffer* m_comp_buffer)
{
  m_hpcl_rec_comp_lwm_pl_proc.resize(pipeline_stage_no);
  for(int i = 0; i < pipeline_stage_no; i++) {
    m_hpcl_rec_comp_lwm_pl_proc[i] = new hpcl_rec_comp_lwm_pl_proc;
    m_hpcl_rec_comp_lwm_pl_proc[i]->create(i, max_inter_comp_size, rec_comp_buffer, id);
    m_hpcl_rec_comp_lwm_pl_proc[i]->set_comp_buffer(m_comp_buffer);
  }
  //connect links between processes
  for(int i = 0; i < (pipeline_stage_no-1); i++) {
    m_hpcl_rec_comp_lwm_pl_proc[i]->set_output_link(m_hpcl_rec_comp_lwm_pl_proc[i+1]->get_input_link());
  }
  m_hpcl_rec_comp_lwm_pl_proc[pipeline_stage_no-1]->set_output_link(new pl_proc_data_link);

  m_id = id;
}


void hpcl_rec_comp_lwm_pl::run()
{
  int last_stage = m_hpcl_rec_comp_lwm_pl_proc.size()-1;
  int first_stage = 0;

  m_output = NULL;
  m_hpcl_rec_comp_lwm_pl_proc[first_stage]->push_input(m_input);

  for(int i = m_hpcl_rec_comp_lwm_pl_proc.size()-1; i >=0; i--) {
    m_hpcl_rec_comp_lwm_pl_proc[i]->run();
  }

  m_output = m_hpcl_rec_comp_lwm_pl_proc[last_stage]->pop_output();
  m_input = NULL;
}

mem_fetch* hpcl_rec_comp_lwm_pl::pop_output()
{
  return m_output;
  //return m_hpcl_intra_coal_buffer->pop_mem_fetch();
  /*
  mem_fetch* mf = m_hpcl_intra_coal_buffer->pop_mem_fetch();
  if(mf) {
    std::vector<mem_fetch*>& other_mfs = mf->get_intra_coalesced_read_reply_mf();
    m_hpcl_intra_coal_buffer->delete_mfs(other_mfs);
  }
  return mf;
  */
}

//void hpcl_rec_comp_lwm_pl::create(unsigned sm_no, unsigned max_intra_coal_size)
//{
  /*
  m_hpcl_intra_coal_buffer = new hpcl_intra_coal_buffer;
  m_hpcl_intra_coal_buffer->create(g_hpcl_coalescing_config.hpcl_read_reply_intra_coalescing_buffer_size, sm_no);
  m_max_intra_coal_size = max_intra_coal_size;
  //printf("m_max_intra_coal_size : %u\n", m_max_intra_coal_size);
   *
   */
//}


mem_fetch* hpcl_rec_comp_lwm_pl::top_output()
{
  return m_output;

  //return m_hpcl_intra_coal_buffer->top_mem_fetch();

  /*
  mem_fetch* mf = m_hpcl_intra_coal_buffer->top_mem_fetch();
  if(mf && !mf->get_is_write()) {

    //Step1: clean previously coalesced mfs.
    mf->clean_intra_coalesced_read_reply_mf();

    //Step2: add mfs destined to the same SM
    unsigned response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
    std::vector<mem_fetch*>& other_mfs = m_hpcl_intra_coal_buffer->get_all_waiting_mfs(mf);
    for(unsigned i = 0; i < other_mfs.size(); i++) {
      if(!other_mfs[i]->get_is_write() && other_mfs[i] != mf) {
	response_size += other_mfs[i]->get_data_size();
	response_size += 4;	//4B address only.

	if(response_size <= m_max_intra_coal_size) {
	  mf->push_intra_coalesced_read_reply_mf(other_mfs[i]);
	  INTRACOAL_DEBUG_PRINT("mf %u is coalesced into mf %u\n", other_mfs[i]->get_request_uid(), mf->get_request_uid());
	} else {
	  break;
	}
      }
    }

  }

  return mf;
  */
}


bool hpcl_rec_comp_lwm_pl::has_buffer_space()
{
  //return m_hpcl_intra_coal_buffer->has_comp_buffer_space();
}

void hpcl_rec_comp_lwm_pl::print()
{
  //m_hpcl_intra_coal_buffer->print();
}
