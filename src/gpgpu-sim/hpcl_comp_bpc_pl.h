
// added by abpd

#ifndef HPCL_COMP_BPC_PL_H_
#define HPCL_COMP_BPC_PL_H_

#include "hpcl_comp_bpc_pl_proc.h"
#include "hpcl_comp_pl_data.h"
#include <vector>
#include<unordered_map>
#include<utility>
#include<fstream>
#include<sstream>
#include<algorithm>
#include <sstream>
#include <iomanip>
using namespace std;

template<class K>
class hpcl_comp_bpc_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_bpc_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;
  
public:

  hpcl_comp_bpc_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_bpc_pl() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }

  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run();
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();

};

template<class K>
hpcl_comp_bpc_pl<K>::hpcl_comp_bpc_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_comp_buffer_size = -1;
}

template<class K>
void hpcl_comp_bpc_pl<K>::create(unsigned comp_buffer_size) {
  /* can be used cycle */
  if(m_pipeline_no==1){

	  m_comp_pl_proc.push_back(new hpcl_comp_bpc_pl_proc<K>());
	  m_comp_pl_proc[0]->set_pl_index(0);
	  m_input = m_comp_pl_proc[0]->get_input();
	  m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
	  m_comp_buffer_size = comp_buffer_size;

  }else{
    for(unsigned i = 0; i < m_pipeline_no; i++)
    {
	    m_comp_pl_proc.push_back(new hpcl_comp_bpc_pl_proc<K>());
	    m_comp_pl_proc[i]->set_pl_index(i);
    }
    //m_comp_pl_proc[0]->set_pl_type(hpcl_comp_bpc_pl_proc<K>::GET_OUTPUT);
    m_comp_pl_proc[0]->set_pl_type(hpcl_comp_bpc_pl_proc<K>::COMP);
    for(unsigned i = 1; i < m_pipeline_no-1; i++)
    {
	    m_comp_pl_proc[i]->set_pl_type(hpcl_comp_bpc_pl_proc<K>::DUMMY);
    }
    //m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_bpc_pl_proc<K>::COMP);
    m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_bpc_pl_proc<K>::GET_OUTPUT);
    //for(unsigned i = 0; i < m_pipeline_no-1; i++)
    for(unsigned i = 1; i < m_pipeline_no; i++)
    {
	    m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
    }


    //m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
    m_input = m_comp_pl_proc[0]->get_input();
    m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
    //m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
    m_comp_buffer_size = comp_buffer_size;
  }

}

template<class K>
void hpcl_comp_bpc_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_bpc_pl<K>::get_input() {
  return m_input;
}
template<class K>
mem_fetch* hpcl_comp_bpc_pl<K>::run() {
  //cout<<"Entered here ct="<<hpcl_bpc_iter<<"\n ";
  mem_fetch* ret = NULL;
  
  //For Encode
  //cout<<"ABPD ENTER ENCODE\n";
  //deleted by kh(083116)
  //for(int i = 0; i < m_pipeline_no; i++)
  //added by kh(083116)
  for(int i = m_pipeline_no-1; i >=0; i--)
  {
    if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_bpc_pl_proc<K>::COMP)
    {
      m_comp_pl_proc[i]->run();
      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      m_comp_pl_proc[i+1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
      tmp->clean();
    }
    else if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_bpc_pl_proc<K>::GET_OUTPUT)
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
	m_comp_pl_proc[i+1]->get_output()->set_mem_fetch(mf);
      }
      tmp->clean();
    }

  }
  //cout<<"EXIT ENCODE\n";
  return ret;

}
	


template<class K>
mem_fetch* hpcl_comp_bpc_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_bpc_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_bpc_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}








#endif /* HPCL_COMP_PL_H_ */
