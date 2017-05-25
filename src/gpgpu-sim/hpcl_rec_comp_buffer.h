/*
 * hpcl_rec_comp_buffer.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_REC_COMP_BUFFER_H_
#define HPCL_REC_COMP_BUFFER_H_

#include <vector>
#include "mem_fetch.h"

class hpcl_rec_comp_buffer {
private:
  unsigned m_id;

  std::vector<mem_fetch*> m_comp_buffer;

public:
  hpcl_rec_comp_buffer()		{	m_id = 0;	m_comp_buffer_size = 0;	};
  hpcl_rec_comp_buffer(int id)		{	m_id = id;	m_comp_buffer_size = 0;	};
  ~hpcl_rec_comp_buffer()		{	};


  void create(unsigned buffer_size);
  void push_mem_fetch(mem_fetch* mf);
  void pop_mem_fetch();
  mem_fetch* top_mem_fetch();

private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();

//added by kH(042316)
public:
  std::vector<mem_fetch*>& get_all_waiting_mfs (mem_fetch* mf);
  void delete_mfs (std::vector<mem_fetch*>& mfs);
  mem_fetch* get_next_mem_fetch();
  void del_mem_fetch(mem_fetch* mf);
///

  //added by kh(062816)
  enum last_mem_fetch_status {
    NOT_FOUND = 0,
    COMP_RES_MISMATCH,
    FOUND,
    LAST_MF_FULL,
  };

//added by kh(062316)
public:
  enum last_mem_fetch_status get_last_mem_fetch(int comp_res=-1, int sm=-1, mem_fetch** last_mf=NULL);
  void print();

//added by kh(062916)
public:
  mem_fetch* get_mem_fetch_to_same_sm(int comp_res, int sm);
  int compute_dist(mem_fetch* new_mf, mem_fetch* prev_mf);
  mem_fetch* get_closest_mem_fetch(int comp_res, int sm, mem_fetch* new_mf);
  void get_candi_for_rec_comp_to_diff_sm(int comp_res, int sm, mem_fetch* new_mf, std::vector<mem_fetch*>& candi);



//added by kh(071516)
public:
  mem_fetch* get_mem_fetch_to_same_sm(int sm);
  void get_mem_fetch_to_diff_sm(int sm, std::vector<mem_fetch*>& diff_sm_mfs);
};


#endif /* HPCL_COMP_PL_H_ */
