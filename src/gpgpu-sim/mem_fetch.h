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

#ifndef MEM_FETCH_H
#define MEM_FETCH_H

#include "addrdec.h"
#include "../abstract_hardware_model.h"
#include <bitset>

enum mf_type {
   READ_REQUEST = 0,
   WRITE_REQUEST,
   READ_REPLY, // send to shader
   WRITE_ACK,
   // added by abpd (050516)
   CTRL_MSG,
};

#define MF_TUP_BEGIN(X) enum X {
#define MF_TUP(X) X
#define MF_TUP_END(X) };
#include "mem_fetch_status.tup"
#undef MF_TUP_BEGIN
#undef MF_TUP
#undef MF_TUP_END

//added by kh(051916)
#include "hpcl_comp_config.h"
#include <cmath>
extern hpcl_comp_config g_hpcl_comp_config;
///

//added by kh(060216)
//#define	DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif
///

//added by kh(062416)
#include "hpcl_user_define_stmts.h"


//added by kh(021817)
#include <vector>
///

class mem_fetch {
public:
    mem_fetch( const mem_access_t &access, 
               const warp_inst_t *inst,
               unsigned ctrl_size, 
               unsigned wid,
               unsigned sid, 
               unsigned tpc, 
               const class memory_config *config );
    // added by abpd (050516)
    mem_fetch(enum mf_type type,
	      unsigned tpc,
	      unsigned size,
              unsigned wid,
    	      unsigned sid);

    ~mem_fetch();



   void set_status( enum mem_fetch_status status, unsigned long long cycle );
   void set_reply() 
   { 
       assert( m_access.get_type() != L1_WRBK_ACC && m_access.get_type() != L2_WRBK_ACC );
       if( m_type==READ_REQUEST ) {
           assert( !get_is_write() );
           m_type = READ_REPLY;
       } else if( m_type == WRITE_REQUEST ) {
           assert( get_is_write() );
           m_type = WRITE_ACK;
       }
   }
   void do_atomic();

   void print( FILE *fp, bool print_inst = true ) const;

   const addrdec_t &get_tlx_addr() const { return m_raw_addr; }
   unsigned get_data_size() const { return m_data_size; }
   void     set_data_size( unsigned size ) { m_data_size=size; }
   unsigned get_ctrl_size() const { return m_ctrl_size; }
   unsigned size() const { return m_data_size+m_ctrl_size; }
   bool is_write() {return m_access.is_write();}
   void set_addr(new_addr_type addr) { m_access.set_addr(addr); }
   new_addr_type get_addr() const { return m_access.get_addr(); }
   new_addr_type get_partition_addr() const { return m_partition_addr; }
   unsigned get_sub_partition_id() const { return m_raw_addr.sub_partition; }
   bool     get_is_write() const { return m_access.is_write(); }
   unsigned get_request_uid() const { return m_request_uid; }
   unsigned get_sid() const { return m_sid; }
   unsigned get_tpc() const { return m_tpc; }
   unsigned get_wid() const { return m_wid; }
   bool istexture() const;
   bool isconst() const;
   enum mf_type get_type() const { return m_type; }
   bool isatomic() const;

   void set_return_timestamp( unsigned t ) { m_timestamp2=t; }
   void set_icnt_receive_time( unsigned t ) { m_icnt_receive_time=t; }
   unsigned get_timestamp() const { return m_timestamp; }
   unsigned get_return_timestamp() const { return m_timestamp2; }
   unsigned get_icnt_receive_time() const { return m_icnt_receive_time; }

   enum mem_access_type get_access_type() const { return m_access.get_type(); }
   const active_mask_t& get_access_warp_mask() const { return m_access.get_warp_mask(); }
   mem_access_byte_mask_t get_access_byte_mask() const { return m_access.get_byte_mask(); }

   address_type get_pc() const { return m_inst.empty()?-1:m_inst.pc; }
   const warp_inst_t &get_inst() { return m_inst; }
   enum mem_fetch_status get_status() const { return m_status; }

   const memory_config *get_mem_config(){return m_mem_config;}

   unsigned get_num_flits(bool simt_to_mem);
private:
   // request source information
   unsigned m_request_uid;
   unsigned m_sid;
   unsigned m_tpc;
   unsigned m_wid;

   // where is this request now?
   enum mem_fetch_status m_status;
   unsigned long long m_status_change;

   // request type, address, size, mask
   mem_access_t m_access;
   unsigned m_data_size; // how much data is being written
   unsigned m_ctrl_size; // how big would all this meta data be in hardware (does not necessarily match actual size of mem_fetch)
   new_addr_type m_partition_addr; // linear physical address *within* dram partition (partition bank select bits squeezed out)
   addrdec_t m_raw_addr; // raw physical address (i.e., decoded DRAM chip-row-bank-column address)
   enum mf_type m_type;

   // statistics
   unsigned m_timestamp;  // set to gpu_sim_cycle+gpu_tot_sim_cycle at struct creation
   unsigned m_timestamp2; // set to gpu_sim_cycle+gpu_tot_sim_cycle when pushed onto icnt to shader; only used for reads
   unsigned m_icnt_receive_time; // set to gpu_sim_cycle + interconnect_latency when fixed icnt latency mode is enabled

   // requesting instruction (put last so mem_fetch prints nicer in gdb)
   warp_inst_t m_inst;

   static unsigned sm_next_mf_request_uid;

   const class memory_config *m_mem_config;
   unsigned icnt_flit_size;

public:
   std::vector<unsigned long long> m_time_stamp;
   unsigned m_MC_id;
   std::vector<unsigned long long> m_data_size_hist;

   new_addr_type m_blk_addr;

//added by kh(070715)
   public:
   std::vector<unsigned long long> m_status_timestamp;
   ///

  //added by kh(070715)
public:
  void set_timestamp(enum mem_fetch_status status, unsigned long long cycle);
  unsigned long long get_timestamp(enum mem_fetch_status status);
  ///


  //added by kh(031716)
  //Common member variables for all compressors
private:
  std::vector<unsigned char> m_real_data;
  unsigned m_comp_data_size;	//best compression result
  unsigned m_comp_res;
  int m_final_packet_size;	//packet size after final compression

public:
  unsigned char* config_real_data(unsigned data_size);
  unsigned char get_real_data(unsigned data_index);
  void set_real_data(unsigned data_index, unsigned char data);
  void get_real_data_stream(unsigned flit_seq, std::vector<unsigned char>& dest, int stream_size=32);
  unsigned get_real_data_size();
  std::vector<unsigned char>& get_real_data_ptr();
  void clear_real_data();





  void set_comp_data_size(unsigned comp_data_size);
  unsigned get_comp_data_size();
  void set_comp_res(unsigned comp_res);
  unsigned get_comp_res();
  void set_final_packet_size(int final_packet_size);
  int get_final_packet_size();
  ///

  //added by kh(051816)
  //Optimization for LWM algorithm
private:
  int m_comp_data_size_2B;
  int m_comp_data_size_4B;
  int m_comp_data_size_8B;
  int m_comp_data_bits_2B;
  int m_comp_data_bits_4B;
  int m_comp_data_bits_8B;

  void* m_loc_dict_2B;
  void* m_loc_dict_4B;
  void* m_loc_dict_8B;
  int m_comp_data_bits;		//best compression result

public:
  void set_comp_data_size(unsigned comp_data_size, unsigned res);
  unsigned get_comp_data_size(unsigned res);
  void* get_loc_dict(unsigned size);
  void set_loc_dict(unsigned size, void* loc_dict);
  void set_comp_data_bits(unsigned comp_data_bits, int res=-1);
  unsigned get_comp_data_bits(int res=-1);

//  //added by kh(051916)
//private:
//  unsigned m_inter_comp_data_size;
//  unsigned m_inter_comp_matching_word_no;
//
//public:
//  void set_inter_comp_data_bits(unsigned comp_data_size);
//  unsigned get_inter_comp_data_bits();
//  void set_inter_comp_matching_word_no(unsigned matching_word_no);
//  unsigned get_inter_comp_matching_word_no();
/////

  //added by kh(042316)
  //inter-replies merging to reduce the fragmentation
private:
  std::vector<mem_fetch*> m_merged_mfs;
  //unsigned m_merged_mf_comp_size_bits;				//out-of-dated
public:
  void add_merged_mfs(std::vector<mem_fetch*>& mfs);
  void del_merged_mfs();
  std::vector<mem_fetch*>& get_merged_mfs();
  bool has_merged_mem_fetch();
  unsigned get_merged_mf_no();
  mem_fetch* get_merged_mf(unsigned index);
  void add_merged_mf(mem_fetch* mf);
  //unsigned get_comp_size_merged_mem_fetch();
  //void set_merged_mf_comp_size_bits(unsigned merged_mf_comp_size_bits);
  ///

  //added by kh(062516)
private:
  std::vector<mem_fetch*> m_merged_mfs_to_same_SM;
public:
  void add_merged_mf_to_same_SM(mem_fetch* mf);
  unsigned get_merged_mf_to_same_SM_no();
  mem_fetch* get_merged_mf_to_same_SM(unsigned index);

  void set_merged_mfs_to_same_SM(std::vector<mem_fetch*>& merged_mfs_to_same_SM);
  std::vector<mem_fetch*>& get_merged_mfs_to_same_SM();
  ///

  //added by kh(062316)
private:
  unsigned m_rec_comp_merged_pkt_bits;	//pkt bits of merged mfs
public:
  void set_rec_comp_merged_pkt_bits(unsigned rec_comp_pkt_bits);
  void add_rec_comp_merged_pkt_bits(unsigned rec_comp_pkt_bits);
  unsigned get_rec_comp_merged_pkt_bits();
  ///

  //added by kh(062316)
private:
  unsigned m_rec_comp_data_bits;	//pkt bits of my mf
public:
  void set_rec_comp_data_bits(unsigned rec_comp_data_bits);
  void add_rec_comp_data_bits(unsigned rec_comp_data_bits);
  unsigned get_rec_comp_data_bits();
  ///

  //added by kh(060616)
  //Data Remapping
public:
  enum COMP_DATA_TYPE {
    NO_DATA_TYPE = -1,
    ORG_DATA,
    REMAPPED_DATA_1,	//4B
    REMAPPED_DATA_2,	//4B
    REMAPPED_DATA_3,	//8B
    REMAPPED_DATA_4,	//8B
    REMAPPED_DATA_5,	//2B
    REMAPPED_DATA_6,	//2B

	//added by kh(021817)
	CHAR_DATA,	//1B
	///

  };

  enum REMAP_OP_TYPE {
    MASK_OP = 0,
    COMP_OP,
    NEIGHBOR_CONST_DELTA_OP,
    DELTA_UPTO_1_OP,
    DELTA_UPTO_3_OP,
    COMP_DELTA_UPTO_1_OP,
    COMP_DELTA_UPTO_3_OP,
  };

private:
  std::vector<std::vector<unsigned char> > m_trans_data;
  std::vector<enum COMP_DATA_TYPE> m_trans_data_type;
  std::vector<unsigned> m_remap_op_bits;
  std::vector<std::vector<enum REMAP_OP_TYPE> > m_remap_op_types;
  //added by kh(080416)
  //std::vector<unsigned> m_remap_op_locs;

  std::vector<void*> m_trans_loc_dict_2B;
  std::vector<void*> m_trans_loc_dict_4B;
  std::vector<void*> m_trans_loc_dict_8B;
  std::vector<unsigned> m_comp_trans_data_bits_2B;
  std::vector<unsigned> m_comp_trans_data_bits_4B;
  std::vector<unsigned> m_comp_trans_data_bits_8B;

  //added by kh(072816)
  std::vector<unsigned> m_approx_op;
  std::vector<unsigned> m_approx_exception;


public:
  unsigned char* config_trans_data(unsigned data_size, unsigned index=0);
  unsigned char get_trans_data(unsigned data_index, unsigned index=0);
  void set_trans_data(unsigned data_index, unsigned char data, unsigned index=0);
  unsigned get_trans_data_size(unsigned index=0);
  void* get_trans_loc_dict(unsigned res, unsigned index=0);
  void set_trans_loc_dict(void* trans_loc_dict, unsigned res, unsigned index=0);
  std::vector<unsigned char>& get_trans_data_ptr(unsigned index=0);
  void commit_trans_data_info(unsigned res=14, unsigned index=0);
  void set_trans_comp_data_bits(unsigned comp_bits_size, unsigned res, unsigned index=0);
  unsigned get_trans_comp_data_bits(unsigned res, unsigned index=0);
  unsigned get_trans_loc_dict_no();
  unsigned get_trans_data_no();

  //added by kh(071316)
  //void save_remapped_data(unsigned index, std::vector<unsigned char>& remapped_data, enum COMP_DATA_TYPE data_type);
  std::vector<unsigned char>* config_remapped_data(unsigned index, unsigned data_size);
  void set_remapped_data_type(unsigned index, enum COMP_DATA_TYPE data_type);
  enum COMP_DATA_TYPE get_remapped_data_type(unsigned index);
  int get_remapped_data_type_index(enum COMP_DATA_TYPE data_type);
  ///

  //added by kh(072016)
  void set_remap_op_bits(unsigned index, unsigned remap_op_bits);
  unsigned get_remap_op_bits(unsigned index);
  ///

public:

  //added by kh(072016)
  void set_remap_op_type(unsigned index, std::vector<enum REMAP_OP_TYPE>& remap_op_type);
  std::vector<enum REMAP_OP_TYPE>& get_remap_op_type(unsigned index);
  ///

public:
  //added by kh(072816)
  void set_approx_op(unsigned index, unsigned approx_op);
  unsigned get_approx_op(unsigned index);
  void set_approx_exception(unsigned index, unsigned approx_exception);
  unsigned get_approx_exception(unsigned index);
  ///

//added by kh(070416)
private:
  int m_dsc_comp_res;
public:
  void set_dsc_comp_res(int dsc_comp_res);
  int get_dsc_comp_res();
private:
  enum COMP_DATA_TYPE m_dsc_comp_data_type;
public:
  void set_dsc_comp_data_type(enum COMP_DATA_TYPE type);
  enum COMP_DATA_TYPE get_dsc_comp_data_type();
  void print_dsc_comp_data_type(enum COMP_DATA_TYPE type);
///

//added by kh(070416)
//DSC
private:
  std::vector<int> m_comp_ds_status;	//final status based on 16B resolution uniformly regardless of
  std::vector<std::vector<int> > m_comp_ds_status_list;
public:
  void set_comp_ds_status_size(unsigned size, int init_val=-1, enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  void set_comp_ds_status(unsigned index, unsigned comp_status, enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  unsigned get_comp_ds_status_size(enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  unsigned get_comp_ds_status(unsigned index, enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  bool has_uncompressed_ds(enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  std::vector<int>& get_comp_ds_status(enum COMP_DATA_TYPE type=NO_DATA_TYPE);


private:
  unsigned m_dsc_compressed_bits_only;	//header + compressed ds.
  std::vector<unsigned> m_dsc_compressed_bits_only_list;
public:
  void set_dsc_compressed_bits_only(unsigned compressed_bits_only, enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  unsigned get_dsc_compressed_bits_only(enum COMP_DATA_TYPE type=NO_DATA_TYPE);

//LWM
private:
  std::vector<double> m_comp_ds_bits;
public:
  void set_comp_ds_bits_size(unsigned size, double init_val=-1);
  void set_comp_ds_bits(unsigned index, unsigned comp_ds_bits);	//based on 16B resolution
  unsigned get_comp_ds_bits_size();
  unsigned get_comp_ds_bits(unsigned index);
///

//added by kh(090116)
/*
private:
  unsigned m_lwm_compressed_bits_only;
public:
  void set_lwm_compressed_bits_only(unsigned compressed_bits_only, enum COMP_DATA_TYPE type=NO_DATA_TYPE);
  unsigned get_lwm_compressed_bits_only(enum COMP_DATA_TYPE type=NO_DATA_TYPE);
*/
//


private:
  std::vector<std::vector<double> > m_lwm_comp_ds_bits;			//0-2: 2B, 4B, 8B
  std::vector<std::vector<double> > m_lwm_comp_ds_bits_remap_type1;	//0-2: 2B, 4B, 8B
  std::vector<std::vector<double> > m_lwm_comp_ds_bits_remap_type2;	//0-2: 2B, 4B, 8B
public:
  void set_comp_ds_bits(std::vector<double>& comp_ds_bits, unsigned comp_res, enum COMP_DATA_TYPE type);
  std::vector<double>& get_comp_ds_bits(unsigned comp_res, enum COMP_DATA_TYPE type);


//added by kh(070616)
public:
  //void print_data(enum COMP_DATA_TYPE data_type);
  void print_data_all(unsigned space_gap);
  void print_data(std::vector<unsigned char>& data, unsigned space_gap);
  void print_real_data(unsigned space_gap);

//added by kh(071216)
public:
  enum COMP_ALGO_TYPE {
    NO_COMP = -1,
    DSM_COMP,
    LWM_COMP,
  };
  void set_comp_algo_type(enum COMP_ALGO_TYPE algo_type);
  enum COMP_ALGO_TYPE get_comp_algo_type();
private:
  enum COMP_ALGO_TYPE m_algo_type;

private:
  //m_redund_nibble_rate[data_type][resolution_index]
  //data_type : 0 1 2
  //resolution_index : 0(2B), 1(4B), 2(8B)
  //std::vector<std::vector<double> > m_lwm_redund_nibble_rate;
  double m_redund_nibble_rate;
public:
  /*
  void set_redund_nibble_rate(enum COMP_DATA_TYPE data_type, unsigned res, double redund_nibble_rate);
  void get_redund_nibble_rate(enum COMP_DATA_TYPE data_type, unsigned res);
  */
  void set_redund_nibble_rate(double redund_nibble_rate);
  double get_redund_nibble_rate();

/*
//added by kh(082616)
private:
  unsigned m_cache_request_status;
  warp_inst_t* m_org_inst;
public:
  unsigned get_cache_request_status();
  void set_cache_request_status(unsigned cache_request_status);
  warp_inst_t* get_org_inst();
  void set_org_inst(warp_inst_t* org_inst);
///
*/

//added by kh(090116)
private:
  unsigned m_comp_es_byte;	//encoding status byte
  unsigned m_comp_ed_byte;	//encoding data byte
public:
  void set_comp_es_byte(unsigned comp_es_byte);
  void set_comp_ed_byte(unsigned comp_ed_byte);
  unsigned get_comp_es_byte();
  unsigned get_comp_ed_byte();
///

//added by kh(090116)
private:
  unsigned m_dsc_comp_es_byte;	//encoding status byte
  unsigned m_dsc_comp_ed_byte;	//encoding data byte
  std::vector<unsigned> m_lwm_comp_es_byte;	//encoding status byte
  std::vector<unsigned> m_lwm_comp_ed_byte;	//encoding data byte
public:
  void set_dsc_comp_es_byte(unsigned comp_es_byte);
  void set_dsc_comp_ed_byte(unsigned comp_ed_byte);
  unsigned get_dsc_comp_es_byte();
  unsigned get_dsc_comp_ed_byte();
  void set_lwm_comp_es_byte(unsigned comp_es_byte, unsigned res);
  void set_lwm_comp_ed_byte(unsigned comp_ed_byte, unsigned res);
  unsigned get_lwm_comp_es_byte(unsigned res);
  unsigned get_lwm_comp_ed_byte(unsigned res);
///


//added by kh(092616)
//when a memory access hits a cache block, the compression information for the block
//is stored temporarily.
private:
  unsigned m_is_hit_cache_compressed;

public:
  unsigned get_hit_cache_compressed();
  void set_hit_cache_compressed(unsigned is_hit_cache_compressed);


//added by abpd(042816)
public:
  std::string compare_data;
private:
  std::vector<unsigned char> m_compress_data;
public:
  void set_comp_data(unsigned data_index, unsigned char data)
  {
    m_compress_data[data_index] = data;
  }
  void init_size()
  {
    m_compress_data.resize(m_comp_data_size);
  }
///


//added by kh(021817)
private:
  std::vector<unsigned char> m_txt_trans_data;
  bool m_is_chartype;
public:
  unsigned char get_txt_trans_data(unsigned data_index);
  void set_txt_trans_data(unsigned data_index, unsigned char data);
  unsigned get_txt_trans_data_size();
  std::vector<unsigned char>& get_txt_trans_data_ptr();


  void set_chartype();
  bool get_chartype();
///




#ifdef COMP_WORD_ANALYSIS
  //added by kh(042216)
public:
  enum comp_status_type {
    UNCOMPRESSED = 0,
    COMPRESSED,
    INTRA_COMPRESSED,
    INTER_COMPRESSED,
  };

  class comp_word {
    public:
      comp_word()	{};
      ~comp_word()	{};
    public:
      unsigned char word1B;
      unsigned short word2B;
      unsigned int word4B;
      unsigned long long word8B;
      unsigned res;
      enum comp_status_type comp_status;

      void print(bool linefeed=true)
      {
	if(linefeed == true)	printf("\t");

	if(res == 1)		printf("word %02x, comp %d", word1B, comp_status);
	else if(res == 2)	printf("word %04x, comp %d", word2B, comp_status);
	else if(res == 4)	printf("word %08x, comp %d", word4B, comp_status);
	else if(res == 8)	printf("word %016llx, comp %d", word8B, comp_status);
	else assert(0);

	if(linefeed == true)	printf("\n");
      }
  };

  std::vector<class comp_word> m_compword2B_list;
  std::vector<class comp_word> m_compword4B_list;
  std::vector<class comp_word> m_compword8B_list;

  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res2;	//row: word, col: one byte in the word.
  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res4;	//row: word, col: one byte in the word.
  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res8;	//row: word, col: one byte in the word.

  void push_compword(unsigned res, unsigned long long comp_word, enum comp_status_type comp_status)
  {
    class comp_word tmp;
    if(res == 2) {
	tmp.res = 2;
	tmp.word2B = (unsigned short) comp_word;
	tmp.comp_status = comp_status;
	m_compword2B_list.push_back(tmp);
    } else if(res == 4) {
	tmp.res = 4;
	tmp.word4B = (unsigned int) comp_word;
	tmp.comp_status = comp_status;
	m_compword4B_list.push_back(tmp);
    } else if(res == 8) {
	tmp.res = 8;
	tmp.word8B = (unsigned long long) comp_word;
	tmp.comp_status = comp_status;
	m_compword8B_list.push_back(tmp);
    }
  }

  void push_uncompword(unsigned res, unsigned row, unsigned char uncomp_word, enum comp_status_type comp_status)
  {
    //printf("push_uncompword starts rest %u\n", res);
    class comp_word tmp;
    if(res == 2) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res2.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res2.push_back(tmp);
      }
      m_uncompword1B_list_res2[row].push_back(tmp);
    } else if(res == 4) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res4.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res4.push_back(tmp);
      }
      m_uncompword1B_list_res4[row].push_back(tmp);
    } else if(res == 8) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res8.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res8.push_back(tmp);
      }
      m_uncompword1B_list_res8[row].push_back(tmp);
    }
    //printf("push_uncompword ends\n");
  }

  void print_compword()
  {
    if(m_comp_res == 2) {
      for(unsigned i = 0; i < m_compword2B_list.size(); i++)	m_compword2B_list[i].print();
    } else if(m_comp_res == 4) {
      for(unsigned i = 0; i < m_compword4B_list.size(); i++)	m_compword4B_list[i].print();
    } else if(m_comp_res == 8) {
      for(unsigned i = 0; i < m_compword8B_list.size(); i++)	m_compword8B_list[i].print();
    }
  }

  void print_uncompword()
  {
    printf("print_uncompword starts, comp_res %u\n", m_comp_res);
    if(m_comp_res == 2) {
      for(unsigned i = 0; i < m_uncompword1B_list_res2.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res2[i].size(); j++) {
	    m_uncompword1B_list_res2[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    } else if(m_comp_res == 4) {
      for(unsigned i = 0; i < m_uncompword1B_list_res4.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res4[i].size(); j++) {
	    m_uncompword1B_list_res4[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    } else if(m_comp_res == 8) {
      for(unsigned i = 0; i < m_uncompword1B_list_res8.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res8[i].size(); j++) {
	    m_uncompword1B_list_res8[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    }
    printf("print_uncompword ends\n");
  }
  ///

  //added by kh(042316)
  unsigned get_uncompword_row_no() {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2.size();
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4.size();
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8.size();

    assert(0);
    return 0;
  }

  unsigned get_uncompword_col_no(unsigned index) {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2[index].size();
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4[index].size();
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8[index].size();

    assert(0);
    return 0;
  }

  class comp_word get_uncompword(unsigned row, unsigned col) {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2[row][col];
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4[row][col];
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8[row][col];

    assert(0);
    class comp_word dummy;
    return dummy;
  }
  ///
#endif

#ifdef REQ_COAL_MODULE
  //added by kh(042516)
private:
  std::vector<mem_fetch*> m_merged_req_mfs;
public:
  void add_merged_req_mfs(std::vector<mem_fetch*>& mfs);
  void del_merged_req_mfs();
  std::vector<mem_fetch*>& get_merged_req_mfs();
  bool has_merged_req_mfs();
  unsigned get_size_merged_req_mfs();
#endif


};

#endif
