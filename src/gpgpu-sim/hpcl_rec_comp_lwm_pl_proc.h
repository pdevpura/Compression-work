
#ifndef HPCL_REC_COMP_LWM_PL_PROC_H_
#define HPCL_REC_COMP_LWM_PL_PROC_H_


#include "hpcl_rec_comp_buffer.h"
#include "hpcl_comp_buffer.h"
#include "mem_fetch.h"

class pl_proc_data_link {
public:
  mem_fetch* m_data;
  //mem_fetch* m_data2;

  pl_proc_data_link() {
    m_data = NULL;
    //m_data2 = NULL;
  }
  ~pl_proc_data_link() {}
};


class hpcl_rec_comp_lwm_pl_proc {
public:
  hpcl_rec_comp_lwm_pl_proc();
  virtual ~hpcl_rec_comp_lwm_pl_proc();

  void init();
  void run();
  void push_input(mem_fetch* new_mf);
  mem_fetch* pop_output();
  //mem_fetch* pop_output_mf();
  //mem_fetch* top_output_mf();
  void create(unsigned pipeline_stage_index, unsigned max_comp_data_size, hpcl_rec_comp_buffer* rec_comp_buffer, unsigned par_id);

private:
  unsigned m_par_id;
  unsigned m_pipeline_stage_index;
  pl_proc_data_link* m_input_link;
  pl_proc_data_link* m_output_link;
  unsigned m_max_comp_data_size;

public:
  void print();
  pl_proc_data_link* get_input_link();
  void set_output_link(pl_proc_data_link* data_link);

public:
  enum rec_lwm_mode {
    NO_MODE = 0,
    REC_COMP_INTRA_SM,
    REC_COMP_INTER_SM,
    REC_COMP_ALL_SM,
  };
private:
  //void rec_append_method(mem_fetch* mf);
  mem_fetch* rec_append_method(mem_fetch* new_mf, mem_fetch* prev_mf);
  unsigned sub_rec_lwm_method(mem_fetch* new_mf, mem_fetch* prev_mf);
  mem_fetch* rec_lwm_method(mem_fetch* mf, mem_fetch* prev_mf, bool max_pkt_turnon=true, enum rec_lwm_mode mode=NO_MODE);

  void rec_comp_test_proc(mem_fetch* new_mf);
  void rec_comp_algo_proc(mem_fetch* new_mf);

private:
   hpcl_rec_comp_buffer* m_rec_comp_buffer;
public:
   void set_rec_comp_buffer(hpcl_rec_comp_buffer* rec_comp_buffer);

private:
   void merge_mf(mem_fetch* new_mf, mem_fetch* prev_mf);


//added by kh(062916)
public:
   int speculate_rec_lwm_method(mem_fetch* new_mf, mem_fetch* prev_mf);
   void clear_rec_comp_info(mem_fetch* new_mf);

//added by kh(071516)
public:
   void rec_comp_algo1_proc(mem_fetch* new_mf);
   mem_fetch* rec_comp_method_for_intra_sm(mem_fetch* new_mf, mem_fetch* prev_mf);
   void rec_comp_algo2_proc(mem_fetch* new_mf);
   mem_fetch* rec_comp_method_for_inter_sm(mem_fetch* new_mf);
   void rec_comp_algo3_proc(mem_fetch* new_mf);
   void rec_comp_algo4_proc(mem_fetch* new_mf);

//added by kh(072716)
private:
   hpcl_comp_buffer* m_comp_buffer;
public:
   void set_comp_buffer(hpcl_comp_buffer* comp_buffer);
//





};

#endif /* NETWORKSTAT_H_ */
