
#ifndef HPCL_REC_COMP_LWM_PL_H_
#define HPCL_REC_COMP_LWM_PL_H_

#include "mem_fetch.h"
#include "hpcl_rec_comp_lwm_pl_proc.h"
#include "hpcl_rec_comp_buffer.h"
#include <vector>



class hpcl_rec_comp_lwm_pl {
public:
  hpcl_rec_comp_lwm_pl();
  virtual ~hpcl_rec_comp_lwm_pl();

  void run();
  void push_input(mem_fetch* mf);
  mem_fetch* pop_output();
  mem_fetch* top_output();
  void create(unsigned pipeline_stage_no, unsigned max_inter_comp_size, hpcl_rec_comp_buffer* m_rec_comp_buffer, unsigned id, hpcl_comp_buffer* m_comp_buffer);


  bool has_buffer_space();

private:
  unsigned m_id;
  mem_fetch* m_input;
  mem_fetch* m_output;
  //hpcl_intra_coal_buffer* m_hpcl_intra_coal_buffer;
  //unsigned m_max_intra_coal_size;

public:
  void print();

private:
  std::vector<hpcl_rec_comp_lwm_pl_proc*> m_hpcl_rec_comp_lwm_pl_proc;





};

#endif /* NETWORKSTAT_H_ */
