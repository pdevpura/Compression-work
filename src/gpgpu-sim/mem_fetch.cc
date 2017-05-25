// Copyright (c) 2009-2011, Tor M. Aamodt
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "mem_fetch.h"
#include "mem_latency_stat.h"
#include "shader.h"
#include "visualizer.h"
#include "gpu-sim.h"

unsigned mem_fetch::sm_next_mf_request_uid=1;


#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

mem_fetch::mem_fetch( const mem_access_t &access, 
                      const warp_inst_t *inst,
                      unsigned ctrl_size, 
                      unsigned wid,
                      unsigned sid, 
                      unsigned tpc, 
                      const class memory_config *config )
{
   m_request_uid = sm_next_mf_request_uid++;
   m_access = access;
   if( inst ) { 
       m_inst = *inst;
       assert( wid == m_inst.warp_id() );
   }
   m_data_size = access.get_size();
   m_ctrl_size = ctrl_size;
   m_sid = sid;
   m_tpc = tpc;
   m_wid = wid;
   config->m_address_mapping.addrdec_tlx(access.get_addr(),&m_raw_addr);
   m_partition_addr = config->m_address_mapping.partition_address(access.get_addr());
   m_type = m_access.is_write()?WRITE_REQUEST:READ_REQUEST;
   m_timestamp = gpu_sim_cycle + gpu_tot_sim_cycle;
   m_timestamp2 = 0;
   m_status = MEM_FETCH_INITIALIZED;
   m_status_change = gpu_sim_cycle + gpu_tot_sim_cycle;
   m_mem_config = config;
   icnt_flit_size = config->icnt_flit_size;

   //added by kh(070715)
   m_status_timestamp.resize(END_STATUS+1,0);
   ///

   //added by kh(051816)
   m_loc_dict_2B = NULL;
   m_loc_dict_4B = NULL;
   m_loc_dict_8B = NULL;
   ///

   //added by kh(070216)
   int MAX_DATA_REMAP_TYPE_NO = g_hpcl_comp_config.hpcl_data_remap_function.size();
   //std::vector<unsigned char> tmp;
   //for(int i = 0; i < MAX_DATA_REMAP_TYPE_NO; i++)	{m_trans_data.push_back(tmp); }
   m_trans_data.resize(MAX_DATA_REMAP_TYPE_NO);
   m_trans_data_type.resize(MAX_DATA_REMAP_TYPE_NO, NO_DATA_TYPE);
   m_trans_loc_dict_2B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_trans_loc_dict_4B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_trans_loc_dict_8B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_comp_trans_data_bits_2B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_comp_trans_data_bits_4B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_comp_trans_data_bits_8B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   ///
   //added by kh(072016)
   m_remap_op_bits.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_remap_op_types.resize(MAX_DATA_REMAP_TYPE_NO);
   ///
   //added by kh(072816)
   m_approx_op.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_approx_exception.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   ///

   //added by kh(080416)
   //m_remap_op_locs.resize(MAX_DATA_REMAP_TYPE_NO);
   ///

   m_final_packet_size = -1;

   m_comp_data_size_2B = -1;
   m_comp_data_size_4B = -1;
   m_comp_data_size_8B = -1;
   m_comp_data_bits_2B = -1;
   m_comp_data_bits_4B = -1;
   m_comp_data_bits_8B = -1;

   m_rec_comp_merged_pkt_bits = 0;
   m_rec_comp_data_bits = 0;

   //added by kh(070416)
   m_dsc_comp_res = -1;
   m_dsc_comp_data_type = COMP_DATA_TYPE::NO_DATA_TYPE;

   //deleted by kh(073016)
   /*
   m_lwm_comp_ds_bits.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_lwm_comp_ds_bits_remap_type1.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_lwm_comp_ds_bits_remap_type2.resize(MAX_DATA_REMAP_TYPE_NO+1);
   */

   //added by kh(073016)
   unsigned res_no = 0;
   if(g_hpcl_comp_config.hpcl_comp_word_size == 2
   || g_hpcl_comp_config.hpcl_comp_word_size == 4
   || g_hpcl_comp_config.hpcl_comp_word_size == 8) {
     res_no = 1;
   } else if(g_hpcl_comp_config.hpcl_comp_word_size == 6) {
     res_no = 2;
   } else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {
     res_no = 3;
   } else {
     assert(0);
   }

   m_lwm_comp_ds_bits.resize(res_no);
   m_lwm_comp_ds_bits_remap_type1.resize(res_no);
   m_lwm_comp_ds_bits_remap_type2.resize(res_no);
   ///

   m_dsc_compressed_bits_only = 0;
   ///

   //added by kh(070616)
   m_comp_ds_status_list.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_dsc_compressed_bits_only_list.resize(MAX_DATA_REMAP_TYPE_NO+1, 0);
   ///


   m_algo_type = NO_COMP;

   //added by kh(082616)
   //m_cache_request_status = (unsigned)-1;
   //m_org_inst = NULL;
   ///

   //added by kh(090116)
   m_comp_es_byte = 0;
   m_comp_ed_byte = 0;
   m_dsc_comp_es_byte = 0;
   m_dsc_comp_ed_byte = 0;
   m_lwm_comp_es_byte.resize(3,0);
   m_lwm_comp_ed_byte.resize(3,0);
   //m_lwm_compressed_bits_only = 0;
   ///

   //added by kh(021817)
   m_is_chartype = false;
   ///
}

//added by abpd (050516)
mem_fetch::mem_fetch(enum mf_type type,
		     unsigned tpc,
		     unsigned size,
		     unsigned wid,
		     unsigned sid)
{
   m_type = type;
   m_tpc = tpc;
   m_sid= sid;
   m_wid=wid;
   m_timestamp = gpu_sim_cycle + gpu_tot_sim_cycle;
   //m_ctrl_size = size;
   m_status_timestamp.resize(END_STATUS+1,0);
   //m_compress_data.resize(4*size);
   m_comp_data_size = (4*size);

   m_data_size = 0;
   m_ctrl_size = 8;

   //added by kh(051816)
   m_loc_dict_2B = NULL;
   m_loc_dict_4B = NULL;
   m_loc_dict_8B = NULL;
   ///

   //added by kh(070216)
   int MAX_DATA_REMAP_TYPE_NO = g_hpcl_comp_config.hpcl_data_remap_function.size();
   //std::vector<unsigned char> tmp;
   //for(int i = 0; i < MAX_DATA_REMAP_TYPE_NO; i++)	{m_trans_data.push_back(tmp); }
   m_trans_data.resize(MAX_DATA_REMAP_TYPE_NO);
   m_trans_data_type.resize(MAX_DATA_REMAP_TYPE_NO, NO_DATA_TYPE);
   m_trans_loc_dict_2B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_trans_loc_dict_4B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_trans_loc_dict_8B.resize(MAX_DATA_REMAP_TYPE_NO, NULL);
   m_comp_trans_data_bits_2B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_comp_trans_data_bits_4B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_comp_trans_data_bits_8B.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   ///
   //added by kh(072016)
   m_remap_op_bits.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_remap_op_types.resize(MAX_DATA_REMAP_TYPE_NO);
   ///
   //added by kh(072816)
   m_approx_op.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   m_approx_exception.resize(MAX_DATA_REMAP_TYPE_NO, 0);
   ///


   //added by kh(062316)
   m_final_packet_size = -1;
   ///


   m_comp_data_size_2B = -1;
   m_comp_data_size_4B = -1;
   m_comp_data_size_8B = -1;
   m_comp_data_bits_2B = -1;
   m_comp_data_bits_4B = -1;
   m_comp_data_bits_8B = -1;


   m_rec_comp_merged_pkt_bits = 0;
   m_rec_comp_data_bits = 0;

   //added by kh(070416)
   m_dsc_comp_res = -1;
   m_dsc_comp_data_type = COMP_DATA_TYPE::NO_DATA_TYPE;
   /*
   m_lwm_comp_ds_bits.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_lwm_comp_ds_bits_remap_type1.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_lwm_comp_ds_bits_remap_type2.resize(MAX_DATA_REMAP_TYPE_NO+1);
   */

   //added by kh(073016)
   unsigned res_no = 0;
   if(g_hpcl_comp_config.hpcl_comp_word_size == 2
   || g_hpcl_comp_config.hpcl_comp_word_size == 4
   || g_hpcl_comp_config.hpcl_comp_word_size == 8) {
     res_no = 1;
   } else if(g_hpcl_comp_config.hpcl_comp_word_size == 6) {
     res_no = 2;
   } else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {
     res_no = 3;
   } else {
     assert(0);
   }

   m_lwm_comp_ds_bits.resize(res_no);
   m_lwm_comp_ds_bits_remap_type1.resize(res_no);
   m_lwm_comp_ds_bits_remap_type2.resize(res_no);
   ///

   m_dsc_compressed_bits_only = 0;
   ///


   //added by kh(070616)
   m_comp_ds_status_list.resize(MAX_DATA_REMAP_TYPE_NO+1);
   m_dsc_compressed_bits_only_list.resize(MAX_DATA_REMAP_TYPE_NO+1, 0);
   ///

   m_algo_type = NO_COMP;

   //added by kh(082616)
   //m_cache_request_status = -1;
   //m_org_inst = NULL;
   ///

   //added by kh(090116)
   m_comp_es_byte = 0;
   m_comp_ed_byte = 0;
   m_dsc_comp_es_byte = 0;
   m_dsc_comp_ed_byte = 0;
   m_lwm_comp_es_byte.resize(3,0);
   m_lwm_comp_ed_byte.resize(3,0);
   //m_lwm_compressed_bits_only = 0;
   ///

   //added by kh(021817)
   m_is_chartype = false;
   ///

}
///

mem_fetch::~mem_fetch()
{
    m_status = MEM_FETCH_DELETED;
}

#define MF_TUP_BEGIN(X) static const char* Status_str[] = {
#define MF_TUP(X) #X
#define MF_TUP_END(X) };
#include "mem_fetch_status.tup"
#undef MF_TUP_BEGIN
#undef MF_TUP
#undef MF_TUP_END

void mem_fetch::print( FILE *fp, bool print_inst ) const
{
    if( this == NULL ) {
        fprintf(fp," <NULL mem_fetch pointer>\n");
        return;
    }
    fprintf(fp,"  mf: uid=%6u, sid%02u:w%02u, part=%u, ", m_request_uid, m_sid, m_wid, m_raw_addr.chip );
    m_access.print(fp);
    if( (unsigned)m_status < NUM_MEM_REQ_STAT ) 
       fprintf(fp," status = %s (%llu), ", Status_str[m_status], m_status_change );
    else
       fprintf(fp," status = %u??? (%llu), ", m_status, m_status_change );
    if( !m_inst.empty() && print_inst ) m_inst.print(fp);
    else fprintf(fp,"\n");
}

void mem_fetch::set_status( enum mem_fetch_status status, unsigned long long cycle ) 
{
    m_status = status;
    m_status_change = cycle;

    //added by kh(070715)
    assert(m_status_timestamp.size() > status);
    m_status_timestamp[status] = cycle;
    ///
}

bool mem_fetch::isatomic() const
{
   if( m_inst.empty() ) return false;
   return m_inst.isatomic();
}

void mem_fetch::do_atomic()
{
    m_inst.do_atomic( m_access.get_warp_mask() );
}

bool mem_fetch::istexture() const
{
    if( m_inst.empty() ) return false;
    return m_inst.space.get_type() == tex_space;
}

bool mem_fetch::isconst() const
{ 
    if( m_inst.empty() ) return false;
    return (m_inst.space.get_type() == const_space) || (m_inst.space.get_type() == param_space_kernel);
}

/// Returns number of flits traversing interconnect. simt_to_mem specifies the direction
unsigned mem_fetch::get_num_flits(bool simt_to_mem){
	unsigned sz=0;
	// If atomic, write going to memory, or read coming back from memory, size = ctrl + data. Else, only ctrl
	if( isatomic() || (simt_to_mem && get_is_write()) || !(simt_to_mem || get_is_write()) )
		sz = size();
	else
		sz = get_ctrl_size();

	return (sz/icnt_flit_size) + ( (sz % icnt_flit_size)? 1:0);
}


//added by kh(070715)

void mem_fetch::set_timestamp(enum mem_fetch_status status, unsigned long long cycle)
{
    assert(m_status_timestamp.size() > status);
    m_status_timestamp[status] = cycle;
}

unsigned long long mem_fetch::get_timestamp(enum mem_fetch_status status)
{
    assert(m_status_timestamp.size() > status);
    return m_status_timestamp[status];
}
///


//added by kh(030816)
unsigned char* mem_fetch::config_real_data(unsigned data_size)
{
  m_real_data.clear();
  m_real_data.resize(data_size, 0);
  return &m_real_data[0];
}

unsigned char mem_fetch::get_real_data(unsigned data_index)
{
  assert(m_real_data.size() > data_index);
  return m_real_data[data_index];
}

void mem_fetch::set_real_data(unsigned data_index, unsigned char data)
{
  m_real_data[data_index] = data;
}

void mem_fetch::get_real_data_stream(unsigned flit_seq, std::vector<unsigned char>& dest, int stream_size)
{
  assert(m_real_data.size() >= (flit_seq+1)*stream_size);
  for(int i = 0; i < stream_size; i++) {
      dest[i] = m_real_data[flit_seq*stream_size+i];
  }
}

unsigned mem_fetch::get_real_data_size()
{
  return m_real_data.size();
}
///

//added by kh(031716)
std::vector<unsigned char>& mem_fetch::get_real_data_ptr()
{
  return m_real_data;
}
///

void mem_fetch::clear_real_data()
{
  m_real_data.clear();
}


void mem_fetch::set_comp_data_size(unsigned comp_data_size) {
  m_comp_data_size = comp_data_size;
}
unsigned mem_fetch::get_comp_data_size() {
  return m_comp_data_size;
}

void mem_fetch::set_comp_res(unsigned comp_res)	{
  m_comp_res = comp_res;
}
unsigned mem_fetch::get_comp_res() {
  return m_comp_res;
}

//added by kh(060616)
unsigned char* mem_fetch::config_trans_data(unsigned data_size, unsigned index)
{
  m_trans_data[index].clear();
  m_trans_data[index].resize(data_size, 0);
  return &m_trans_data[index][0];
}
unsigned char mem_fetch::get_trans_data(unsigned data_index, unsigned index)
{
  assert(m_trans_data[index].size() > data_index);
  return m_trans_data[index][data_index];
}
void mem_fetch::set_trans_data(unsigned data_index, unsigned char data, unsigned index)
{
  assert(m_trans_data[index].size() > data_index);
  m_trans_data[index][data_index] = data;
}
unsigned mem_fetch::get_trans_data_size(unsigned index)
{
  return m_trans_data[index].size();
}
std::vector<unsigned char>& mem_fetch::get_trans_data_ptr(unsigned index)
{
  return m_trans_data[index];
}
unsigned mem_fetch::get_trans_data_no()
{
  return m_trans_data.size();
}


void mem_fetch::commit_trans_data_info(unsigned res, unsigned index)
{
  std::vector<unsigned char> tmp1 = m_real_data;
  m_real_data = m_trans_data[index];
  m_trans_data[index] = tmp1;

  if(res == 2 || res == 14) {
    void* tmp2 = m_loc_dict_2B;
    m_loc_dict_2B = m_trans_loc_dict_2B[index];
    m_trans_loc_dict_2B[index] = tmp2;
  }

  if(res == 4 || res == 14) {
    void* tmp2 = m_loc_dict_4B;
    m_loc_dict_4B = m_trans_loc_dict_4B[index];
    m_trans_loc_dict_4B[index] = tmp2;
  }

  if(res == 8 || res == 14) {
    void* tmp2 = m_loc_dict_8B;
    m_loc_dict_8B = m_trans_loc_dict_8B[index];
    m_trans_loc_dict_8B[index] = tmp2;
  }
//  unsigned tmp3 = m_comp_data_size;
//  m_comp_data_size = m_comp_trans_data_size;
//  m_comp_trans_data_size = tmp3;
}
void* mem_fetch::get_trans_loc_dict(unsigned res, unsigned index)
{
  if(res == 2)		return m_trans_loc_dict_2B[index];
  else if(res == 4)	return m_trans_loc_dict_4B[index];
  else if(res == 8)	return m_trans_loc_dict_8B[index];
  else	assert(0);

  return NULL;
}
void mem_fetch::set_trans_loc_dict(void* trans_loc_dict, unsigned res, unsigned index)	{
  if(res == 2) 		m_trans_loc_dict_2B[index] = trans_loc_dict;
  else if(res == 4) 	m_trans_loc_dict_4B[index] = trans_loc_dict;
  else if(res == 8) 	m_trans_loc_dict_8B[index] = trans_loc_dict;
  else	assert(0);
}
void mem_fetch::set_trans_comp_data_bits(unsigned comp_bits_size, unsigned res, unsigned index) {
  if(res == 2)		m_comp_trans_data_bits_2B[index] = comp_bits_size;
  else if(res == 4)	m_comp_trans_data_bits_4B[index] = comp_bits_size;
  else if(res == 8)	m_comp_trans_data_bits_8B[index] = comp_bits_size;
  else	assert(0);
}
unsigned mem_fetch::get_trans_comp_data_bits(unsigned res, unsigned index) {
  if(res == 2)		return m_comp_trans_data_bits_2B[index];
  else if(res == 4)	return m_comp_trans_data_bits_4B[index];
  else if(res == 8)	return m_comp_trans_data_bits_8B[index];
  else	assert(0);
  return 0;
}
unsigned mem_fetch::get_trans_loc_dict_no() {
  return m_trans_loc_dict_2B.size();
}



/*
//added by kh(070216)
unsigned char* mem_fetch::config_retrans_data(unsigned data_size)
{
  m_retrans_data.clear();
  m_retrans_data.resize(data_size, 0);
  return &m_retrans_data[0];
}
unsigned char mem_fetch::get_retrans_data(unsigned data_index)
{
  assert(m_retrans_data.size() > data_index);
  return m_retrans_data[data_index];
}
void mem_fetch::set_retrans_data(unsigned data_index, unsigned char data)
{
  assert(m_retrans_data.size() > data_index);
  m_retrans_data[data_index] = data;
}
unsigned mem_fetch::get_retrans_data_size()
{
  return m_retrans_data.size();
}
std::vector<unsigned char>& mem_fetch::get_retrans_data_ptr()
{
  return m_retrans_data;
}
void mem_fetch::commit_retrans_data_info(unsigned res)
{
  std::vector<unsigned char> tmp1 = m_real_data;
  m_real_data = m_retrans_data;
  m_retrans_data = tmp1;

  if(res == 2 || res == 14) {
    void* tmp2 = m_loc_dict_2B;
    m_loc_dict_2B = m_retrans_loc_dict_2B;
    m_retrans_loc_dict_2B = tmp2;
  }

  if(res == 4 || res == 14) {
    void* tmp2 = m_loc_dict_4B;
    m_loc_dict_4B = m_retrans_loc_dict_4B;
    m_retrans_loc_dict_4B = tmp2;
  }

  if(res == 8 || res == 14) {
    void* tmp2 = m_loc_dict_8B;
    m_loc_dict_8B = m_retrans_loc_dict_8B;
    m_retrans_loc_dict_8B = tmp2;
  }
//  unsigned tmp3 = m_comp_data_size;
//  m_comp_data_size = m_comp_trans_data_size;
//  m_comp_trans_data_size = tmp3;
}
void* mem_fetch::get_retrans_loc_dict(unsigned res)
{
  if(res == 2)	return m_retrans_loc_dict_2B;
  else if(res == 4)	return m_retrans_loc_dict_4B;
  else if(res == 8)	return m_retrans_loc_dict_8B;
  else	assert(0);

  return NULL;
}
void mem_fetch::set_retrans_loc_dict(void* retrans_loc_dict, unsigned res)	{
  if(res == 2) 		m_retrans_loc_dict_2B = retrans_loc_dict;
  else if(res == 4) 	m_retrans_loc_dict_4B = retrans_loc_dict;
  else if(res == 8) 	m_retrans_loc_dict_8B = retrans_loc_dict;
  else	assert(0);
}
void mem_fetch::set_retrans_comp_data_bits(unsigned comp_bits_size, unsigned res) {
  if(res == 2)		m_comp_retrans_data_bits_2B = comp_bits_size;
  else if(res == 4)	m_comp_retrans_data_bits_4B = comp_bits_size;
  else if(res == 8)	m_comp_retrans_data_bits_8B = comp_bits_size;
  else	assert(0);
}
unsigned mem_fetch::get_retrans_comp_data_bits(unsigned res) {
  if(res == 2)		return m_comp_retrans_data_bits_2B;
  else if(res == 4)	return m_comp_retrans_data_bits_4B;
  else if(res == 8)	return m_comp_retrans_data_bits_8B;
  else	assert(0);
  return 0;
}
*/

//added by kh(062316)
void mem_fetch::set_final_packet_size(int final_packet_size) {
  m_final_packet_size = final_packet_size;
}

int mem_fetch::get_final_packet_size() {
  return m_final_packet_size;
}
///

//added by kh(062416)
void mem_fetch::set_rec_comp_merged_pkt_bits(unsigned rec_comp_merged_pkt_bits) {
  m_rec_comp_merged_pkt_bits = rec_comp_merged_pkt_bits;
}
void mem_fetch::add_rec_comp_merged_pkt_bits(unsigned rec_comp_merged_pkt_bits) {
  m_rec_comp_merged_pkt_bits += rec_comp_merged_pkt_bits;
}
unsigned mem_fetch::get_rec_comp_merged_pkt_bits() {
  return m_rec_comp_merged_pkt_bits;
}

//added by kh(062916)
void mem_fetch::set_rec_comp_data_bits(unsigned rec_comp_data_bits) {
  m_rec_comp_data_bits = rec_comp_data_bits;
}
void mem_fetch::add_rec_comp_data_bits(unsigned rec_comp_data_bits) {
  m_rec_comp_data_bits += rec_comp_data_bits;
}
unsigned mem_fetch::get_rec_comp_data_bits() {
  return m_rec_comp_data_bits;
}



void mem_fetch::set_comp_data_bits(unsigned comp_data_bits, int res) {
  if(res == -1) {
    m_comp_data_bits = comp_data_bits;
  } else if(res == 2) {
    m_comp_data_bits_2B = comp_data_bits;
  } else if(res == 4) {
    m_comp_data_bits_4B = comp_data_bits;
  } else if(res == 8) {
    m_comp_data_bits_8B = comp_data_bits;
  } else assert(0);
}
unsigned mem_fetch::get_comp_data_bits(int res) {
  //std::cout << "mf " << this->get_request_uid() << " " << this->get_is_write() << " " << this->get_access_type() << std::endl;
  if(res == -1) {

    if(m_comp_data_bits <= 0) {
      this->print(stdout);
    }
    assert(m_comp_data_bits > 0);
    return (unsigned) m_comp_data_bits;
  }
  else {
    if(res == 2) {
      assert(m_comp_data_bits_2B > 0);
      return (unsigned) m_comp_data_bits_2B;
    }
    else if(res == 4) {
      assert(m_comp_data_bits_4B > 0);
      return m_comp_data_bits_4B;
    }
    else if(res == 8) {
      assert(m_comp_data_bits_8B > 0);
      return m_comp_data_bits_8B;
    }
    else assert(0);
  }
  assert(0);
  return 0;
}
///

void mem_fetch::add_merged_mfs(std::vector<mem_fetch*>& mfs) {
  for(unsigned i = 0; i < mfs.size(); i++)	m_merged_mfs.push_back(mfs[i]);
}

void mem_fetch::del_merged_mfs() {
  m_merged_mfs.clear();
}

std::vector<mem_fetch*>& mem_fetch::get_merged_mfs() {
  return m_merged_mfs;
}

bool mem_fetch::has_merged_mem_fetch() {
  return (m_merged_mfs.size() > 0)? true: false;
}

unsigned mem_fetch::get_merged_mf_no() {
  return m_merged_mfs.size();
}

mem_fetch* mem_fetch::get_merged_mf(unsigned index) {
  assert(m_merged_mfs.size() > index);
  return m_merged_mfs[index];
}

//unsigned mem_fetch::get_comp_size_merged_mem_fetch() {
//  unsigned ret = 0;
//
//  if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::BASIC_APPEND) {
//    DEBUG_PRINT("mf %u\n", get_request_uid());
//    //printf("\tctrl0 %u, comp_size0 %u\n", get_ctrl_size(), get_comp_data_size());
//    for(unsigned i = 0; i < m_merged_mfs.size(); i++) {
//      ret = ret + (m_merged_mfs[i]->get_ctrl_size()+m_merged_mfs[i]->get_comp_data_size());
//      //printf("\tctrl %u, comp_size %u\n", m_merged_mfs[i]->get_ctrl_size(),m_merged_mfs[i]->get_comp_data_size());
//    }
//    //printf("\tret %u\n", ret);
//  } else if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP
//    || g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP_FB) {
//
//    //printf("mem_fetch::mf %u ", get_request_uid());
//    for(unsigned i = 0; i < m_merged_mfs.size(); i++) {
//      ret = ret + (m_merged_mfs[i]->get_ctrl_size()*8+m_merged_mfs[i]->get_inter_comp_data_bits());
//      //printf("\tmem_fetch:: mf %u, intra_comp_data_bits %u, inter_comp_data_bits %u\n",m_merged_mfs[i]->get_request_uid(), m_merged_mfs[i]->get_comp_data_size()*8, m_merged_mfs[i]->get_inter_comp_data_bits());
//    }
//    ret = ceil((double)ret/8.0);
//    //ret = ceil((double)m_merged_mf_comp_size_bits/8.0);
//    DEBUG_PRINT("\tfinal_inter_comp_data_bytes %u\n", ret);
//  } else assert(0);
//
//  return ret;
//}


void mem_fetch::add_merged_mf(mem_fetch* mf) {
  m_merged_mfs.push_back(mf);
}

void mem_fetch::add_merged_mf_to_same_SM(mem_fetch* mf) {
  m_merged_mfs_to_same_SM.push_back(mf);
}
unsigned mem_fetch::get_merged_mf_to_same_SM_no() {
  return m_merged_mfs_to_same_SM.size();
}
mem_fetch* mem_fetch::get_merged_mf_to_same_SM(unsigned index) {
  assert(m_merged_mfs_to_same_SM.size() > index);
  return m_merged_mfs_to_same_SM[index];
}

void mem_fetch::set_merged_mfs_to_same_SM(std::vector<mem_fetch*>& merged_mfs_to_same_SM) {
  m_merged_mfs_to_same_SM = merged_mfs_to_same_SM;
}
std::vector<mem_fetch*>& mem_fetch::get_merged_mfs_to_same_SM() {
  return m_merged_mfs_to_same_SM;
}



//void mem_fetch::set_merged_mf_comp_size_bits(unsigned merged_mf_comp_size_bits) {
//  m_merged_mf_comp_size_bits = merged_mf_comp_size_bits;
//}

void* mem_fetch::get_loc_dict(unsigned size) {
  if(size == sizeof(unsigned short))	return m_loc_dict_2B;
  else if(size == sizeof(unsigned int))	return m_loc_dict_4B;
  else if(size == sizeof(unsigned long long))	return m_loc_dict_8B;
  else					assert(0);
  return NULL;
}

void mem_fetch::set_loc_dict(unsigned size, void* loc_dict) {
  if(size == sizeof(unsigned short))	m_loc_dict_2B = loc_dict;
  else if(size == sizeof(unsigned int))	m_loc_dict_4B = loc_dict;
  else if(size == sizeof(unsigned long long))	m_loc_dict_8B = loc_dict;
  else					assert(0);
}

//void mem_fetch::set_inter_comp_data_bits(unsigned comp_data_size) {
//  //printf("\tset_inter_comp_data_bits:: mf %u, comp_data_size %u\n",this->get_request_uid(),comp_data_size);
//  m_inter_comp_data_size = comp_data_size;
//}
//
//unsigned mem_fetch::get_inter_comp_data_bits() {
//  return m_inter_comp_data_size;
//}
//
//void mem_fetch::set_inter_comp_matching_word_no(unsigned matching_word_no) {
//  m_inter_comp_matching_word_no = matching_word_no;
//}
//unsigned mem_fetch::get_inter_comp_matching_word_no() {
//  return m_inter_comp_matching_word_no;
//}

void mem_fetch::set_comp_data_size(unsigned comp_data_size, unsigned res) {
  if(res == 2) m_comp_data_size_2B = comp_data_size;
  else if(res == 4) m_comp_data_size_4B = comp_data_size;
  else if(res == 8) m_comp_data_size_8B = comp_data_size;
  else assert(0);
}
unsigned mem_fetch::get_comp_data_size(unsigned res) {
  if(res == 2) {
    assert(m_comp_data_size_2B > 0);
    return (unsigned) m_comp_data_size_2B;
  } else if(res == 4) {
    assert(m_comp_data_size_4B > 0);
    return (unsigned) m_comp_data_size_4B;
  } else if(res == 8) {
    assert(m_comp_data_size_8B > 0);
    return (unsigned) m_comp_data_size_8B;
  }
  else assert(0);

  return 0;
}

#ifdef REQ_COAL_MODULE
void mem_fetch::add_merged_req_mfs(std::vector<mem_fetch*>& mfs) {
  for(unsigned i = 0; i < mfs.size(); i++)	m_merged_req_mfs.push_back(mfs[i]);
}
void mem_fetch::del_merged_req_mfs() {
  m_merged_req_mfs.clear();
}

std::vector<mem_fetch*>& mem_fetch::get_merged_req_mfs() {
  return m_merged_req_mfs;
}

bool mem_fetch::has_merged_req_mfs() {
  return (m_merged_req_mfs.size() > 0)? true: false;
}

unsigned mem_fetch::get_size_merged_req_mfs() {
  unsigned ret = 0;
  for(unsigned i = 0; i < m_merged_req_mfs.size(); i++) {
    ret = ret + (m_merged_req_mfs[i]->get_ctrl_size());
  }
  return ret;
}
#endif


//added by kh(070416)
void mem_fetch::set_dsc_comp_res(int dsc_comp_res) {
  m_dsc_comp_res = dsc_comp_res;
}
int mem_fetch::get_dsc_comp_res() {
  return m_dsc_comp_res;
}
void mem_fetch::set_dsc_comp_data_type(enum COMP_DATA_TYPE type) {
  m_dsc_comp_data_type = type;
}
enum mem_fetch::COMP_DATA_TYPE mem_fetch::get_dsc_comp_data_type() {
  return m_dsc_comp_data_type;
}
void mem_fetch::print_dsc_comp_data_type(enum COMP_DATA_TYPE type) {
  if(type == ORG_DATA)	printf("DataType : ORG_DATA\n");
  else			printf("DataType : REMAPPED_DATA_%d\n", type);
}
///

//added by kh(070416)
void mem_fetch::set_comp_ds_status_size(unsigned size, int init_val, enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	m_comp_ds_status.resize(size,init_val);
  else		    		m_comp_ds_status_list[type].resize(size,init_val);
}
void mem_fetch::set_comp_ds_status(unsigned index, unsigned comp_status, enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	m_comp_ds_status[index] = comp_status;
  else				m_comp_ds_status_list[type][index] = comp_status;
}
unsigned mem_fetch::get_comp_ds_status_size(enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	return m_comp_ds_status.size();
  else				return m_comp_ds_status_list[type].size();
}
unsigned mem_fetch::get_comp_ds_status(unsigned index, enum COMP_DATA_TYPE type) {
  //assert(m_comp_ds_status[index] >= 0);
  if(type == NO_DATA_TYPE)	return m_comp_ds_status[index];
  else				return m_comp_ds_status_list[type][index];
}
bool mem_fetch::has_uncompressed_ds(enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE) {
    for(int i = 0; i < m_comp_ds_status.size(); i++) {
      if(m_comp_ds_status[i] == 0) return true;
    }
    return false;
  } else {
    for(int i = 0; i < m_comp_ds_status_list[type].size(); i++) {
      if(m_comp_ds_status_list[type][i] == 0) return true;
    }
    return false;
  }
  assert(0);
  return false;
}
std::vector<int>& mem_fetch::get_comp_ds_status(enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	return m_comp_ds_status;
  else				return m_comp_ds_status_list[type];
}


void mem_fetch::set_dsc_compressed_bits_only(unsigned compressed_bits_only, enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	m_dsc_compressed_bits_only = compressed_bits_only;
  else				m_dsc_compressed_bits_only_list[type] = compressed_bits_only;
}
unsigned mem_fetch::get_dsc_compressed_bits_only(enum COMP_DATA_TYPE type) {
  if(type == NO_DATA_TYPE)	return m_dsc_compressed_bits_only;
  else				return m_dsc_compressed_bits_only_list[type];
}

/*
void mem_fetch::set_lwm_compressed_bits_only(unsigned compressed_bits_only, enum COMP_DATA_TYPE type=NO_DATA_TYPE) {
  m_lwm_compressed_bits_only = compressed_bits_only;
}
unsigned mem_fetch::get_lwm_compressed_bits_only(enum COMP_DATA_TYPE type=NO_DATA_TYPE) {
  return m_lwm_compressed_bits_only;
}
*/



void mem_fetch::set_comp_ds_bits_size(unsigned size, double init_val) {
  m_comp_ds_bits.resize(size, init_val);
}
void mem_fetch::set_comp_ds_bits(unsigned index, unsigned comp_ds_bits) {
  m_comp_ds_bits[index] = comp_ds_bits;
}
unsigned mem_fetch::get_comp_ds_bits_size() {
  return m_comp_ds_bits.size();
}
unsigned mem_fetch::get_comp_ds_bits(unsigned index) {
  assert(m_comp_ds_bits[index] >= 0);
  return m_comp_ds_bits[index];
}

void mem_fetch::set_comp_ds_bits(std::vector<double>& comp_ds_bits, unsigned comp_res, enum COMP_DATA_TYPE type) {
  unsigned index = log2(comp_res)-1;
  if(type == ORG_DATA) {
    if(m_lwm_comp_ds_bits.size() <= index) {
      printf("set_comp_ds_bits: index %u m_lwm_comp_ds_bits.size() %u\n", index, m_lwm_comp_ds_bits.size());
    }
    assert(m_lwm_comp_ds_bits.size() > index);
    m_lwm_comp_ds_bits[index] = comp_ds_bits;
  } else if(type == REMAPPED_DATA_1) {
    m_lwm_comp_ds_bits_remap_type1[index] = comp_ds_bits;
  } else if(type == REMAPPED_DATA_2) {
    m_lwm_comp_ds_bits_remap_type2[index] = comp_ds_bits;
  }
}

std::vector<double>& mem_fetch::get_comp_ds_bits(unsigned comp_res, enum COMP_DATA_TYPE type) {
  unsigned index = log2(comp_res)-1;
  if(type == ORG_DATA) {
    return m_lwm_comp_ds_bits[index];
  } else if(type == REMAPPED_DATA_1) {
    return m_lwm_comp_ds_bits_remap_type1[index];
  } else if(type == REMAPPED_DATA_2) {
    return m_lwm_comp_ds_bits_remap_type2[index];
  }
  assert(0);
  return m_lwm_comp_ds_bits[index];
}

void mem_fetch::print_real_data(unsigned space_gap)
{
  printf("ORG_DATA");
  print_data(m_real_data, space_gap);
  printf("\n");
}


void mem_fetch::print_data_all(unsigned space_gap)
{
  printf("ORG_DATA");
  print_data(m_real_data, space_gap);
  printf("\n");

  for(int i = 0; i < m_trans_data.size(); i++) {

    if(m_trans_data_type[i] == REMAPPED_DATA_1)		printf("REMAPPED_DATA_1");
    else if(m_trans_data_type[i] == REMAPPED_DATA_2)	printf("REMAPPED_DATA_2");
    else if(m_trans_data_type[i] == REMAPPED_DATA_3)	printf("REMAPPED_DATA_3");
    else if(m_trans_data_type[i] == REMAPPED_DATA_4)	printf("REMAPPED_DATA_4");
    else if(m_trans_data_type[i] == REMAPPED_DATA_5)	printf("REMAPPED_DATA_5");
    else if(m_trans_data_type[i] == REMAPPED_DATA_6)	printf("REMAPPED_DATA_6");
    else {
      printf("m_trans_data_type[%d] = %d\n", i, m_trans_data_type[i]);
      assert(0);
    }
    print_data(m_trans_data[i], space_gap);
    printf("\n");
  }
}

void mem_fetch::print_data(std::vector<unsigned char>& data, unsigned space_gap)
{
  int data_size = data.size();
  printf(" (%d) = ", data_size);
  for(int i = 0; i < data_size; i++) {
    if(i % space_gap == 0) printf(" ");
    printf("%02x", data[i]);
  }
  printf("\n");
}
/*
void mem_fetch::print_data(enum COMP_DATA_TYPE data_type)
{
  std::vector<unsigned char>* data = NULL;
  if(data_type == ORG_DATA)	{
    printf("ORG_DATA");
    data = &m_real_data;
  } else if(data_type == REMAPPED_DATA_1)  {
    printf("REMAP_DATA_1");
    data = &m_trans_data[0];
  } else if(data_type == REMAPPED_DATA_2)  {
    printf("REMAP_DATA_2");
    data = &m_trans_data[1];
  } else if(data_type == REMAPPED_DATA_3)  {
    printf("REMAP_DATA_3");
    data = &m_trans_data[2];
  } else if(data_type == REMAPPED_DATA_4)  {
    printf("REMAP_DATA_4");
    data = &m_trans_data[3];
  }
  else	assert(0);

  int data_size = data->size();
  printf(" (%d) = ", data_size);
  for(int i = 0; i < data_size; i++) {
    if(i % 8 == 0) printf(" ");
    printf("%02x", data->at(i));
  }
  printf("\n");
}
*/

void mem_fetch::set_comp_algo_type(enum COMP_ALGO_TYPE algo_type)
{
  m_algo_type = algo_type;
}
enum mem_fetch::COMP_ALGO_TYPE mem_fetch::get_comp_algo_type()
{
  return m_algo_type;
}

void mem_fetch::set_redund_nibble_rate(double redund_nibble_rate)
{
  m_redund_nibble_rate = redund_nibble_rate;
}
double mem_fetch::get_redund_nibble_rate()
{
  return m_redund_nibble_rate;
}

std::vector<unsigned char>* mem_fetch::config_remapped_data(unsigned index, unsigned data_size)
{
  assert(m_trans_data.size() > index);
  m_trans_data[index].clear();
  m_trans_data[index].resize(data_size, 0);
  return &m_trans_data[index];
}

void mem_fetch::set_remapped_data_type(unsigned index, enum COMP_DATA_TYPE data_type)
{
  assert(m_trans_data_type.size() > index);
  m_trans_data_type[index] = data_type;
  //printf("set_remapped_data_type: mf %u, type %d\n", this->get_request_uid(), data_type);
}

enum mem_fetch::COMP_DATA_TYPE mem_fetch::get_remapped_data_type(unsigned index)
{
  assert(m_trans_data_type.size() > index);
  return m_trans_data_type[index];
}

int mem_fetch::get_remapped_data_type_index(enum COMP_DATA_TYPE data_type)
{
  int ret = -1;
  for(unsigned i = 0; i < m_trans_data_type.size(); i++) {
    if(m_trans_data_type[i] == data_type) {
      ret = i;
      break;
    }
  }
  return ret;
}


//added by kh(072016)
void mem_fetch::set_remap_op_bits(unsigned index, unsigned remap_op_bits)
{
  m_remap_op_bits[index] = remap_op_bits;
}
unsigned mem_fetch::get_remap_op_bits(unsigned index)
{
  return m_remap_op_bits[index];
}
///

//added by kh(072016)
void mem_fetch::set_remap_op_type(unsigned index, std::vector<enum mem_fetch::REMAP_OP_TYPE>& remap_op_type)
{
  m_remap_op_types[index] = remap_op_type;
}

std::vector<enum mem_fetch::REMAP_OP_TYPE>& mem_fetch::get_remap_op_type(unsigned index)
{
  return m_remap_op_types[index];
}
///

//added by kh(072816)
void mem_fetch::set_approx_op(unsigned index, unsigned approx_op)
{
  m_approx_op[index] = approx_op;
}
unsigned mem_fetch::get_approx_op(unsigned index)
{
  return m_approx_op[index];
}
///

void mem_fetch::set_approx_exception(unsigned index, unsigned approx_exception)
{
  m_approx_exception[index] = approx_exception;
}
unsigned mem_fetch::get_approx_exception(unsigned index)
{
  return m_approx_exception[index];
}

//added by kh(090116)
void mem_fetch::set_comp_es_byte(unsigned comp_es_byte) {
  m_comp_es_byte = comp_es_byte;
}
void mem_fetch::set_comp_ed_byte(unsigned comp_ed_byte) {
  m_comp_ed_byte = comp_ed_byte;
}
unsigned mem_fetch::get_comp_es_byte() {
  return m_comp_es_byte;
}
unsigned mem_fetch::get_comp_ed_byte() {
  return m_comp_ed_byte;
}
///

void mem_fetch::set_dsc_comp_es_byte(unsigned comp_es_byte) {
  m_dsc_comp_es_byte = comp_es_byte;
}
void mem_fetch::set_dsc_comp_ed_byte(unsigned comp_ed_byte) {
  m_dsc_comp_ed_byte = comp_ed_byte;
}
unsigned mem_fetch::get_dsc_comp_es_byte() {
  return m_dsc_comp_es_byte;
}
unsigned mem_fetch::get_dsc_comp_ed_byte() {
  return m_dsc_comp_ed_byte;
}



void mem_fetch::set_lwm_comp_es_byte(unsigned comp_es_byte, unsigned res) {
  if(res == 2)		m_lwm_comp_es_byte[0] = comp_es_byte;
  else if(res == 4)	m_lwm_comp_es_byte[1] = comp_es_byte;
  else if(res == 8)	m_lwm_comp_es_byte[2] = comp_es_byte;
  else assert(0);
}
void mem_fetch::set_lwm_comp_ed_byte(unsigned comp_ed_byte, unsigned res) {
  if(res == 2)		m_lwm_comp_ed_byte[0] = comp_ed_byte;
  else if(res == 4)	m_lwm_comp_ed_byte[1] = comp_ed_byte;
  else if(res == 8)	m_lwm_comp_ed_byte[2] = comp_ed_byte;
  else assert(0);
}
unsigned mem_fetch::get_lwm_comp_es_byte(unsigned res) {
  if(res == 2)		return m_lwm_comp_es_byte[0];
  else if(res == 4)	return m_lwm_comp_es_byte[1];
  else if(res == 8)	return m_lwm_comp_es_byte[2];
  else assert(0);
  return 0;
}
unsigned mem_fetch::get_lwm_comp_ed_byte(unsigned res) {
  if(res == 2)		return m_lwm_comp_ed_byte[0];
  else if(res == 4)	return m_lwm_comp_ed_byte[1];
  else if(res == 8)	return m_lwm_comp_ed_byte[2];
  else assert(0);
  return 0;
}

//added by kh(092616)
unsigned mem_fetch::get_hit_cache_compressed() {
  return m_is_hit_cache_compressed;
}

void mem_fetch::set_hit_cache_compressed(unsigned is_hit_cache_compressed) {
  m_is_hit_cache_compressed = is_hit_cache_compressed;
}
///


//added by kh(021817)
unsigned char mem_fetch::get_txt_trans_data(unsigned data_index) {
	return m_txt_trans_data[data_index];
}
void mem_fetch::set_txt_trans_data(unsigned data_index, unsigned char data) {
	m_txt_trans_data[data_index] = data;
}
unsigned mem_fetch::get_txt_trans_data_size() {
	return m_txt_trans_data.size();
}
std::vector<unsigned char>& mem_fetch::get_txt_trans_data_ptr() {
	return m_txt_trans_data;
}
void mem_fetch::set_chartype() {
	m_is_chartype = true;
}
bool mem_fetch::get_chartype() {
	return m_is_chartype;
}
///





/*
//added by kh(082616)
unsigned mem_fetch::get_cache_request_status()
{
  return m_cache_request_status;
}
void mem_fetch::set_cache_request_status(unsigned cache_request_status)
{
  m_cache_request_status = cache_request_status;
}

warp_inst_t* mem_fetch::get_org_inst()
{
  return m_org_inst;
}
void mem_fetch::set_org_inst(warp_inst_t* org_inst)
{
  m_org_inst = org_inst;
}
*/
///

