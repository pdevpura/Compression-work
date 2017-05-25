/*
 * hpcl_comp_pl.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_DSC_PL_H_
#define HPCL_COMP_DSC_PL_H_

#include "hpcl_comp_dsc_pl_proc.h"
#include "hpcl_comp_pl_data.h"
//#include "hpcl_user_define_stmts.h"
#include <vector>

class hpcl_comp_dsc_pl {

private:
  unsigned m_id;
  unsigned m_pipeline_no;
  std::vector<hpcl_comp_dsc_pl_proc*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

public:
  hpcl_comp_dsc_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_dsc_pl() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }

  void create();
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run();
  void print(unsigned pl_index=0);

};

hpcl_comp_dsc_pl::hpcl_comp_dsc_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
}

void hpcl_comp_dsc_pl::create() {

  if(m_pipeline_no == 1) {
    m_comp_pl_proc.push_back(new hpcl_comp_dsc_pl_proc());
    m_comp_pl_proc[0]->set_pl_index(0);
    m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
    m_input = m_comp_pl_proc[0]->get_input();

  } else {

    //added by kh(073016)
    for(unsigned i = 0; i < m_pipeline_no; i++)
    {
	  m_comp_pl_proc.push_back(new hpcl_comp_dsc_pl_proc());
	  m_comp_pl_proc[i]->set_pl_index(i);
    }
    m_comp_pl_proc[0]->set_pl_type(hpcl_comp_dsc_pl_proc::GET_OUTPUT);
    for(unsigned i = 1; i < m_pipeline_no-1; i++)
    {
	  m_comp_pl_proc[i]->set_pl_type(hpcl_comp_dsc_pl_proc::DUMMY);
    }
    m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_dsc_pl_proc::COMP);
    for(unsigned i = 0; i < m_pipeline_no-1; i++)
    {
	  m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
    }

    m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
    m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
    ///
  }

}

void hpcl_comp_dsc_pl::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

hpcl_comp_pl_data* hpcl_comp_dsc_pl::get_input() {
  return m_input;
}

mem_fetch* hpcl_comp_dsc_pl::run()
{
  mem_fetch* ret = NULL;

  if(m_pipeline_no == 1) {

    m_comp_pl_proc[0]->run();
    hpcl_comp_pl_data* tmp = m_comp_pl_proc[0]->get_output();
    assert(tmp);
    ret = tmp->get_mem_fetch();
    tmp->clean();

  } else {

    //added by kh(073016)
    //deleted by kh(083116)
    //for(int i = 0; i < m_pipeline_no; i++)
    //added by kh(083116)
    for(int i = (m_pipeline_no-1); i >= 0; i--)
    {
      if(m_comp_pl_proc[i]->get_pl_type() == hpcl_comp_dsc_pl_proc::COMP)
      {
	m_comp_pl_proc[i]->run();
	hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
	m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
	tmp->clean();
      }
      else if(m_comp_pl_proc[i]->get_pl_type() == hpcl_comp_dsc_pl_proc::GET_OUTPUT)
      {
	hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
	assert(tmp);
	mem_fetch* mf = tmp->get_mem_fetch();
	tmp->clean();
	ret = mf;
      }
      else
      {
	hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
	assert(tmp);
	mem_fetch* mf = tmp->get_mem_fetch();
	if(mf)
	{
	  m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(mf);
	}
	tmp->clean();
      }
    }
    ///

  }


  return ret;
}


void hpcl_comp_dsc_pl::print(unsigned pl_index) {

}

#endif /* HPCL_COMP_DSC_PL_H_ */
