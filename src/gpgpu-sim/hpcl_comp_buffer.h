/*
 * hpcl_comp_buffer.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_BUFFER_H_
#define HPCL_COMP_BUFFER_H_

#include <vector>
#include "mem_fetch.h"

class hpcl_comp_buffer {
private:
  unsigned m_id;

  std::vector<mem_fetch*> m_comp_buffer;

public:
  hpcl_comp_buffer()		{	m_id = 0;	m_comp_buffer_size = 0;	m_last_sm_id = -1; };
  hpcl_comp_buffer(int id)	{	m_id = id;	m_comp_buffer_size = 0;	m_last_sm_id = -1; };
  ~hpcl_comp_buffer()		{	};


  void create(unsigned buffer_size, unsigned subbuffer_no);
  void push_mem_fetch(mem_fetch* mf);
  void pop_mem_fetch();
  mem_fetch* top_mem_fetch();

private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();

//added by kh(042116)
  std::vector<std::vector<mem_fetch*> > m_comp_subbuffer;
  void print(unsigned id, unsigned comp_res=0);
  bool has_pending_mem_fetch();
  void get_reply_rate_dist(double& single_rep_rate, double& multi_rep_rate, double& no_rep_rate, double& multi_rep_no);
///

#ifdef old
//added by kh(042316)
private:
  hpcl_dict<unsigned char>* m_dict_for_inter_read_replies;
public:
  void anal_redund_inter_read_replies();
///
#endif

//added by kH(042316)
public:
  std::vector<mem_fetch*>& get_all_waiting_mfs (mem_fetch* mf);
  void delete_mfs (std::vector<mem_fetch*>& mfs);
///

  mem_fetch* get_next_mem_fetch();
  void del_mem_fetch(mem_fetch* mf);
///

//added by kh(052416)
private:
  int m_last_sm_id;		//for fair scanning
//

//added by kh(072616)
public:
  void print();

};


#endif /* HPCL_COMP_PL_H_ */
