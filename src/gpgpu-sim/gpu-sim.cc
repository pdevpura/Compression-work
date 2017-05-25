// Copyright (c) 2009-2011, Tor M. Aamodt, Wilson W.L. Fung, George L. Yuan,
// Ali Bakhoda, Andrew Turner, Ivan Sham
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


#include "gpu-sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "zlib.h"
#include <memory>

#include "shader.h"
#include "dram.h"
#include "mem_fetch.h"

#include <time.h>
#include "gpu-cache.h"
#include "gpu-misc.h"
#include "delayqueue.h"
#include "shader.h"
#include "icnt_wrapper.h"
#include "dram.h"
#include "addrdec.h"
#include "stat-tool.h"
#include "l2cache.h"

#include "../cuda-sim/ptx-stats.h"
#include "../statwrapper.h"
#include "../abstract_hardware_model.h"
#include "../debug.h"
#include "../gpgpusim_entrypoint.h"
#include "../cuda-sim/cuda-sim.h"
#include "../trace.h"
#include "mem_latency_stat.h"
#include "power_stat.h"
#include "visualizer.h"
#include "stats.h"

#ifdef GPGPUSIM_POWER_MODEL
#include "power_interface.h"
#else
class  gpgpu_sim_wrapper {};
#endif

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>

#define MAX(a,b) (((a)>(b))?(a):(b))


bool g_interactive_debugger_enabled=false;

unsigned long long  gpu_sim_cycle = 0;
unsigned long long  gpu_tot_sim_cycle = 0;


// performance counter for stalls due to congestion.
unsigned int gpu_stall_dramfull = 0;
unsigned int gpu_stall_icnt2sh = 0;

/* Clock Domains */

#define  CORE  0x01
#define  L2    0x02
#define  DRAM  0x04
#define  ICNT  0x08


#define MEM_LATENCY_STAT_IMPL


//added by kh(053015)
l2_cache_config g_L2_config;
///


//added by kh(061215)
#include "user-defined.h"
extern std::vector<mem_fetch_debug*> g_dbg_msg_vector;
extern std::map<unsigned,mem_fetch_debug*> g_dbg_msg_map;

//check SMs used in each kernel
std::set<unsigned> g_SMs_in_use;
std::set<unsigned> g_MCs_in_use;
///

//added by kh(121715)
struct mc_placement_config g_mc_placement_config;


//added by kh(122815)
#include "hpcl_network_anal.h"
hpcl_network_anal* g_hpcl_network_anal = NULL;
///

#include "mem_latency_stat.h"

//added by kh(030816)
//compression
#include "hpcl_comp.h"
#include "hpcl_comp_anal.h"
#include "hpcl_comp_config.h"

hpcl_comp* g_hpcl_comp = NULL;
hpcl_comp_config g_hpcl_comp_config;
hpcl_comp_anal* g_hpcl_comp_anal = NULL;;
///


#ifdef GLOB_COMP_MODULE
#include "hpcl_comp_pl.h"
#include "hpcl_decomp_pl.h"
std::vector<hpcl_comp_pl<unsigned short>* > g_hpcl_global_comp_pl_2B;
std::vector<hpcl_decomp_pl<unsigned short>* > g_hpcl_global_decomp_pl_2B;
#endif


//added by kh(031716)
#include "hpcl_comp_lwm_pl.h"
//# of compressor = mem_no * sub_partition_no_per_memory_channel
std::vector<hpcl_comp_lwm_pl<unsigned short>* > g_hpcl_comp_lwm_pl_2B;
std::vector<hpcl_comp_lwm_pl<unsigned int>* > g_hpcl_comp_lwm_pl_4B;
std::vector<hpcl_comp_lwm_pl<unsigned long long>* > g_hpcl_comp_lwm_pl_8B;
///

//added by kh(073016)
bool g_hpcl_comp_lwm_2B_en = false;
bool g_hpcl_comp_lwm_4B_en = false;
bool g_hpcl_comp_lwm_8B_en = false;
///

//added by kh(060216)
//std::vector<hpcl_comp_lwm_pl<unsigned char>* > g_hpcl_comp_lwm_pl_1B;

//added bu kh(041816)
#include "hpcl_comp_buffer.h"
std::vector<hpcl_comp_buffer*> g_hpcl_comp_buffer;
///

//added by kh(041216)
#include <cmath>	//for computing compression data size.
///

//added by kh(042516)
#include "hpcl_req_coal_buffer.h"
std::vector<hpcl_req_coal_buffer*> g_hpcl_req_coal_buffer;	//for coalescing requests.
#include "hpcl_coal_anal.h"
hpcl_coal_anal* g_hpcl_coal_anal = NULL;
///


// added by abpd (042816)
#include "hpcl_comp_cpack_pl.h"
#include "hpcl_comp_fpc_pl.h"
#include "hpcl_comp_bdi_pl.h"
#include "hpcl_comp_abpd_local_pl.h"
#include "hpcl_comp_analysis.h"

std::vector<hpcl_comp_bdi_pl<unsigned int>* > g_hpcl_comp_bdi_pl_4B;
std::vector<hpcl_comp_fpc_pl<unsigned int>* > g_hpcl_comp_fpc_pl_4B;
std::vector<hpcl_comp_abpd_local_pl<unsigned int>* > g_hpcl_comp_abpd_local_pl_4B;
std::vector<hpcl_comp_cpack_pl<unsigned int>* > g_hpcl_comp_cpack_pl_4B;
abpd_comp_anal *abpd_anal_ptr = NULL;
///


//added by kh(051816)
#include "hpcl_comp_lwm2_pl.h"
//# of compressor = mem_no * sub_partition_no_per_memory_channel
std::vector<hpcl_comp_lwm2_pl<unsigned short>* > g_hpcl_comp_lwm2_pl_2B;
std::vector<hpcl_comp_lwm2_pl<unsigned int>* > g_hpcl_comp_lwm2_pl_4B;
std::vector<hpcl_comp_lwm2_pl<unsigned long long>* > g_hpcl_comp_lwm2_pl_8B;
///

//added by kh(060216)
unsigned g_NI_input_buffer_size = 0;
///

#define RECURSIVE_COMP_DEBUG 1


//added by kh(060916)
#include "hpcl_comp_lwm_aux.h"
hpcl_comp_lwm_aux* g_hpcl_comp_lwm_aux;

//added by kh(062316)
#ifdef REC_COMP_MODULE
#include "hpcl_rec_comp_lwm_pl.h"
std::vector<hpcl_rec_comp_lwm_pl*> g_hpcl_rec_comp_lwm_pl;
#endif
///

//added bu kh(062416)
std::vector<hpcl_rec_comp_buffer*> g_hpcl_rec_comp_buffer;
bool g_multicast_en = false;
///

//added by kh(070316)
#include "hpcl_comp_dsc_pl.h"
std::vector<hpcl_comp_dsc_pl* > g_hpcl_comp_dsc_pl;


// added by abpd (061916)
//#include "hpcl_comp_sc2_pl.h"
#include "hpcl_comp_fph_pl.h"
std::vector<shared_ptr<hpcl_comp_fph_pl<unsigned int> > >g_hpcl_comp_fph_pl_4B;
//std::vector<hpcl_comp_fph_pl<unsigned int>* > g_hpcl_comp_fph_pl_4B;
//std::vector<hpcl_comp_sc2_pl<unsigned int>* > g_hpcl_comp_sc2_pl_4B;

extern void init_tracker_fph(string file_name,unordered_map<string,string>& src_tracker);

//added by kh(070516)
extern void exit_simulation();

// added by abpd (071616)
#include "hpcl_comp_bpc_pl.h"
std::vector<hpcl_comp_bpc_pl<unsigned int>* > g_hpcl_comp_bpc_pl_4B;
///

// added by abpd (071616)
#include "hpcl_comp_sc2_pl.h"

std::vector<shared_ptr<hpcl_comp_sc2_pl<unsigned int> > > g_hpcl_comp_sc2_pl_4B;


//added by kh(071816)
#include "hpcl_decomp_lwm_pl.h"
std::vector<hpcl_decomp_lwm_pl<unsigned>* > g_hpcl_decomp_lwm_pl;
//std::vector<hpcl_decomp_lwm_pl<unsigned short>* > g_hpcl_decomp_lwm_pl_2B;
//std::vector<hpcl_decomp_lwm_pl<unsigned int>* > g_hpcl_decomp_lwm_pl_4B;
//std::vector<hpcl_decomp_lwm_pl<unsigned long long>* > g_hpcl_decomp_lwm_pl_8B;
std::vector<hpcl_comp_buffer*> g_hpcl_decomp_buffer;
///

#include "hpcl_decomp_dsc_pl.h"
#include "hpcl_decomp_fph_pl.h"
#include "hpcl_decomp_sc2_pl.h"
#include "hpcl_decomp_cpack_pl.h"
#include "hpcl_decomp_fpc_pl.h"
#include "hpcl_decomp_bdi_pl.h"
#include "hpcl_decomp_bpc_pl.h"

std::vector<hpcl_decomp_dsc_pl<unsigned>* > g_hpcl_decomp_dsc_pl;
std::vector<hpcl_decomp_fph_pl<unsigned>* > g_hpcl_decomp_fph_pl;
std::vector<hpcl_decomp_sc2_pl<unsigned>* > g_hpcl_decomp_sc2_pl;
std::vector<hpcl_decomp_cpack_pl<unsigned>* > g_hpcl_decomp_cpack_pl;
std::vector<hpcl_decomp_fpc_pl<unsigned>* > g_hpcl_decomp_fpc_pl;
std::vector<hpcl_decomp_bdi_pl<unsigned>* > g_hpcl_decomp_bdi_pl;
std::vector<hpcl_decomp_bpc_pl<unsigned>* > g_hpcl_decomp_bpc_pl;
std::vector<hpcl_comp_buffer*> g_hpcl_decomp_cache_buffer;
///


//added by hk(102016)
std::vector<hpcl_decomp_dsc_pl<unsigned>* > g_hpcl_decomp_dsc_pl_l1D;
std::vector<hpcl_decomp_fph_pl<unsigned>* > g_hpcl_decomp_fph_pl_l1D;
std::vector<hpcl_decomp_sc2_pl<unsigned>* > g_hpcl_decomp_sc2_pl_l1D;
std::vector<hpcl_decomp_cpack_pl<unsigned>* > g_hpcl_decomp_cpack_pl_l1D;
std::vector<hpcl_decomp_fpc_pl<unsigned>* > g_hpcl_decomp_fpc_pl_l1D;
std::vector<hpcl_decomp_bdi_pl<unsigned>* > g_hpcl_decomp_bdi_pl_l1D;
std::vector<hpcl_decomp_bpc_pl<unsigned>* > g_hpcl_decomp_bpc_pl_l1D;
//std::vector<hpcl_comp_buffer*> g_hpcl_decomp_cache_buffer_l1D;
///


//added by kh(080216)
unsigned int fpc_comp_lat = 3;
unsigned int fpc_decomp_lat = 5;
unsigned int bdi_comp_lat = 2;
unsigned int bdi_decomp_lat = 1;
unsigned int cpack_comp_lat = 16;
unsigned int cpack_decomp_lat = 9;
unsigned int bpc_comp_lat = 7;
unsigned int bpc_decomp_lat = 11;
unsigned int sc2_comp_lat = 6;
unsigned int sc2_decomp_lat = 14;
unsigned int fph_comp_lat = 7;
unsigned int fph_decomp_lat = 20;
///


//added by kh(081516)
#include "hpcl_approx_mem_space.h"
hpcl_approx_mem_space* g_hpcl_approx_mem_space = NULL;
///

//added by kh(101216)
struct cache_sub_stats g_prev_total_L1D_css;
//


//added by kh(101816)
#include "hpcl_dyn_comp.h"
///


//added by kh(021617)
//Analyze data redundancy
#include "hpcl_data_reader.h"
#include "hpcl_data_anal.h"
hpcl_data_reader* g_hpcl_data_reader = NULL;
hpcl_data_anal* g_hpcl_data_anal = NULL;
///


//added by kh(030117)
//#include "hpcl_comp_dsc2_pl.h"
//#include "hpcl_decomp_dsc2_pl.h"
//std::vector<hpcl_comp_dsc2_pl* > g_hpcl_comp_dsc2_pl;
//std::vector<hpcl_decomp_dsc2_pl<unsigned>* > g_hpcl_decomp_dsc2_pl;
////


void power_config::reg_options(class OptionParser * opp)
{
  option_parser_register(opp, "-gpuwattch_xml_file", OPT_CSTR,
						 &g_power_config_name,"GPUWattch XML file",
		   "gpuwattch.xml");

   option_parser_register(opp, "-power_simulation_enabled", OPT_BOOL,
			  &g_power_simulation_enabled, "Turn on power simulator (1=On, 0=Off)",
			  "0");

   option_parser_register(opp, "-power_per_cycle_dump", OPT_BOOL,
			  &g_power_per_cycle_dump, "Dump detailed power output each cycle",
			  "0");

   // Output Data Formats
   option_parser_register(opp, "-power_trace_enabled", OPT_BOOL,
			  &g_power_trace_enabled, "produce a file for the power trace (1=On, 0=Off)",
			  "0");

   option_parser_register(opp, "-power_trace_zlevel", OPT_INT32,
			  &g_power_trace_zlevel, "Compression level of the power trace output log (0=no comp, 9=highest)",
			  "6");

   option_parser_register(opp, "-steady_power_levels_enabled", OPT_BOOL,
			  &g_steady_power_levels_enabled, "produce a file for the steady power levels (1=On, 0=Off)",
			  "0");

   option_parser_register(opp, "-steady_state_definition", OPT_CSTR,
			  &gpu_steady_state_definition, "allowed deviation:number of samples",
			  "8:4");

}

void memory_config::reg_options(class OptionParser * opp)
{
    option_parser_register(opp, "-gpgpu_dram_scheduler", OPT_INT32, &scheduler_type,
                                "0 = fifo, 1 = FR-FCFS (defaul)", "1");
    option_parser_register(opp, "-gpgpu_dram_partition_queues", OPT_CSTR, &gpgpu_L2_queue_config,
                           "i2$:$2d:d2$:$2i",
                           "8:8:8:8");

    option_parser_register(opp, "-l2_ideal", OPT_BOOL, &l2_ideal,
                           "Use a ideal L2 cache that always hit",
                           "0");
    option_parser_register(opp, "-gpgpu_cache:dl2", OPT_CSTR, &m_L2_config.m_config_string,
                   "unified banked L2 data cache config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq>}",
                   "64:128:8,L:B:m:N,A:16:4,4");
    option_parser_register(opp, "-gpgpu_cache:dl2_texture_only", OPT_BOOL, &m_L2_texure_only,
                           "L2 cache used for texture only",
                           "1");
    option_parser_register(opp, "-gpgpu_n_mem", OPT_UINT32, &m_n_mem,
                 "number of memory modules (e.g. memory controllers) in gpu",
                 "8");
    option_parser_register(opp, "-gpgpu_n_sub_partition_per_mchannel", OPT_UINT32, &m_n_sub_partition_per_memory_channel,
                 "number of memory subpartition in each memory module",
                 "1");
    option_parser_register(opp, "-gpgpu_n_mem_per_ctrlr", OPT_UINT32, &gpu_n_mem_per_ctrlr,
                 "number of memory chips per memory controller",
                 "1");
    option_parser_register(opp, "-gpgpu_memlatency_stat", OPT_INT32, &gpgpu_memlatency_stat,
                "track and display latency statistics 0x2 enables MC, 0x4 enables queue logs",
                "0");
    option_parser_register(opp, "-gpgpu_frfcfs_dram_sched_queue_size", OPT_INT32, &gpgpu_frfcfs_dram_sched_queue_size,
                "0 = unlimited (default); # entries per chip",
                "0");
    option_parser_register(opp, "-gpgpu_dram_return_queue_size", OPT_INT32, &gpgpu_dram_return_queue_size,
                "0 = unlimited (default); # entries per chip",
                "0");
    option_parser_register(opp, "-gpgpu_dram_buswidth", OPT_UINT32, &busW,
                 "default = 4 bytes (8 bytes per cycle at DDR)",
                 "4");
    option_parser_register(opp, "-gpgpu_dram_burst_length", OPT_UINT32, &BL,
                 "Burst length of each DRAM request (default = 4 data bus cycle)",
                 "4");
    option_parser_register(opp, "-dram_data_command_freq_ratio", OPT_UINT32, &data_command_freq_ratio,
                 "Frequency ratio between DRAM data bus and command bus (default = 2 times, i.e. DDR)",
                 "2");
    option_parser_register(opp, "-gpgpu_dram_timing_opt", OPT_CSTR, &gpgpu_dram_timing_opt,
                "DRAM timing parameters = {nbk:tCCD:tRRD:tRCD:tRAS:tRP:tRC:CL:WL:tCDLR:tWR:nbkgrp:tCCDL:tRTPL}",
                "4:2:8:12:21:13:34:9:4:5:13:1:0:0");
    option_parser_register(opp, "-rop_latency", OPT_UINT32, &rop_latency,
                     "ROP queue latency (default 85)",
                     "85");
    option_parser_register(opp, "-dram_latency", OPT_UINT32, &dram_latency,
                     "DRAM latency (default 30)",
                     "30");

    m_address_mapping.addrdec_setoption(opp);
}

void shader_core_config::reg_options(class OptionParser * opp)
{
    option_parser_register(opp, "-gpgpu_simd_model", OPT_INT32, &model,
                   "1 = post-dominator", "1");
    option_parser_register(opp, "-gpgpu_shader_core_pipeline", OPT_CSTR, &gpgpu_shader_core_pipeline_opt,
                   "shader core pipeline config, i.e., {<nthread>:<warpsize>}",
                   "1024:32");
    option_parser_register(opp, "-gpgpu_tex_cache:l1", OPT_CSTR, &m_L1T_config.m_config_string,
                   "per-shader L1 texture cache  (READ-ONLY) config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq>:<rf>}",
                   "8:128:5,L:R:m:N,F:128:4,128:2");
    option_parser_register(opp, "-gpgpu_const_cache:l1", OPT_CSTR, &m_L1C_config.m_config_string,
                   "per-shader L1 constant memory cache  (READ-ONLY) config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq>} ",
                   "64:64:2,L:R:f:N,A:2:32,4" );
    option_parser_register(opp, "-gpgpu_cache:il1", OPT_CSTR, &m_L1I_config.m_config_string,
                   "shader L1 instruction cache config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq>} ",
                   "4:256:4,L:R:f:N,A:2:32,4" );
    option_parser_register(opp, "-gpgpu_cache:dl1", OPT_CSTR, &m_L1D_config.m_config_string,
                   "per-shader L1 data cache config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq> | none}",
                   "none" );
    option_parser_register(opp, "-gpgpu_cache:dl1PrefL1", OPT_CSTR, &m_L1D_config.m_config_stringPrefL1,
                   "per-shader L1 data cache config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq> | none}",
                   "none" );
    option_parser_register(opp, "-gpgpu_cache:dl1PreShared", OPT_CSTR, &m_L1D_config.m_config_stringPrefShared,
                   "per-shader L1 data cache config "
                   " {<nsets>:<bsize>:<assoc>,<rep>:<wr>:<alloc>:<wr_alloc>,<mshr>:<N>:<merge>,<mq> | none}",
                   "none" );
    option_parser_register(opp, "-gmem_skip_L1D", OPT_BOOL, &gmem_skip_L1D,
                   "global memory access skip L1D cache (implements -Xptxas -dlcm=cg, default=no skip)",
                   "0");

    option_parser_register(opp, "-gpgpu_perfect_mem", OPT_BOOL, &gpgpu_perfect_mem,
                 "enable perfect memory mode (no cache miss)",
                 "0");
    option_parser_register(opp, "-n_regfile_gating_group", OPT_UINT32, &n_regfile_gating_group,
                 "group of lanes that should be read/written together)",
                 "4");
    option_parser_register(opp, "-gpgpu_clock_gated_reg_file", OPT_BOOL, &gpgpu_clock_gated_reg_file,
                 "enable clock gated reg file for power calculations",
                 "0");
    option_parser_register(opp, "-gpgpu_clock_gated_lanes", OPT_BOOL, &gpgpu_clock_gated_lanes,
                 "enable clock gated lanes for power calculations",
                 "0");
    option_parser_register(opp, "-gpgpu_shader_registers", OPT_UINT32, &gpgpu_shader_registers,
                 "Number of registers per shader core. Limits number of concurrent CTAs. (default 8192)",
                 "8192");
    option_parser_register(opp, "-gpgpu_shader_cta", OPT_UINT32, &max_cta_per_core,
                 "Maximum number of concurrent CTAs in shader (default 8)",
                 "8");
    option_parser_register(opp, "-gpgpu_num_cta_barriers", OPT_UINT32, &max_barriers_per_cta,
                 "Maximum number of named barriers per CTA (default 16)",
                 "16");
    option_parser_register(opp, "-gpgpu_n_clusters", OPT_UINT32, &n_simt_clusters,
                 "number of processing clusters",
                 "10");
    option_parser_register(opp, "-gpgpu_n_cores_per_cluster", OPT_UINT32, &n_simt_cores_per_cluster,
                 "number of simd cores per cluster",
                 "3");
    option_parser_register(opp, "-gpgpu_n_cluster_ejection_buffer_size", OPT_UINT32, &n_simt_ejection_buffer_size,
                 "number of packets in ejection buffer",
                 "8");
    option_parser_register(opp, "-gpgpu_n_ldst_response_buffer_size", OPT_UINT32, &ldst_unit_response_queue_size,
                 "number of response packets in ld/st unit ejection buffer",
                 "2");
    option_parser_register(opp, "-gpgpu_shmem_size", OPT_UINT32, &gpgpu_shmem_size,
                 "Size of shared memory per shader core (default 16kB)",
                 "16384");
    option_parser_register(opp, "-gpgpu_shmem_size", OPT_UINT32, &gpgpu_shmem_sizeDefault,
                 "Size of shared memory per shader core (default 16kB)",
                 "16384");
    option_parser_register(opp, "-gpgpu_shmem_size_PrefL1", OPT_UINT32, &gpgpu_shmem_sizePrefL1,
                 "Size of shared memory per shader core (default 16kB)",
                 "16384");
    option_parser_register(opp, "-gpgpu_shmem_size_PrefShared", OPT_UINT32, &gpgpu_shmem_sizePrefShared,
                 "Size of shared memory per shader core (default 16kB)",
                 "16384");
    option_parser_register(opp, "-gpgpu_shmem_num_banks", OPT_UINT32, &num_shmem_bank,
                 "Number of banks in the shared memory in each shader core (default 16)",
                 "16");
    option_parser_register(opp, "-gpgpu_shmem_limited_broadcast", OPT_BOOL, &shmem_limited_broadcast,
                 "Limit shared memory to do one broadcast per cycle (default on)",
                 "1");
    option_parser_register(opp, "-gpgpu_shmem_warp_parts", OPT_INT32, &mem_warp_parts,
                 "Number of portions a warp is divided into for shared memory bank conflict check ",
                 "2");
    option_parser_register(opp, "-gpgpu_warpdistro_shader", OPT_INT32, &gpgpu_warpdistro_shader,
                "Specify which shader core to collect the warp size distribution from",
                "-1");
    option_parser_register(opp, "-gpgpu_warp_issue_shader", OPT_INT32, &gpgpu_warp_issue_shader,
                "Specify which shader core to collect the warp issue distribution from",
                "0");
    option_parser_register(opp, "-gpgpu_local_mem_map", OPT_BOOL, &gpgpu_local_mem_map,
                "Mapping from local memory space address to simulated GPU physical address space (default = enabled)",
                "1");
    option_parser_register(opp, "-gpgpu_num_reg_banks", OPT_INT32, &gpgpu_num_reg_banks,
                "Number of register banks (default = 8)",
                "8");
    option_parser_register(opp, "-gpgpu_reg_bank_use_warp_id", OPT_BOOL, &gpgpu_reg_bank_use_warp_id,
             "Use warp ID in mapping registers to banks (default = off)",
             "0");
    option_parser_register(opp, "-gpgpu_operand_collector_num_units_sp", OPT_INT32, &gpgpu_operand_collector_num_units_sp,
                "number of collector units (default = 4)",
                "4");
    option_parser_register(opp, "-gpgpu_operand_collector_num_units_sfu", OPT_INT32, &gpgpu_operand_collector_num_units_sfu,
                "number of collector units (default = 4)",
                "4");
    option_parser_register(opp, "-gpgpu_operand_collector_num_units_mem", OPT_INT32, &gpgpu_operand_collector_num_units_mem,
                "number of collector units (default = 2)",
                "2");
    option_parser_register(opp, "-gpgpu_operand_collector_num_units_gen", OPT_INT32, &gpgpu_operand_collector_num_units_gen,
                "number of collector units (default = 0)",
                "0");
    option_parser_register(opp, "-gpgpu_operand_collector_num_in_ports_sp", OPT_INT32, &gpgpu_operand_collector_num_in_ports_sp,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_in_ports_sfu", OPT_INT32, &gpgpu_operand_collector_num_in_ports_sfu,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_in_ports_mem", OPT_INT32, &gpgpu_operand_collector_num_in_ports_mem,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_in_ports_gen", OPT_INT32, &gpgpu_operand_collector_num_in_ports_gen,
                           "number of collector unit in ports (default = 0)",
                           "0");
    option_parser_register(opp, "-gpgpu_operand_collector_num_out_ports_sp", OPT_INT32, &gpgpu_operand_collector_num_out_ports_sp,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_out_ports_sfu", OPT_INT32, &gpgpu_operand_collector_num_out_ports_sfu,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_out_ports_mem", OPT_INT32, &gpgpu_operand_collector_num_out_ports_mem,
                           "number of collector unit in ports (default = 1)",
                           "1");
    option_parser_register(opp, "-gpgpu_operand_collector_num_out_ports_gen", OPT_INT32, &gpgpu_operand_collector_num_out_ports_gen,
                           "number of collector unit in ports (default = 0)",
                           "0");
    option_parser_register(opp, "-gpgpu_coalesce_arch", OPT_INT32, &gpgpu_coalesce_arch,
                            "Coalescing arch (default = 13, anything else is off for now)",
                            "13");
    option_parser_register(opp, "-gpgpu_num_sched_per_core", OPT_INT32, &gpgpu_num_sched_per_core,
                            "Number of warp schedulers per core",
                            "1");
    option_parser_register(opp, "-gpgpu_max_insn_issue_per_warp", OPT_INT32, &gpgpu_max_insn_issue_per_warp,
                            "Max number of instructions that can be issued per warp in one cycle by scheduler",
                            "2");
    option_parser_register(opp, "-gpgpu_simt_core_sim_order", OPT_INT32, &simt_core_sim_order,
                            "Select the simulation order of cores in a cluster (0=Fix, 1=Round-Robin)",
                            "1");
    option_parser_register(opp, "-gpgpu_pipeline_widths", OPT_CSTR, &pipeline_widths_string,
                            "Pipeline widths "
                            "ID_OC_SP,ID_OC_SFU,ID_OC_MEM,OC_EX_SP,OC_EX_SFU,OC_EX_MEM,EX_WB",
                            "1,1,1,1,1,1,1" );
    option_parser_register(opp, "-gpgpu_num_sp_units", OPT_INT32, &gpgpu_num_sp_units,
                            "Number of SP units (default=1)",
                            "1");
    option_parser_register(opp, "-gpgpu_num_sfu_units", OPT_INT32, &gpgpu_num_sfu_units,
                            "Number of SF units (default=1)",
                            "1");
    option_parser_register(opp, "-gpgpu_num_mem_units", OPT_INT32, &gpgpu_num_mem_units,
                            "Number if ldst units (default=1) WARNING: not hooked up to anything",
                             "1");
    option_parser_register(opp, "-gpgpu_scheduler", OPT_CSTR, &gpgpu_scheduler_string,
                                "Scheduler configuration: < lrr | gto | two_level_active > "
                                "If two_level_active:<num_active_warps>:<inner_prioritization>:<outer_prioritization>"
                                "For complete list of prioritization values see shader.h enum scheduler_prioritization_type"
                                "Default: gto",
                                 "gto");

    //added by kh(030816)
    g_hpcl_comp_config.reg_options(opp);
    ///

}

void gpgpu_sim_config::reg_options(option_parser_t opp)
{
    gpgpu_functional_sim_config::reg_options(opp);
    m_shader_config.reg_options(opp);
    m_memory_config.reg_options(opp);
    power_config::reg_options(opp);
   option_parser_register(opp, "-gpgpu_max_cycle", OPT_INT32, &gpu_max_cycle_opt,
               "terminates gpu simulation early (0 = no limit)",
               "0");
   option_parser_register(opp, "-gpgpu_max_insn", OPT_INT32, &gpu_max_insn_opt,
               "terminates gpu simulation early (0 = no limit)",
               "0");
   option_parser_register(opp, "-gpgpu_max_cta", OPT_INT32, &gpu_max_cta_opt,
               "terminates gpu simulation early (0 = no limit)",
               "0");
   option_parser_register(opp, "-gpgpu_runtime_stat", OPT_CSTR, &gpgpu_runtime_stat,
                  "display runtime statistics such as dram utilization {<freq>:<flag>}",
                  "10000:0");
   option_parser_register(opp, "-liveness_message_freq", OPT_INT64, &liveness_message_freq,
               "Minimum number of seconds between simulation liveness messages (0 = always print)",
               "1");
   option_parser_register(opp, "-gpgpu_flush_l1_cache", OPT_BOOL, &gpgpu_flush_l1_cache,
                "Flush L1 cache at the end of each kernel call",
                "0");

   option_parser_register(opp, "-gpgpu_flush_l2_cache", OPT_BOOL, &gpgpu_flush_l2_cache,
                   "Flush L2 cache at the end of each kernel call",
                   "0");

   option_parser_register(opp, "-gpgpu_deadlock_detect", OPT_BOOL, &gpu_deadlock_detect,
                "Stop the simulation at deadlock (1=on (default), 0=off)",
                "1");
   option_parser_register(opp, "-gpgpu_ptx_instruction_classification", OPT_INT32,
               &gpgpu_ptx_instruction_classification,
               "if enabled will classify ptx instruction types per kernel (Max 255 kernels now)",
               "0");
   option_parser_register(opp, "-gpgpu_ptx_sim_mode", OPT_INT32, &g_ptx_sim_mode,
               "Select between Performance (default) or Functional simulation (1)",
               "0");
   option_parser_register(opp, "-gpgpu_clock_domains", OPT_CSTR, &gpgpu_clock_domains,
                  "Clock Domain Frequencies in MhZ {<Core Clock>:<ICNT Clock>:<L2 Clock>:<DRAM Clock>}",
                  "500.0:2000.0:2000.0:2000.0");
   option_parser_register(opp, "-gpgpu_max_concurrent_kernel", OPT_INT32, &max_concurrent_kernel,
                          "maximum kernels that can run concurrently on GPU", "8" );
   option_parser_register(opp, "-gpgpu_cflog_interval", OPT_INT32, &gpgpu_cflog_interval,
               "Interval between each snapshot in control flow logger",
               "0");
   option_parser_register(opp, "-visualizer_enabled", OPT_BOOL,
                          &g_visualizer_enabled, "Turn on visualizer output (1=On, 0=Off)",
                          "1");
   option_parser_register(opp, "-visualizer_outputfile", OPT_CSTR,
                          &g_visualizer_filename, "Specifies the output log file for visualizer",
                          NULL);
   option_parser_register(opp, "-visualizer_zlevel", OPT_INT32,
                          &g_visualizer_zlevel, "Compression level of the visualizer output log (0=no comp, 9=highest)",
                          "6");
    option_parser_register(opp, "-trace_enabled", OPT_BOOL,
                          &Trace::enabled, "Turn on traces",
                          "0");
    option_parser_register(opp, "-trace_components", OPT_CSTR,
                          &Trace::config_str, "comma seperated list of traces to enable. "
                          "Complete list found in trace_streams.tup. "
                          "Default none",
                          "none");
    option_parser_register(opp, "-trace_sampling_core", OPT_INT32,
                          &Trace::sampling_core, "The core which is printed using CORE_DPRINTF. Default 0",
                          "0");
    option_parser_register(opp, "-trace_sampling_memory_partition", OPT_INT32,
                          &Trace::sampling_memory_partition, "The memory partition which is printed using MEMPART_DPRINTF. Default -1 (i.e. all)",
                          "-1");
   ptx_file_line_stats_options(opp);
}

/////////////////////////////////////////////////////////////////////////////

void increment_x_then_y_then_z( dim3 &i, const dim3 &bound)
{
   i.x++;
   if ( i.x >= bound.x ) {
      i.x = 0;
      i.y++;
      if ( i.y >= bound.y ) {
         i.y = 0;
         if( i.z < bound.z )
            i.z++;
      }
   }
}

void gpgpu_sim::launch( kernel_info_t *kinfo )
{
   unsigned cta_size = kinfo->threads_per_cta();
   if ( cta_size > m_shader_config->n_thread_per_shader ) {
      printf("Execution error: Shader kernel CTA (block) size is too large for microarch config.\n");
      printf("                 CTA size (x*y*z) = %u, max supported = %u\n", cta_size,
             m_shader_config->n_thread_per_shader );
      printf("                 => either change -gpgpu_shader argument in gpgpusim.config file or\n");
      printf("                 modify the CUDA source to decrease the kernel block size.\n");
      abort();
   }
   unsigned n=0;
   for(n=0; n < m_running_kernels.size(); n++ ) {
       if( (NULL==m_running_kernels[n]) || m_running_kernels[n]->done() ) {
           m_running_kernels[n] = kinfo;
           break;
       }
   }
   assert(n < m_running_kernels.size());
}

bool gpgpu_sim::can_start_kernel()
{
   for(unsigned n=0; n < m_running_kernels.size(); n++ ) {
       if( (NULL==m_running_kernels[n]) || m_running_kernels[n]->done() )
           return true;
   }
   return false;
}

bool gpgpu_sim::get_more_cta_left() const
{
   if (m_config.gpu_max_cta_opt != 0) {
      if( m_total_cta_launched >= m_config.gpu_max_cta_opt )
          return false;
   }
   for(unsigned n=0; n < m_running_kernels.size(); n++ ) {
       if( m_running_kernels[n] && !m_running_kernels[n]->no_more_ctas_to_run() )
           return true;
   }
   return false;
}

kernel_info_t *gpgpu_sim::select_kernel()
{
    for(unsigned n=0; n < m_running_kernels.size(); n++ ) {
        unsigned idx = (n+m_last_issued_kernel+1)%m_config.max_concurrent_kernel;
        if( m_running_kernels[idx] && !m_running_kernels[idx]->no_more_ctas_to_run() ) {
            m_last_issued_kernel=idx;
            // record this kernel for stat print if it is the first time this kernel is selected for execution
            unsigned launch_uid = m_running_kernels[idx]->get_uid();
            if (std::find(m_executed_kernel_uids.begin(), m_executed_kernel_uids.end(), launch_uid) == m_executed_kernel_uids.end()) {
               m_executed_kernel_uids.push_back(launch_uid);
               m_executed_kernel_names.push_back(m_running_kernels[idx]->name());
            }

            return m_running_kernels[idx];
        }
    }
    return NULL;
}

unsigned gpgpu_sim::finished_kernel()
{
    if( m_finished_kernel.empty() )
        return 0;
    unsigned result = m_finished_kernel.front();
    m_finished_kernel.pop_front();
    return result;
}

void gpgpu_sim::set_kernel_done( kernel_info_t *kernel )
{
    unsigned uid = kernel->get_uid();
    m_finished_kernel.push_back(uid);
    std::vector<kernel_info_t*>::iterator k;
    for( k=m_running_kernels.begin(); k!=m_running_kernels.end(); k++ ) {
        if( *k == kernel ) {
            *k = NULL;
            break;
        }
    }
    assert( k != m_running_kernels.end() );
}

void set_ptx_warp_size(const struct core_config * warp_size);

gpgpu_sim::gpgpu_sim( const gpgpu_sim_config &config )
    : gpgpu_t(config), m_config(config)
{
    m_shader_config = &m_config.m_shader_config;
    m_memory_config = &m_config.m_memory_config;
    set_ptx_warp_size(m_shader_config);
    ptx_file_line_stats_create_exposed_latency_tracker(m_config.num_shader());

#ifdef GPGPUSIM_POWER_MODEL
        m_gpgpusim_wrapper = new gpgpu_sim_wrapper(config.g_power_simulation_enabled,config.g_power_config_name);
#endif

    m_shader_stats = new shader_core_stats(m_shader_config);
    m_memory_stats = new memory_stats_t(m_config.num_shader(),m_shader_config,m_memory_config);
    average_pipeline_duty_cycle = (float *)malloc(sizeof(float));
    active_sms=(float *)malloc(sizeof(float));
    m_power_stats = new power_stat_t(m_shader_config,average_pipeline_duty_cycle,active_sms,m_shader_stats,m_memory_config,m_memory_stats);

    gpu_sim_insn = 0;
    gpu_tot_sim_insn = 0;
    gpu_tot_issued_cta = 0;
    gpu_deadlock = false;


    m_cluster = new simt_core_cluster*[m_shader_config->n_simt_clusters];
    for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++)
        m_cluster[i] = new simt_core_cluster(this,i,m_shader_config,m_memory_config,m_shader_stats,m_memory_stats);

    m_memory_partition_unit = new memory_partition_unit*[m_memory_config->m_n_mem];
    m_memory_sub_partition = new memory_sub_partition*[m_memory_config->m_n_mem_sub_partition];
    for (unsigned i=0;i<m_memory_config->m_n_mem;i++) {
        m_memory_partition_unit[i] = new memory_partition_unit(i, m_memory_config, m_memory_stats);
        for (unsigned p = 0; p < m_memory_config->m_n_sub_partition_per_memory_channel; p++) {
            unsigned submpid = i * m_memory_config->m_n_sub_partition_per_memory_channel + p;
            m_memory_sub_partition[submpid] = m_memory_partition_unit[i]->get_sub_partition(p);
        }
    }

    //added by kh (122815)
    g_hpcl_network_anal = new hpcl_network_anal();
    ///

    //added by kh(062416)
    #ifdef REC_COMP_MODULE
    if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_rec_comp_en == 1) {
      if(g_hpcl_comp_config.hpcl_rec_comp_algo >= hpcl_comp_config::INTER_COMP_DIFF_SM_PKT) {
	//g_multicast_en should be enabled before ICNT is initialized
	g_multicast_en = true;
      }
    }
    #endif
    ///

    icnt_wrapper_init();

    //added by kh(041216)
    if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::GLOBAL_CROSSBAR) {
      icnt_create(m_shader_config->n_simt_clusters,m_memory_config->m_n_mem_sub_partition);
    } else if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::MESH) {
      icnt_create(m_shader_config->n_simt_clusters,m_memory_config->m_n_mem);
    }
    ///

    time_vector_create(NUM_MEM_REQ_STAT);
    fprintf(stdout, "GPGPU-Sim uArch: performance model initialization complete.\n");

    m_running_kernels.resize( config.max_concurrent_kernel, NULL );
    m_last_issued_kernel = 0;
    m_last_cluster_issue = 0;
    *average_pipeline_duty_cycle=0;
    *active_sms=0;

    last_liveness_message_time = 0;

    //added by kh(053015)
    g_L2_config = m_memory_config->m_L2_config;
    ///

    //added by kh(042516)
    g_hpcl_comp_anal = new hpcl_comp_anal;
    g_hpcl_comp_anal->create(m_memory_config->m_n_mem_sub_partition);


    //added by kh(042916)
    g_hpcl_comp = new hpcl_comp;
    g_hpcl_comp->create(m_memory_config, m_global_mem);


    //added by kh(102016)
    if(g_hpcl_comp_config.hpcl_cache_comp_en == 1) 	{     assert(g_hpcl_comp_config.hpcl_comp_en == 1);    }


    //added by kh(030816)
    if(g_hpcl_comp_config.hpcl_comp_en == 1) {

      //deleted by kh(042916)
      //g_hpcl_comp = new hpcl_comp;
      //g_hpcl_comp->create(m_memory_config, m_global_mem);

      //deleted by kh(042516)
      //g_hpcl_comp_anal = new hpcl_comp_anal;
      //g_hpcl_comp_anal->create(m_memory_config->m_n_mem_sub_partition);
      #ifdef GLOB_COMP_MODULE
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	int node_no = (g_mc_placement_config.sm_node_list.size()+g_mc_placement_config.mc_node_list.size());
	g_hpcl_global_comp_pl_2B.clear();
	g_hpcl_global_comp_pl_2B.resize(node_no, NULL);
	for(int i = 0; i < g_mc_placement_config.mc_node_list.size(); i++) {
	    int mc_node_id = g_mc_placement_config.mc_node_list[i];
	    g_hpcl_global_comp_pl_2B[mc_node_id] = new hpcl_comp_pl<unsigned short>(6, mc_node_id);
	    g_hpcl_global_comp_pl_2B[mc_node_id]->create(32, HPCL_LFU);
	}

	g_hpcl_global_decomp_pl_2B.clear();
	g_hpcl_global_decomp_pl_2B.resize(node_no, NULL);
	for(int i = 0; i < g_mc_placement_config.sm_node_list.size(); i++) {
	    int sm_node_id = g_mc_placement_config.sm_node_list[i];
	    g_hpcl_global_decomp_pl_2B[sm_node_id] = new hpcl_decomp_pl<unsigned short>(5, sm_node_id);
	    g_hpcl_global_decomp_pl_2B[sm_node_id]->create(32, HPCL_LFU);
	}
	///
      }
      #endif

      //added by kh(031716)
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::LOCAL_WORD_MATCHING
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_MATCHING
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_SEQ
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL)
      {
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();

	//14 condition is added by kh(041816)
	int lwm_comp_word_size = g_hpcl_comp_config.hpcl_comp_word_size;

	if(lwm_comp_word_size == 2 || lwm_comp_word_size == 6 || lwm_comp_word_size == 14) {
	  g_hpcl_comp_lwm_pl_2B.clear();
	  g_hpcl_comp_lwm_pl_2B.resize(tot_sub_partition_no, NULL);
	  for(int i = 0; i < tot_sub_partition_no; i++) {
	      g_hpcl_comp_lwm_pl_2B[i] = new hpcl_comp_lwm_pl<unsigned short>(g_hpcl_comp_config.hpcl_dsc_lwm_comp_cycle,i);
	      g_hpcl_comp_lwm_pl_2B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	  }
	  g_hpcl_comp_lwm_2B_en = true;
	}

	//14 condition is added by kh(041816)
	if(lwm_comp_word_size == 4 || lwm_comp_word_size == 6 || lwm_comp_word_size == 14) {
	  g_hpcl_comp_lwm_pl_4B.clear();
	  g_hpcl_comp_lwm_pl_4B.resize(tot_sub_partition_no, NULL);
	  for(int i = 0; i < tot_sub_partition_no; i++) {
	      g_hpcl_comp_lwm_pl_4B[i] = new hpcl_comp_lwm_pl<unsigned int>(g_hpcl_comp_config.hpcl_dsc_lwm_comp_cycle,i);
	      g_hpcl_comp_lwm_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	  }
	  g_hpcl_comp_lwm_4B_en = true;
	}

	//14 condition is added by kh(041816)
	if(lwm_comp_word_size == 8 || lwm_comp_word_size == 14) {
	  g_hpcl_comp_lwm_pl_8B.clear();
	  g_hpcl_comp_lwm_pl_8B.resize(tot_sub_partition_no, NULL);
	  for(int i = 0; i < tot_sub_partition_no; i++) {
	      g_hpcl_comp_lwm_pl_8B[i] = new hpcl_comp_lwm_pl<unsigned long long>(g_hpcl_comp_config.hpcl_dsc_lwm_comp_cycle,i);
	      g_hpcl_comp_lwm_pl_8B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	  }
	  g_hpcl_comp_lwm_8B_en = true;
	}

	//added by kh(071816)
	//Decopmression Module for LWM
	g_hpcl_decomp_lwm_pl.clear();
	if(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle > 0) {
	  g_hpcl_decomp_lwm_pl.resize(m_shader_config->n_simt_clusters, NULL);
	  for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	    g_hpcl_decomp_lwm_pl[i] = new hpcl_decomp_lwm_pl<unsigned>(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle,i);
	    g_hpcl_decomp_lwm_pl[i]->create();
	  }
	}
	///

      }
      ///

      //added by kh(051816)
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::LOCAL_WORD_MATCHING2)
      {
      	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();

      	if(g_hpcl_comp_config.hpcl_comp_word_size == 2 || g_hpcl_comp_config.hpcl_comp_word_size == 14) {
      	  g_hpcl_comp_lwm2_pl_2B.clear();
      	  g_hpcl_comp_lwm2_pl_2B.resize(tot_sub_partition_no, NULL);
      	  for(int i = 0; i < tot_sub_partition_no; i++) {
      	      g_hpcl_comp_lwm2_pl_2B[i] = new hpcl_comp_lwm2_pl<unsigned short>(1,i);
      	      g_hpcl_comp_lwm2_pl_2B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
      	  }
      	}

      	if(g_hpcl_comp_config.hpcl_comp_word_size == 4 || g_hpcl_comp_config.hpcl_comp_word_size == 14) {
      	  g_hpcl_comp_lwm2_pl_4B.clear();
      	  g_hpcl_comp_lwm2_pl_4B.resize(tot_sub_partition_no, NULL);
      	  for(int i = 0; i < tot_sub_partition_no; i++) {
      	      g_hpcl_comp_lwm2_pl_4B[i] = new hpcl_comp_lwm2_pl<unsigned int>(1,i);
      	      g_hpcl_comp_lwm2_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
      	  }
      	}

      	if(g_hpcl_comp_config.hpcl_comp_word_size == 8 || g_hpcl_comp_config.hpcl_comp_word_size == 14) {
      	  g_hpcl_comp_lwm2_pl_8B.clear();
      	  g_hpcl_comp_lwm2_pl_8B.resize(tot_sub_partition_no, NULL);
      	  for(int i = 0; i < tot_sub_partition_no; i++) {
      	      g_hpcl_comp_lwm2_pl_8B[i] = new hpcl_comp_lwm2_pl<unsigned long long>(1,i);
      	      g_hpcl_comp_lwm2_pl_8B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
      	  }
      	}
      }
      ///

      if(g_hpcl_comp_config.hpcl_comp_algo >= hpcl_comp_config::LOCAL_WORD_MATCHING
      && g_hpcl_comp_config.hpcl_comp_algo <= hpcl_comp_config::END_OF_COMP_ALGO) {
	//added by kh(041816)
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	g_hpcl_comp_buffer.clear();
	g_hpcl_comp_buffer.resize(tot_sub_partition_no, NULL);
	for(int i = 0; i < tot_sub_partition_no; i++) {
	  g_hpcl_comp_buffer[i] = new hpcl_comp_buffer(i);
	  g_hpcl_comp_buffer[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size, m_shader_config->n_simt_clusters);
	}
	///

	//added by kh(071816)
	g_hpcl_decomp_buffer.clear();
	g_hpcl_decomp_buffer.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_buffer[i] = new hpcl_comp_buffer(i);
	  g_hpcl_decomp_buffer[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size, 0);
	}
	///

	//added by kh(082716)
	g_hpcl_decomp_cache_buffer.clear();
	g_hpcl_decomp_cache_buffer.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_cache_buffer[i] = new hpcl_comp_buffer(i);
	  g_hpcl_decomp_cache_buffer[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size, 0);
	}
	///

      }

      //added by kh(060916)
      g_hpcl_comp_lwm_aux = new hpcl_comp_lwm_aux;
      ///
    }

    //added by kh(042516)
    #ifdef REQ_COAL_MODULE
    //if(g_hpcl_comp_config.hpcl_req_coal_en == 1) {
      //added by kh(042516)
      g_hpcl_req_coal_buffer.clear();
      g_hpcl_req_coal_buffer.resize(m_shader_config->n_simt_clusters, NULL);
      for(unsigned i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_req_coal_buffer[i] = new hpcl_req_coal_buffer(i);
	  g_hpcl_req_coal_buffer[i]->create(g_hpcl_comp_config.hpcl_req_coal_buffer_size, m_memory_config->m_n_mem_sub_partition);
      }
      ///

      g_hpcl_coal_anal = new hpcl_coal_anal;
      g_hpcl_coal_anal->create(m_shader_config->n_simt_clusters);
    //}
    ///
    #endif


    if(g_hpcl_comp_config.hpcl_comp_en == 1) {
      // added by abpd (042816)
      abpd_anal_ptr = new abpd_comp_anal();

      //*********For ABPD local based*******************8
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::ABPD_LOCAL_WORD_MATCHING)
      {
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();

	//TODO: By default we are considering it for 4B word

	g_hpcl_comp_abpd_local_pl_4B.clear();
	g_hpcl_comp_abpd_local_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  g_hpcl_comp_abpd_local_pl_4B[i] = new hpcl_comp_abpd_local_pl<unsigned int>(1,i);
	  g_hpcl_comp_abpd_local_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	}
      }
      /********************************************** BDI based ************************************/


      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::BDI_WORD_MATCHING)
      {
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();

	//TODO: By default we are considering it for 4B word

	g_hpcl_comp_bdi_pl_4B.clear();
	g_hpcl_comp_bdi_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  g_hpcl_comp_bdi_pl_4B[i] = new hpcl_comp_bdi_pl<unsigned int>(bdi_comp_lat,i);
	  g_hpcl_comp_bdi_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	  g_hpcl_comp_bdi_pl_4B[i]->init_blocks();
	}

	//added by abpd(080216)
	g_hpcl_decomp_bdi_pl.clear();
	g_hpcl_decomp_bdi_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_bdi_pl[i] = new hpcl_decomp_bdi_pl<unsigned>(bdi_decomp_lat,i);
	  g_hpcl_decomp_bdi_pl[i]->create();
	}
	///

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_bdi_pl_l1D.clear();
	g_hpcl_decomp_bdi_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	    g_hpcl_decomp_bdi_pl_l1D[i] = new hpcl_decomp_bdi_pl<unsigned>(bdi_decomp_lat,i);
	  g_hpcl_decomp_bdi_pl_l1D[i]->create();
	}
	///

      }


      /********************************************** CPACK based ************************************/
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::CPACK_WORD_MATCHING)
      {
	// mcs
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	//TODO: By default we are considering it for 4B word

	g_hpcl_comp_cpack_pl_4B.clear();
	g_hpcl_comp_cpack_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  // cpack 9 stage pipeline for compression
	  g_hpcl_comp_cpack_pl_4B[i] = new hpcl_comp_cpack_pl<unsigned int>(cpack_comp_lat,i);
	  g_hpcl_comp_cpack_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	}

	//added by abpd(080216)
	g_hpcl_decomp_cpack_pl.clear();
	g_hpcl_decomp_cpack_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_cpack_pl[i] = new hpcl_decomp_cpack_pl<unsigned>(cpack_decomp_lat,i);
	  g_hpcl_decomp_cpack_pl[i]->create();
	}
	///

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_cpack_pl_l1D.clear();
	g_hpcl_decomp_cpack_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_cpack_pl_l1D[i] = new hpcl_decomp_cpack_pl<unsigned>(cpack_decomp_lat,i);
	  g_hpcl_decomp_cpack_pl_l1D[i]->create();
	}
	///

      }


      /********************************************** FPC based ************************************/
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::FPC_WORD_MATCHING)
      {
	// mcs
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	//TODO: By default we are considering it for 4B word

	g_hpcl_comp_fpc_pl_4B.clear();
	g_hpcl_comp_fpc_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  // cpack 3 stage pipeline for compression
	  g_hpcl_comp_fpc_pl_4B[i] = new hpcl_comp_fpc_pl<unsigned int>(fpc_comp_lat,i);
	  g_hpcl_comp_fpc_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	}

	//added by abpd(080216)
	g_hpcl_decomp_fpc_pl.clear();
	g_hpcl_decomp_fpc_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_fpc_pl[i] = new hpcl_decomp_fpc_pl<unsigned>(fpc_decomp_lat,i);
	  g_hpcl_decomp_fpc_pl[i]->create();
	}
	///

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_fpc_pl_l1D.clear();
	g_hpcl_decomp_fpc_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_fpc_pl_l1D[i] = new hpcl_decomp_fpc_pl<unsigned>(fpc_decomp_lat,i);
	  g_hpcl_decomp_fpc_pl_l1D[i]->create();
	}
	///
      }

      //added by abpd(080216)
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::SC2_WORD_MATCHING)
      {
	// mcs
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	//TODO: By default we are considering it for 4B word

	g_hpcl_comp_sc2_pl_4B.clear();

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  shared_ptr<hpcl_comp_sc2_pl<unsigned int> > ob(new hpcl_comp_sc2_pl<unsigned int>(sc2_comp_lat, i,(unsigned)g_hpcl_comp_config.hpcl_sc2_mode));
	  g_hpcl_comp_sc2_pl_4B.push_back(ob);
	  g_hpcl_comp_sc2_pl_4B.back()->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	}

	g_hpcl_decomp_sc2_pl.clear();
	g_hpcl_decomp_sc2_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_sc2_pl[i] = new hpcl_decomp_sc2_pl<unsigned>(sc2_decomp_lat, i);
	  g_hpcl_decomp_sc2_pl[i]->create();
	}

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_sc2_pl_l1D.clear();
	g_hpcl_decomp_sc2_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_sc2_pl_l1D[i] = new hpcl_decomp_sc2_pl<unsigned>(sc2_decomp_lat, i);
	  g_hpcl_decomp_sc2_pl_l1D[i]->create();
	}
	///
     }
     ///

      /********************************************** FPH based ************************************/
      //added by abpd(080216)
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::FPH_WORD_MATCHING)
      {
	// mcs
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	//TODO: By default we are considering it for 8B word

	g_hpcl_comp_fph_pl_4B.clear();
	//g_hpcl_comp_fph_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	  shared_ptr<hpcl_comp_fph_pl<unsigned int> > ob(new hpcl_comp_fph_pl<unsigned int>(fph_comp_lat,i,(unsigned)g_hpcl_comp_config.hpcl_fph_mode));
	  g_hpcl_comp_fph_pl_4B.push_back(ob);
	  g_hpcl_comp_fph_pl_4B.back()->create(g_hpcl_comp_config.hpcl_comp_buffer_size);

	}

	g_hpcl_decomp_fph_pl.clear();
	g_hpcl_decomp_fph_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_fph_pl[i] = new hpcl_decomp_fph_pl<unsigned>(fph_decomp_lat,i);
	  g_hpcl_decomp_fph_pl[i]->create();
	}

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_fph_pl_l1D.clear();
	g_hpcl_decomp_fph_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_fph_pl_l1D[i] = new hpcl_decomp_fph_pl<unsigned>(fph_decomp_lat,i);
	  g_hpcl_decomp_fph_pl_l1D[i]->create();
	}
	///

      }
      ///

      /********************************************** BPC based ************************************/
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::BPC_WORD_MATCHING)
      {
	// mcs
	unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
	//TODO: By default we are considering it for 8B word

	g_hpcl_comp_bpc_pl_4B.clear();
	g_hpcl_comp_bpc_pl_4B.resize(tot_sub_partition_no, NULL);

	for(int i = 0; i < tot_sub_partition_no; i++)
	{
	    // TODO pipeline stages for bpc for compression
	    g_hpcl_comp_bpc_pl_4B[i] = new hpcl_comp_bpc_pl<unsigned int>(bpc_comp_lat,i);
	    g_hpcl_comp_bpc_pl_4B[i]->create(g_hpcl_comp_config.hpcl_comp_buffer_size);
	}
	//added by abpd(080216)
	g_hpcl_decomp_bpc_pl.clear();
	g_hpcl_decomp_bpc_pl.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_bpc_pl[i] = new hpcl_decomp_bpc_pl<unsigned>(bpc_decomp_lat,i);
	  g_hpcl_decomp_bpc_pl[i]->create();
	}
	///

	//added by kh(102016)
	//Decompression for L1D Cache Compression
	g_hpcl_decomp_bpc_pl_l1D.clear();
	g_hpcl_decomp_bpc_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
	for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
	  g_hpcl_decomp_bpc_pl_l1D[i] = new hpcl_decomp_bpc_pl<unsigned>(bpc_decomp_lat,i);
	  g_hpcl_decomp_bpc_pl_l1D[i]->create();
	}
	///
      }

    }

    //added by kh(060216)
    g_NI_input_buffer_size = ::icnt_get_input_buffer_size();
    printf("g_NI_input_buffer_size %u \n", g_NI_input_buffer_size);
    ///

    //added by kh(062416)
    #ifdef REC_COMP_MODULE
    if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_rec_comp_en == 1) {

      g_hpcl_rec_comp_buffer.clear();
      g_hpcl_rec_comp_buffer.resize(m_memory_config->m_n_mem_sub_partition, NULL);
      for(int i = 0; i < m_memory_config->m_n_mem_sub_partition; i++) {
	g_hpcl_rec_comp_buffer[i] = new hpcl_rec_comp_buffer(i);
	g_hpcl_rec_comp_buffer[i]->create(g_hpcl_comp_config.hpcl_rec_comp_buffer_size);
	printf("g_hpcl_rec_comp_buffer_size %u\n", g_hpcl_comp_config.hpcl_rec_comp_buffer_size);
      }
      ///

      g_hpcl_rec_comp_lwm_pl.resize(m_memory_config->m_n_mem_sub_partition);
      unsigned pipeline_stage_no = 1;
      for(int i = 0; i < g_hpcl_rec_comp_lwm_pl.size(); i++) {
	g_hpcl_rec_comp_lwm_pl[i] = new hpcl_rec_comp_lwm_pl;
	g_hpcl_rec_comp_lwm_pl[i]->create(pipeline_stage_no, ::icnt_get_input_buffer_size()*32, g_hpcl_rec_comp_buffer[i], i, g_hpcl_comp_buffer[i]);
      }


    }
    #endif
    ///

    //added by kh(070316)
    if(g_hpcl_comp_config.hpcl_comp_en == 1)
    {
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_MATCHING
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_SEQ
      || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL
	  || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING2)
      {
		unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
		g_hpcl_comp_dsc_pl.clear();
		g_hpcl_comp_dsc_pl.resize(tot_sub_partition_no, NULL);
		for(int i = 0; i < tot_sub_partition_no; i++) {
			g_hpcl_comp_dsc_pl[i] = new hpcl_comp_dsc_pl(g_hpcl_comp_config.hpcl_dsc_lwm_comp_cycle, i);
			g_hpcl_comp_dsc_pl[i]->create();
		}


		//added by kh(071816)
		//Decopmression Module for DSM
		g_hpcl_decomp_dsc_pl.clear();
		if(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle > 0) {
			g_hpcl_decomp_dsc_pl.resize(m_shader_config->n_simt_clusters, NULL);
			for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
				g_hpcl_decomp_dsc_pl[i] = new hpcl_decomp_dsc_pl<unsigned>(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle,i);
				g_hpcl_decomp_dsc_pl[i]->create();
			}
		}
		///


		//added by kh(102016)
		//decompression unit for L1D cache
		g_hpcl_decomp_dsc_pl_l1D.clear();
		g_hpcl_decomp_dsc_pl_l1D.resize(m_shader_config->n_simt_clusters, NULL);
		for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
			g_hpcl_decomp_dsc_pl_l1D[i] = new hpcl_decomp_dsc_pl<unsigned>(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle,i);
			g_hpcl_decomp_dsc_pl_l1D[i]->create();
		}
		///

      }
    }
    ///


//    //added by kh(030117)
//    if(g_hpcl_comp_config.hpcl_comp_en == 1)
//	{
//    	if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING2) {
//			unsigned tot_sub_partition_no = g_hpcl_comp->get_mem_no() * g_hpcl_comp->get_sub_partition_no_per_memory_channel();
//			g_hpcl_comp_dsc2_pl.clear();
//			g_hpcl_comp_dsc2_pl.resize(tot_sub_partition_no, NULL);
//			for(int i = 0; i < tot_sub_partition_no; i++) {
//				g_hpcl_comp_dsc2_pl[i] = new hpcl_comp_dsc2_pl(g_hpcl_comp_config.hpcl_dsc_lwm_comp_cycle, i);
//				g_hpcl_comp_dsc2_pl[i]->create();
//			}
//
//
//			//added by kh(071816)
//			//Decopmression Module for DSM
//			g_hpcl_decomp_dsc2_pl.clear();
//			if(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle > 0) {
//				g_hpcl_decomp_dsc2_pl.resize(m_shader_config->n_simt_clusters, NULL);
//				for(int i = 0; i < m_shader_config->n_simt_clusters; i++) {
//					g_hpcl_decomp_dsc2_pl[i] = new hpcl_decomp_dsc2_pl<unsigned>(g_hpcl_comp_config.hpcl_dsc_lwm_decomp_cycle,i);
//					g_hpcl_decomp_dsc2_pl[i]->create();
//				}
//			}
//			///
//    	}
//	}
//    ///


    printf("hpcl_data_remap_function_list %s \n", g_hpcl_comp_config.hpcl_data_remap_function_list);
    //added by kh(071316)
    char* toks = new char[1024];
    strcpy(toks, g_hpcl_comp_config.hpcl_data_remap_function_list);
    toks = strtok(toks,",");
    bool is_end = false;
    while(toks != NULL) {
      unsigned func_index = 0;
      sscanf(toks,"%d", &func_index);
      g_hpcl_comp_config.hpcl_data_remap_function.push_back(func_index);
      toks = strtok(NULL,",");
    }
    //printf("toks %s \n", toks);
    delete[] toks;

    for(int i = 0; i < g_hpcl_comp_config.hpcl_data_remap_function.size(); i++) {
      printf("remap_function %u\n", g_hpcl_comp_config.hpcl_data_remap_function[i]);
    }
    ///
    //assert(0);


    //added by kh(081516)
    g_hpcl_approx_mem_space = new hpcl_approx_mem_space;
    g_hpcl_approx_mem_space->create();

    //added by kh(081516)
    printf("hpcl_approx_memcpy_index_list %s \n", g_hpcl_comp_config.hpcl_approx_memcpy_index_list);
    toks = new char[1024];
    strcpy(toks, g_hpcl_comp_config.hpcl_approx_memcpy_index_list);
    toks = strtok(toks,",");
    is_end = false;
    while(toks != NULL) {
      unsigned func_index = 0;
      sscanf(toks,"%d", &func_index);
      //g_hpcl_comp_config.hpcl_approx_memcpy_index.push_back(func_index);
      g_hpcl_comp_config.hpcl_approx_memcpy_index.insert(std::pair<unsigned,unsigned>(func_index,1));
      toks = strtok(NULL,",");
    }
    //printf("toks %s \n", toks);
    delete[] toks;

    std::map<unsigned,unsigned>::iterator it = g_hpcl_comp_config.hpcl_approx_memcpy_index.begin();
    for(; it != g_hpcl_comp_config.hpcl_approx_memcpy_index.end(); ++it) {
      printf("approx_memcpy_index %u\n", it->first);
    }
    /*
    for(int i = 0; i < g_hpcl_comp_config.hpcl_approx_memcpy_index.size(); i++) {
      printf("approx_memcpy_index %u\n", g_hpcl_comp_config.hpcl_approx_memcpy_index[i]);
    }
    */
    ///

	//added by kh(021617)
	g_hpcl_data_anal = new hpcl_data_anal;
	g_hpcl_data_anal->create(g_hpcl_comp_config.hpcl_char_type_pct_th);

    g_hpcl_data_reader = new hpcl_data_reader;
    g_hpcl_data_reader->create(m_memory_config, m_global_mem);
	///

}

int gpgpu_sim::shared_mem_size() const
{
   return m_shader_config->gpgpu_shmem_size;
}

int gpgpu_sim::num_registers_per_core() const
{
   return m_shader_config->gpgpu_shader_registers;
}

int gpgpu_sim::wrp_size() const
{
   return m_shader_config->warp_size;
}

int gpgpu_sim::shader_clock() const
{
   return m_config.core_freq/1000;
}

void gpgpu_sim::set_prop( cudaDeviceProp *prop )
{
   m_cuda_properties = prop;
}

const struct cudaDeviceProp *gpgpu_sim::get_prop() const
{
   return m_cuda_properties;
}

enum divergence_support_t gpgpu_sim::simd_model() const
{
   return m_shader_config->model;
}

void gpgpu_sim_config::init_clock_domains(void )
{
   sscanf(gpgpu_clock_domains,"%lf:%lf:%lf:%lf",
          &core_freq, &icnt_freq, &l2_freq, &dram_freq);
   core_freq = core_freq MhZ;
   icnt_freq = icnt_freq MhZ;
   l2_freq = l2_freq MhZ;
   dram_freq = dram_freq MhZ;
   core_period = 1/core_freq;
   icnt_period = 1/icnt_freq;
   dram_period = 1/dram_freq;
   l2_period = 1/l2_freq;
   printf("GPGPU-Sim uArch: clock freqs: %lf:%lf:%lf:%lf\n",core_freq,icnt_freq,l2_freq,dram_freq);
   printf("GPGPU-Sim uArch: clock periods: %.20lf:%.20lf:%.20lf:%.20lf\n",core_period,icnt_period,l2_period,dram_period);
}

void gpgpu_sim::reinit_clock_domains(void)
{
   core_time = 0;
   dram_time = 0;
   icnt_time = 0;
   l2_time = 0;
}

bool gpgpu_sim::active()
{
    if (m_config.gpu_max_cycle_opt && (gpu_tot_sim_cycle + gpu_sim_cycle) >= m_config.gpu_max_cycle_opt)
       return false;
    if (m_config.gpu_max_insn_opt && (gpu_tot_sim_insn + gpu_sim_insn) >= m_config.gpu_max_insn_opt)
       return false;
    if (m_config.gpu_max_cta_opt && (gpu_tot_issued_cta >= m_config.gpu_max_cta_opt) )
       return false;
    if (m_config.gpu_deadlock_detect && gpu_deadlock)
       return false;
    for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++)
       if( m_cluster[i]->get_not_completed()>0 )
           return true;;

    //added by kh(030816)
    #ifdef GLOB_COMP_MODULE
    if(g_hpcl_comp_config.hpcl_comp_en == 1) {
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	for(int i = 0; i < g_hpcl_global_decomp_pl_2B.size(); i++)
	  if(g_hpcl_global_decomp_pl_2B[i] && g_hpcl_global_decomp_pl_2B[i]->active())	return true;
      }
    }
    ///
    #endif

    for (unsigned i=0;i<m_memory_config->m_n_mem;i++)
       if( m_memory_partition_unit[i]->busy()>0 )
           return true;;

    #ifdef GLOB_COMP_MODULE
    //added by kh(030816)
    if(g_hpcl_comp_config.hpcl_comp_en == 1) {
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	for(int i = 0; i < g_hpcl_global_comp_pl_2B.size(); i++)
	  if(g_hpcl_global_comp_pl_2B[i] && g_hpcl_global_comp_pl_2B[i]->active())	return true;
      }
    }
    ///
    #endif

    if( icnt_busy() )
        return true;
    if( get_more_cta_left() )
        return true;
    return false;
}

void gpgpu_sim::init()
{
    // run a CUDA grid on the GPU microarchitecture simulator
    gpu_sim_cycle = 0;
    gpu_sim_insn = 0;
    last_gpu_sim_insn = 0;
    m_total_cta_launched=0;

    reinit_clock_domains();
    set_param_gpgpu_num_shaders(m_config.num_shader());
    for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++)
       m_cluster[i]->reinit();
    m_shader_stats->new_grid();
    // initialize the control-flow, memory access, memory latency logger
    if (m_config.g_visualizer_enabled) {
        create_thread_CFlogger( m_config.num_shader(), m_shader_config->n_thread_per_shader, 0, m_config.gpgpu_cflog_interval );
    }
    shader_CTA_count_create( m_config.num_shader(), m_config.gpgpu_cflog_interval);
    if (m_config.gpgpu_cflog_interval != 0) {
       insn_warp_occ_create( m_config.num_shader(), m_shader_config->warp_size );
       shader_warp_occ_create( m_config.num_shader(), m_shader_config->warp_size, m_config.gpgpu_cflog_interval);
       shader_mem_acc_create( m_config.num_shader(), m_memory_config->m_n_mem, 4, m_config.gpgpu_cflog_interval);
       shader_mem_lat_create( m_config.num_shader(), m_config.gpgpu_cflog_interval);
       shader_cache_access_create( m_config.num_shader(), 3, m_config.gpgpu_cflog_interval);
       set_spill_interval (m_config.gpgpu_cflog_interval * 40);
    }

    if (g_network_mode)
       icnt_init();

    //added by kh(092616)
    //clear inst_stat_per_kernel
    for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++){
      m_cluster[i]->clear_inst_stat_per_kernel();
    }
    ///

    // McPAT initialization function. Called on first launch of GPU
#ifdef GPGPUSIM_POWER_MODEL
    if(m_config.g_power_simulation_enabled){
        init_mcpat(m_config, m_gpgpusim_wrapper, m_config.gpu_stat_sample_freq,  gpu_tot_sim_insn, gpu_sim_insn);
    }
#endif
}

void gpgpu_sim::update_stats() {
    m_memory_stats->memlatstat_lat_pw();
    gpu_tot_sim_cycle += gpu_sim_cycle;
    gpu_tot_sim_insn += gpu_sim_insn;
}

void gpgpu_sim::print_stats()
{
    ptx_file_line_stats_write_file();
    gpu_print_stat();

    if (g_network_mode) {
        printf("----------------------------Interconnect-DETAILS--------------------------------\n" );
        icnt_display_stats();
        icnt_display_overall_stats();
        printf("----------------------------END-of-Interconnect-DETAILS-------------------------\n" );
    }

    //added by kh(062416)
    if(g_hpcl_comp_anal) {
      g_hpcl_comp_anal->update_overall_stat();
      g_hpcl_comp_anal->display();
    }
    ///

    //addded by kh(081516)
    if(g_hpcl_approx_mem_space) {
      g_hpcl_approx_mem_space->update_overall_stat();
      g_hpcl_approx_mem_space->display();
    }
    ///

    //added by kh(102016)
    if(g_hpcl_comp_config.hpcl_adaptive_comp_en == 1) {
      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
				m_cluster[i]->get_dyn_comp().print();
      }
    }
    ///

	//added by kh(021617)
	if(g_hpcl_data_anal) {
		g_hpcl_data_anal->display();
	}
	///





    //added by kh(070516)
    //The maximum instruction is set to 1000000
    /*
    unsigned long long max_insn = 1000000000;
    if(gpu_tot_sim_insn > max_insn) {
      //exit_simulation();
      printf ("Simulation maximum instructions %llu\n", max_insn);

      exit (EXIT_SUCCESS);
    }
    */

    //added by kh(073016)
    unsigned long long max_insn = g_hpcl_comp_config.hpcl_max_sim_inst_no;
    if(max_insn == 0) {
      //Run a benchmark to the end.
    } else {
      if(gpu_tot_sim_insn > max_insn) {
	//exit_simulation();
	printf ("Simulation maximum instructions %llu\n", max_insn);

	exit (EXIT_SUCCESS);
      }
    }
    ///


}

void gpgpu_sim::deadlock_check()
{
   if (m_config.gpu_deadlock_detect && gpu_deadlock) {
      fflush(stdout);
      printf("\n\nGPGPU-Sim uArch: ERROR ** deadlock detected: last writeback core %u @ gpu_sim_cycle %u (+ gpu_tot_sim_cycle %u) (%u cycles ago)\n",
             gpu_sim_insn_last_update_sid,
             (unsigned) gpu_sim_insn_last_update, (unsigned) (gpu_tot_sim_cycle-gpu_sim_cycle),
             (unsigned) (gpu_sim_cycle - gpu_sim_insn_last_update ));
      unsigned num_cores=0;
      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
         unsigned not_completed = m_cluster[i]->get_not_completed();
         if( not_completed ) {
             if ( !num_cores )  {
                 printf("GPGPU-Sim uArch: DEADLOCK  shader cores no longer committing instructions [core(# threads)]:\n" );
                 printf("GPGPU-Sim uArch: DEADLOCK  ");
                 m_cluster[i]->print_not_completed(stdout);
             } else if (num_cores < 8 ) {
                 m_cluster[i]->print_not_completed(stdout);
             } else if (num_cores >= 8 ) {
                 printf(" + others ... ");
             }
             num_cores+=m_shader_config->n_simt_cores_per_cluster;
         }
      }
      printf("\n");
      for (unsigned i=0;i<m_memory_config->m_n_mem;i++) {
         bool busy = m_memory_partition_unit[i]->busy();
         if( busy )
             printf("GPGPU-Sim uArch DEADLOCK:  memory partition %u busy\n", i );
      }
      if( icnt_busy() ) {
         printf("GPGPU-Sim uArch DEADLOCK:  iterconnect contains traffic\n");
         icnt_display_state( stdout );
      }
      printf("\nRe-run the simulator in gdb and use debug routines in .gdbinit to debug this\n");
      fflush(stdout);
      abort();
   }
}

/// printing the names and uids of a set of executed kernels (usually there is only one)
std::string gpgpu_sim::executed_kernel_info_string()
{
   std::stringstream statout;

   statout << "kernel_name = ";
   for (unsigned int k = 0; k < m_executed_kernel_names.size(); k++) {
      statout << m_executed_kernel_names[k] << " ";
   }
   statout << std::endl;
   statout << "kernel_launch_uid = ";
   for (unsigned int k = 0; k < m_executed_kernel_uids.size(); k++) {
      statout << m_executed_kernel_uids[k] << " ";
   }
   statout << std::endl;

   return statout.str();
}
void gpgpu_sim::set_cache_config(std::string kernel_name,  FuncCache cacheConfig )
{
	m_special_cache_config[kernel_name]=cacheConfig ;
}

FuncCache gpgpu_sim::get_cache_config(std::string kernel_name)
{
	for(std::map<std::string, FuncCache>::iterator iter = m_special_cache_config.begin(); iter != m_special_cache_config.end(); iter++)
	{
		std::string kernel= iter->first;
		if (kernel_name.compare(kernel) == 0){
			return iter->second;
		}
	}
	return (FuncCache)0;
}

bool gpgpu_sim::has_special_cache_config(std::string kernel_name)
{
	for(std::map<std::string, FuncCache>::iterator iter = m_special_cache_config.begin(); iter != m_special_cache_config.end(); iter++)
	{
		std::string kernel= iter->first;
		if (kernel_name.compare(kernel) == 0){
			return true;
		}
	}
	return false;
}


void gpgpu_sim::set_cache_config(std::string kernel_name)
{
	if(has_special_cache_config(kernel_name)){
		change_cache_config(get_cache_config(kernel_name));
	}else{
		change_cache_config(FuncCachePreferNone);
	}
}


void gpgpu_sim::change_cache_config(FuncCache cache_config)
{
	if(cache_config != m_shader_config->m_L1D_config.get_cache_status()){
		printf("FLUSH L1 Cache at configuration change between kernels\n");
		for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
			m_cluster[i]->cache_flush();
	    }
	}

	switch(cache_config){
	case FuncCachePreferNone:
		m_shader_config->m_L1D_config.init(m_shader_config->m_L1D_config.m_config_string, FuncCachePreferNone);
		m_shader_config->gpgpu_shmem_size=m_shader_config->gpgpu_shmem_sizeDefault;
		break;
	case FuncCachePreferL1:
		if((m_shader_config->m_L1D_config.m_config_stringPrefL1 == NULL) || (m_shader_config->gpgpu_shmem_sizePrefL1 == (unsigned)-1))
		{
			printf("WARNING: missing Preferred L1 configuration\n");
			m_shader_config->m_L1D_config.init(m_shader_config->m_L1D_config.m_config_string, FuncCachePreferNone);
			m_shader_config->gpgpu_shmem_size=m_shader_config->gpgpu_shmem_sizeDefault;

		}else{
			m_shader_config->m_L1D_config.init(m_shader_config->m_L1D_config.m_config_stringPrefL1, FuncCachePreferL1);
			m_shader_config->gpgpu_shmem_size=m_shader_config->gpgpu_shmem_sizePrefL1;
		}
		break;
	case FuncCachePreferShared:
		if((m_shader_config->m_L1D_config.m_config_stringPrefShared == NULL) || (m_shader_config->gpgpu_shmem_sizePrefShared == (unsigned)-1))
		{
			printf("WARNING: missing Preferred L1 configuration\n");
			m_shader_config->m_L1D_config.init(m_shader_config->m_L1D_config.m_config_string, FuncCachePreferNone);
			m_shader_config->gpgpu_shmem_size=m_shader_config->gpgpu_shmem_sizeDefault;
		}else{
			m_shader_config->m_L1D_config.init(m_shader_config->m_L1D_config.m_config_stringPrefShared, FuncCachePreferShared);
			m_shader_config->gpgpu_shmem_size=m_shader_config->gpgpu_shmem_sizePrefShared;
		}
		break;
	default:
		break;
	}
}


void gpgpu_sim::clear_executed_kernel_info()
{
   m_executed_kernel_names.clear();
   m_executed_kernel_uids.clear();
}
void gpgpu_sim::gpu_print_stat()
{
   FILE *statfout = stdout;

   std::string kernel_info_str = executed_kernel_info_string();
   fprintf(statfout, "%s", kernel_info_str.c_str());

   printf("gpu_sim_cycle = %lld\n", gpu_sim_cycle);
   printf("gpu_sim_insn = %lld\n", gpu_sim_insn);
   printf("gpu_ipc = %12.4f\n", (float)gpu_sim_insn / gpu_sim_cycle);
   printf("gpu_tot_sim_cycle = %lld\n", gpu_tot_sim_cycle+gpu_sim_cycle);
   printf("gpu_tot_sim_insn = %lld\n", gpu_tot_sim_insn+gpu_sim_insn);
   printf("gpu_tot_ipc = %12.4f\n", (float)(gpu_tot_sim_insn+gpu_sim_insn) / (gpu_tot_sim_cycle+gpu_sim_cycle));
   printf("gpu_tot_issued_cta = %lld\n", gpu_tot_issued_cta);



   // performance counter for stalls due to congestion.
   printf("gpu_stall_dramfull = %d\n", gpu_stall_dramfull);
   printf("gpu_stall_icnt2sh    = %d\n", gpu_stall_icnt2sh );

   time_t curr_time;
   time(&curr_time);
   unsigned long long elapsed_time = MAX( curr_time - g_simulation_starttime, 1 );
   printf( "gpu_total_sim_rate=%u\n", (unsigned)( ( gpu_tot_sim_insn + gpu_sim_insn ) / elapsed_time ) );

   //shader_print_l1_miss_stat( stdout );
   shader_print_cache_stats(stdout);

   cache_stats core_cache_stats;
   core_cache_stats.clear();
   for(unsigned i=0; i<m_config.num_cluster(); i++){
       m_cluster[i]->get_cache_stats(core_cache_stats);
   }
   printf("\nTotal_core_cache_stats:\n");
   core_cache_stats.print_stats(stdout, "Total_core_cache_stats_breakdown");
   shader_print_scheduler_stat( stdout, false );

   m_shader_stats->print(stdout);
#ifdef GPGPUSIM_POWER_MODEL
   if(m_config.g_power_simulation_enabled){
	   m_gpgpusim_wrapper->print_power_kernel_stats(gpu_sim_cycle, gpu_tot_sim_cycle, gpu_tot_sim_insn + gpu_sim_insn, kernel_info_str, true );
	   mcpat_reset_perf_count(m_gpgpusim_wrapper);
   }
#endif

   // performance counter that are not local to one shader
   m_memory_stats->memlatstat_print(m_memory_config->m_n_mem,m_memory_config->nbk);
   for (unsigned i=0;i<m_memory_config->m_n_mem;i++)
      m_memory_partition_unit[i]->print(stdout);

   // L2 cache stats
   if(!m_memory_config->m_L2_config.disabled()){
       cache_stats l2_stats;
       struct cache_sub_stats l2_css;
       struct cache_sub_stats total_l2_css;
       l2_stats.clear();
       l2_css.clear();
       total_l2_css.clear();

       printf("\n========= L2 cache stats =========\n");
       for (unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++){
           m_memory_sub_partition[i]->accumulate_L2cache_stats(l2_stats);
           m_memory_sub_partition[i]->get_L2cache_sub_stats(l2_css);

           fprintf( stdout, "L2_cache_bank[%d]: Access = %u, Miss = %u, Miss_rate = %.3lf, Pending_hits = %u, Reservation_fails = %u\n",
                    i, l2_css.accesses, l2_css.misses, (double)l2_css.misses / (double)l2_css.accesses, l2_css.pending_hits, l2_css.res_fails);

           total_l2_css += l2_css;
       }
       if (!m_memory_config->m_L2_config.disabled() && m_memory_config->m_L2_config.get_num_lines()) {
          //L2c_print_cache_stat();
          printf("L2_total_cache_accesses = %u\n", total_l2_css.accesses);
          printf("L2_total_cache_misses = %u\n", total_l2_css.misses);
          if(total_l2_css.accesses > 0)
              printf("L2_total_cache_miss_rate = %.4lf\n", (double)total_l2_css.misses/(double)total_l2_css.accesses);
          printf("L2_total_cache_pending_hits = %u\n", total_l2_css.pending_hits);
          printf("L2_total_cache_reservation_fails = %u\n", total_l2_css.res_fails);
          printf("L2_total_cache_breakdown:\n");
          l2_stats.print_stats(stdout, "L2_cache_stats_breakdown");
          total_l2_css.print_port_stats(stdout, "L2_cache");
       }
   }

   if (m_config.gpgpu_cflog_interval != 0) {
      spill_log_to_file (stdout, 1, gpu_sim_cycle);
      insn_warp_occ_print(stdout);
   }
   if ( gpgpu_ptx_instruction_classification ) {
      StatDisp( g_inst_classification_stat[g_ptx_kernel_count]);
      StatDisp( g_inst_op_classification_stat[g_ptx_kernel_count]);
   }

#ifdef GPGPUSIM_POWER_MODEL
   if(m_config.g_power_simulation_enabled){
       m_gpgpusim_wrapper->detect_print_steady_state(1,gpu_tot_sim_insn+gpu_sim_insn);
   }
#endif


   // Interconnect power stat print
   long total_simt_to_mem=0;
   long total_mem_to_simt=0;
   long temp_stm=0;
   long temp_mts = 0;
   for(unsigned i=0; i<m_config.num_cluster(); i++){
	   m_cluster[i]->get_icnt_stats(temp_stm, temp_mts);
	   total_simt_to_mem += temp_stm;
	   total_mem_to_simt += temp_mts;
   }
   printf("\nicnt_total_pkts_mem_to_simt=%ld\n", total_mem_to_simt);
   printf("icnt_total_pkts_simt_to_mem=%ld\n", total_simt_to_mem);

   time_vector_print();
   fflush(stdout);

   clear_executed_kernel_info();
}


// performance counter that are not local to one shader
unsigned gpgpu_sim::threads_per_core() const
{
   return m_shader_config->n_thread_per_shader;
}

void shader_core_ctx::mem_instruction_stats(const warp_inst_t &inst)
{
    unsigned active_count = inst.active_count();
    //this breaks some encapsulation: the is_[space] functions, if you change those, change this.
    switch (inst.space.get_type()) {
    case undefined_space:
    case reg_space:
        break;
    case shared_space:
        m_stats->gpgpu_n_shmem_insn += active_count;
        break;
    case const_space:
        m_stats->gpgpu_n_const_insn += active_count;
        break;
    case param_space_kernel:
    case param_space_local:
        m_stats->gpgpu_n_param_insn += active_count;
        break;
    case tex_space:
        m_stats->gpgpu_n_tex_insn += active_count;
        break;
    case global_space:
    case local_space:
        if( inst.is_store() )
            m_stats->gpgpu_n_store_insn += active_count;
        else
            m_stats->gpgpu_n_load_insn += active_count;
        break;
    default:
        abort();
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Launches a cooperative thread array (CTA).
 *
 * @param kernel
 *    object that tells us which kernel to ask for a CTA from
 */

void shader_core_ctx::issue_block2core( kernel_info_t &kernel )
{
    set_max_cta(kernel);

    // find a free CTA context
    unsigned free_cta_hw_id=(unsigned)-1;
    for (unsigned i=0;i<kernel_max_cta_per_shader;i++ ) {
      if( m_cta_status[i]==0 ) {
         free_cta_hw_id=i;
         break;
      }
    }
    assert( free_cta_hw_id!=(unsigned)-1 );

    // determine hardware threads and warps that will be used for this CTA
    int cta_size = kernel.threads_per_cta();

    // hw warp id = hw thread id mod warp size, so we need to find a range
    // of hardware thread ids corresponding to an integral number of hardware
    // thread ids
    int padded_cta_size = cta_size;
    if (cta_size%m_config->warp_size)
      padded_cta_size = ((cta_size/m_config->warp_size)+1)*(m_config->warp_size);
    unsigned start_thread = free_cta_hw_id * padded_cta_size;
    unsigned end_thread  = start_thread +  cta_size;

    // reset the microarchitecture state of the selected hardware thread and warp contexts
    reinit(start_thread, end_thread,false);

    // initalize scalar threads and determine which hardware warps they are allocated to
    // bind functional simulation state of threads to hardware resources (simulation)
    warp_set_t warps;
    unsigned nthreads_in_block= 0;
    for (unsigned i = start_thread; i<end_thread; i++) {
        m_threadState[i].m_cta_id = free_cta_hw_id;
        unsigned warp_id = i/m_config->warp_size;
        nthreads_in_block += ptx_sim_init_thread(kernel,&m_thread[i],m_sid,i,cta_size-(i-start_thread),m_config->n_thread_per_shader,this,free_cta_hw_id,warp_id,m_cluster->get_gpu());
        m_threadState[i].m_active = true;
        warps.set( warp_id );
    }
    assert( nthreads_in_block > 0 && nthreads_in_block <= m_config->n_thread_per_shader); // should be at least one, but less than max
    m_cta_status[free_cta_hw_id]=nthreads_in_block;

    // now that we know which warps are used in this CTA, we can allocate
    // resources for use in CTA-wide barrier operations
    m_barriers.allocate_barrier(free_cta_hw_id,warps);

    // initialize the SIMT stacks and fetch hardware
    init_warps( free_cta_hw_id, start_thread, end_thread);
    m_n_active_cta++;

    shader_CTA_count_log(m_sid, 1);
    printf("GPGPU-Sim uArch: core:%3d, cta:%2u initialized @(%lld,%lld)\n", m_sid, free_cta_hw_id, gpu_sim_cycle, gpu_tot_sim_cycle );
}

///////////////////////////////////////////////////////////////////////////////////////////

void dram_t::dram_log( int task )
{
   if (task == SAMPLELOG) {
      StatAddSample(mrqq_Dist, que_length());
   } else if (task == DUMPLOG) {
      printf ("Queue Length DRAM[%d] ",id);StatDisp(mrqq_Dist);
   }
}

//Find next clock domain and increment its time
int gpgpu_sim::next_clock_domain(void)
{
   double smallest = min3(core_time,icnt_time,dram_time);
   int mask = 0x00;
   if ( l2_time <= smallest ) {
      smallest = l2_time;
      mask |= L2 ;
      l2_time += m_config.l2_period;
   }
   if ( icnt_time <= smallest ) {
      mask |= ICNT;
      icnt_time += m_config.icnt_period;
   }
   if ( dram_time <= smallest ) {
      mask |= DRAM;
      dram_time += m_config.dram_period;
   }
   if ( core_time <= smallest ) {
      mask |= CORE;
      core_time += m_config.core_period;
   }
   return mask;
}

void gpgpu_sim::issue_block2core()
{
    unsigned last_issued = m_last_cluster_issue;
    for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
        unsigned idx = (i + last_issued + 1) % m_shader_config->n_simt_clusters;
        unsigned num = m_cluster[idx]->issue_block2core();
        if( num ) {
            m_last_cluster_issue=idx;
            m_total_cta_launched += num;
        }
    }
}

unsigned long long g_single_step=0; // set this in gdb to single step the pipeline


void gpgpu_sim::pass_mf_from_mem_partition_to_compressor()
{
  //Stage: Move mem_fetch from L2-to-ICNT queue to compressor and run compressor
  for (unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++) {
    mem_fetch* mf = m_memory_sub_partition[i]->top();
    bool has_comp_buffer_space = false;

    //added by kh(041816)
    has_comp_buffer_space = g_hpcl_comp_buffer[i]->has_comp_buffer_space();
    ///

    if(has_comp_buffer_space == true) {

      if(mf) {
		//added by kh(070616)
		if(g_hpcl_comp->is_data_retrievable(mf) == true)
		{
			unsigned char* real_data = mf->config_real_data(mf->get_data_size());
			g_hpcl_comp->get_cache_data(mf, real_data);


			#ifdef old
			//added by kh(111016)
			//To see top 1000 data patterns
			static unsigned long long count = 0;
			count++;
			//if(count <= 10000) {
				printf("%u Data(%03u) = ", count, mf->get_real_data_size());
				//printf("Data(%03u) = ", mf->get_real_data_size());
				for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
					printf("%02x", mf->get_real_data(i));
				}
				printf("\n");
			//} else {
			//  assert(0);
			//}
			#endif


			//printf("read_data_size = %u\n", mf->get_real_data_size());
		} else {

			//added by kh(070616)
			//If reply cannot be compressible, send it to comp_buffer directly
			g_hpcl_comp_buffer[i]->push_mem_fetch(mf);
			m_memory_sub_partition[i]->pop();
			continue;

		}
		COMP_DEBUG_PRINT("%llu | Stage1 | mf %u is sent to comp_buffer\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid());
      }

      //if(mf)	std::cout << "send mf " << mf->get_request_uid() << " to compressor!" << std::endl;
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::LOCAL_WORD_MATCHING) {

	if(g_hpcl_comp_config.hpcl_comp_word_size == 2) {
	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  //deleted by kh(041816)
	  //g_hpcl_comp_lwm_pl_2B[i]->run();				//run compressor
	  //added by kh(041816)
	  mem_fetch* ret_mf = g_hpcl_comp_lwm_pl_2B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	      ret_mf->set_comp_res(2);
	  }
	  ///
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) {
	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  //deleted by kh(041816)
	  //g_hpcl_comp_lwm_pl_4B[i]->run();				//run compressor
	  //added by kh(041816)
	  mem_fetch* ret_mf = g_hpcl_comp_lwm_pl_4B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	      ret_mf->set_comp_res(4);
	  }
	  ///
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) {
	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  //deleted by kh(041816)
	  //g_hpcl_comp_lwm_pl_8B[i]->run();				//run compressor
	  //added by kh(041816)
	  mem_fetch* ret_mf = g_hpcl_comp_lwm_pl_8B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      ret_mf->set_comp_res(8);
	  }
	  ///
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {

	  unsigned data_type_no = 1;

	  if(mf) {
	    if(g_hpcl_comp_config.hpcl_data_remap_en == 1) {
	      data_type_no += g_hpcl_comp_config.hpcl_data_remap_function.size();
	      g_hpcl_comp->remap_nonchar_data(mf);
	    }
	  }


	  unsigned res_no = 3;
	  std::vector<mem_fetch*> ret_mf(res_no, NULL);
	  std::vector<std::vector<unsigned> > comp_bits;	//comp_bits[data_type][resolution_index]
	  comp_bits.resize(data_type_no);
	  for(int k = 0; k < data_type_no; k++)	comp_bits[k].resize(res_no,0);

	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(mf);
	  ret_mf[0] = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	  if(ret_mf[0]) {
	    comp_bits[0][0] = ret_mf[0]->get_comp_data_bits(2);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][0] = ret_mf[0]->get_trans_comp_data_bits(2,k-1);
	  }

	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(mf);
	  ret_mf[1] = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	  if(ret_mf[1]) {
	    comp_bits[0][1] = ret_mf[1]->get_comp_data_bits(4);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][1] = ret_mf[1]->get_trans_comp_data_bits(4,k-1);
	  }

	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(mf);
	  ret_mf[2] = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	  if(ret_mf[2]) {
	    comp_bits[0][2] = ret_mf[2]->get_comp_data_bits(8);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][2] = ret_mf[2]->get_trans_comp_data_bits(8,k-1);
	  }

	  if(ret_mf[0]) {
	    std::vector<unsigned> min_lwm_comp_res_candi(data_type_no,0);
	    std::vector<unsigned> min_lwm_comp_bits_candi(data_type_no,0);

	    g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[0], min_lwm_comp_res_candi[0], min_lwm_comp_bits_candi[0]);
	    for(int k = 1; k < data_type_no; k++) {
	      g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[k], min_lwm_comp_res_candi[k], min_lwm_comp_bits_candi[k]);
	    }

	    unsigned min_comp_bits_index = 0;
	    unsigned min_lwm_comp_bits = min_lwm_comp_bits_candi[0];
	    unsigned min_lwm_comp_res = min_lwm_comp_res_candi[0];
	    enum mem_fetch::COMP_DATA_TYPE min_data_type = mem_fetch::ORG_DATA;
	    for(int i = 1; i < min_lwm_comp_bits_candi.size(); i++) {
	      if(min_lwm_comp_bits > min_lwm_comp_bits_candi[i]) {
		min_lwm_comp_bits = min_lwm_comp_bits_candi[i];
		min_lwm_comp_res = min_lwm_comp_res_candi[i];
		min_comp_bits_index = i;
	      }
	    }

	    if(min_comp_bits_index == 0)	min_data_type = mem_fetch::ORG_DATA;
	    else 							min_data_type = ret_mf[0]->get_remapped_data_type(min_comp_bits_index-1);

	    /*
	    if(min_data_type == mem_fetch::ORG_DATA) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE0_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, ORG_DATA_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_1) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE1_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA1_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_2) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE2_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA2_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_3) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE3_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA3_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_4) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE4_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_5) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE5_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else if(min_data_type == mem_fetch::REMAPPED_DATA_6) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE6_NO, 1);
	      //DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	    } else assert(0);
	    */

	    assert(min_lwm_comp_bits > 0);

	    enum mem_fetch::COMP_ALGO_TYPE comp_algo_type = mem_fetch::LWM_COMP;
	    if(min_lwm_comp_bits > ret_mf[0]->get_real_data_size()*8) {
	      min_lwm_comp_res = 0;
	      min_lwm_comp_bits = ret_mf[0]->get_real_data_size()*8+1;
	      comp_algo_type = mem_fetch::NO_COMP;
	    }

	    ret_mf[0]->set_comp_data_bits(min_lwm_comp_bits);
	    ret_mf[0]->set_comp_res(min_lwm_comp_res);
	    ret_mf[0]->set_comp_algo_type(comp_algo_type);
	    ret_mf[0]->set_dsc_comp_data_type(min_data_type);
	    ret_mf[0]->set_comp_data_size((unsigned)ceil(min_lwm_comp_bits/8.0));


	    //added by kh(090116)
	    if(min_lwm_comp_res == 0) {
	      ret_mf[0]->set_comp_ed_byte(ret_mf[0]->get_real_data_size());
	      ret_mf[0]->set_comp_es_byte(8);	//1bit is ceiled to 8byte
	    } else {
	      ret_mf[0]->set_comp_ed_byte(ret_mf[0]->get_lwm_comp_ed_byte(min_lwm_comp_res));
	      ret_mf[0]->set_comp_es_byte(ret_mf[0]->get_lwm_comp_es_byte(min_lwm_comp_res));
	    }
	    ///


	    g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf[0]);

	    //since dsm is not used.
	    //ret_mf[0]->set_dsc_comp_res(0);

	    /*
	    if(min_lwm_comp_res == 0) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_0B_NO, 1);
	    } else if(min_lwm_comp_res == 2) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	    } else if(min_lwm_comp_res == 4) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	    } else if(min_lwm_comp_res == 8) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	    } else assert(0);
	    */
	  }
	}

#ifdef old
	else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {		//hybrid compressor for 2/4/8

	  std::vector<mem_fetch*> ret_mf(3, NULL);
	  std::vector<unsigned> comp_bits(3,0);
	  std::vector<unsigned> trans_comp_bits(3,0);

	  if(mf) {

	    //added by kh(060616)
	    //transform data
	    g_hpcl_comp->transform_cache_data_type0(mf, 4, 0);
	    ///

	    //added by kh(070216)
	    //retransform data
	    g_hpcl_comp->transform_cache_data_type1(mf, 4, 0, 1);
	    ///


	    g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[0] = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	    comp_bits[0] = ret_mf[0]->get_comp_data_bits(2);

	    g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[1] = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	    comp_bits[1] = ret_mf[1]->get_comp_data_bits(4);

	    g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[2] = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	    comp_bits[2] = ret_mf[2]->get_comp_data_bits(8);

	    //added by kh(060916)
	    //when data remapping is applied to all data
	    //if(g_hpcl_comp_config.hpcl_trans_mode == hpcl_comp_config::PRE_TRANS) {
	    trans_comp_bits[0] = ret_mf[0]->get_trans_comp_data_bits(2);
	    trans_comp_bits[1] = ret_mf[0]->get_trans_comp_data_bits(4);
	    trans_comp_bits[2] = ret_mf[0]->get_trans_comp_data_bits(8);
	    //printf("2B_trans_comp = %u, 4B_trans_comp = %u, 8B_trans_comp = %u\n", comp_size[0], comp_size[1], comp_size[2]);


	    //added by kh(061016)
	    //Measure the distribution of max word index
	    g_hpcl_comp_lwm_aux->get_stat_for_max_word_index(ret_mf[0]);
	    ///

	    //added by kh(060916)
	    unsigned min_comp_bits = 0;
	    unsigned min_comp_res = 0;
	    //Choose the best compression result
	    if(g_hpcl_comp_config.hpcl_trans_mode < hpcl_comp_config::PRE_TRANS) {
	      g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits, min_comp_res, min_comp_bits);
	    } else if(g_hpcl_comp_config.hpcl_trans_mode == hpcl_comp_config::PRE_TRANS) {
	      g_hpcl_comp_lwm_aux->select_best_compressor(trans_comp_bits, min_comp_res, min_comp_bits);
	      mf->commit_trans_data_info();	//copy the trans data and dict
	    } else if(g_hpcl_comp_config.hpcl_trans_mode == hpcl_comp_config::PRE_TRANS_OPT) {

	      unsigned min_comp_bits_1 = 0;
	      unsigned min_comp_res_1 = 0;
	      unsigned min_comp_bits_2 = 0;
	      unsigned min_comp_res_2 = 0;
	      g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits, min_comp_res_1, min_comp_bits_1);
	      g_hpcl_comp_lwm_aux->select_best_compressor(trans_comp_bits, min_comp_res_2, min_comp_bits_2);
	      if(min_comp_bits_2 <= min_comp_bits_1) {
		  min_comp_bits = min_comp_bits_2;
		min_comp_res = min_comp_res_2;
		mf->commit_trans_data_info(min_comp_res);	//copy the trans data and dict
	      } else {
		min_comp_bits = min_comp_bits_1;
		min_comp_res = min_comp_res_1;

		//added by kh(061316)
		#ifdef DATA_TRANSFORMATION_WORSE_CASE_ANALYSIS
		static unsigned worse_case_count = 10;

		printf("DATA_TRANSFORMATION WORSE CASE %u, Before DT Size %u Res %u | After DT Size %u Res %u\n",
		       worse_case_count, min_comp_size_1, min_comp_res_1, min_comp_size_2, min_comp_res_2);

		printf("Org_Data(%03u) = ", mf->get_real_data_size());
		for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_real_data(j));
		}
		printf("\n");

		printf("Trans_Data(%03u) = ", mf->get_trans_data_size());
		for(int j = mf->get_trans_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_trans_data(j));
		}
		printf("\n");


		if(min_comp_res_1 == 2) ((hpcl_dict<unsigned short>*) mf->get_loc_dict(min_comp_res_1))->print();
		else if(min_comp_res_1 == 4) ((hpcl_dict<unsigned int>*) mf->get_loc_dict(min_comp_res_1))->print();
		else if(min_comp_res_1 == 8) ((hpcl_dict<unsigned long long>*) mf->get_loc_dict(min_comp_res_1))->print();


		if(min_comp_res_2 == 2) ((hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(min_comp_res_2))->print();
		else if(min_comp_res_2 == 4) ((hpcl_dict<unsigned int>*) mf->get_trans_loc_dict(min_comp_res_2))->print();
		else if(min_comp_res_2 == 8) ((hpcl_dict<unsigned long long>*) mf->get_trans_loc_dict(min_comp_res_2))->print();

		worse_case_count--;
		if(worse_case_count == 0) {
		  assert(0);
		}

		#endif
	      }
	    }

	    //deleted by kh(070516)
	    /*
	    //added by kh(061016)
	    //Measure comp data ratio segment-by-segment
	    g_hpcl_comp_lwm_aux->get_stat_for_comp_data_ratio_segment(ret_mf[0], min_comp_res);
	    ///
	    */

	    //Measure how many data is not actually compressed.
	    bool is_data_compressed = g_hpcl_comp_lwm_aux->is_data_compressed(mf, min_comp_res);
	    ///

	    //added by kh(060216)
	    if(is_data_compressed == false) {

	      //deleted by kh(060616)
	      //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_0B_NO, 1);

	      //if no compression happens, use 2B comp result for recursive compression
	      min_comp_res = 2;
	      min_comp_bits = comp_bits[0];
	      ///

	      #ifdef DATA_TRANSFORMATION_TEST
	      //added by kh(060616)
	      //data transformation test
	      printf("Org_Data(%03u) = ", mf->get_real_data_size());
	      for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
		printf("%02x ", mf->get_real_data(j));
	      }
	      printf("\n");

	      printf("Trans_Data(%03u) = ", mf->get_trans_data_size());
	      for(int j = mf->get_trans_data_size()-1; j >= 0 ; j--) {
		printf("%02x ", mf->get_trans_data(j));
	      }
	      printf("\n");
	      ///

	      loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(2);
	      loc_dict_2B->print();


	      hpcl_dict<unsigned short>* trans_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(2);
	      assert(trans_loc_dict_2B);
	      trans_loc_dict_2B->print();

	      static int count = 0;
	      count++;

	      //printf("comp_data_size %u, comp_trans_data_size %u\n", min_comp_size, mf->get_comp_trans_data_size());
	      if(count == 5) {
	    	//assert(0);
	      }
	      #endif

	      if(g_hpcl_comp_config.hpcl_trans_mode == hpcl_comp_config::NO_TRANS) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_0B_NO, 1);
	      } else if(g_hpcl_comp_config.hpcl_trans_mode > hpcl_comp_config::NO_TRANS
	        && g_hpcl_comp_config.hpcl_trans_mode < hpcl_comp_config::PRE_TRANS)
		{
		  unsigned comp_trans_data_size = mf->get_trans_comp_data_bits(2);
		  if(comp_bits[0] > comp_trans_data_size) {
		    printf("comp_trans_data_size %u wins over comp_data_size %u!!!\n", comp_trans_data_size, comp_bits[0]);
		    min_comp_bits= comp_trans_data_size;
		    mf->commit_trans_data_info(2);	//copy the trans data and dict
		    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
		  } else {
		    printf("comp_trans_data_size %u loses over comp_data_size %u!!!\n", comp_trans_data_size, comp_bits[0]);
		    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_0B_NO, 1);
		  }
		}
	    } else {
	      if(min_comp_res == 2) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);

		//added by kh(060616)
		unsigned comp_trans_data_bits = mf->get_trans_comp_data_bits(2);
		if(min_comp_bits > comp_trans_data_bits) {
		    //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::TRANS_COMP_2B_WIN_NO, 1);

		    if(g_hpcl_comp_config.hpcl_trans_mode >= hpcl_comp_config::POST_TRANS_UPTO_COMP_2B
		    && g_hpcl_comp_config.hpcl_trans_mode < hpcl_comp_config::PRE_TRANS) {
			min_comp_bits = comp_trans_data_bits;
			mf->commit_trans_data_info(2);	//copy the trans data and dict
		    }
		}
		///

		#ifdef DATA_TRANSFORMATION_TEST
		printf("Org_Data_COMP_2B(%03u) = ", mf->get_real_data_size());
		for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_real_data(j));
		}
		printf("\n");

		printf("Trans_Data_COMP_2B(%03u) = ", mf->get_trans_data_size());
		for(int j = mf->get_trans_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_trans_data(j));
		}
		printf("\n");
		///
		#endif

	      }
	      else if(min_comp_res == 4) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);

		#ifdef DATA_TRANSFORMATION_TEST
		printf("Org_Data_COMP_4B(%03u) = ", mf->get_real_data_size());
		for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_real_data(j));
		}
		printf("\n");

		printf("Trans_Data_COMP_4B(%03u) = ", mf->get_trans_data_size());
		for(int j = mf->get_trans_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_trans_data(j));
		}
		printf("\n");
		#endif
	      }
	      else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      else assert(0);
	    }




	    //assert(mf == ret_mf[0]);
	    //assert(ret_mf[0] == ret_mf[1]);
	    //assert(ret_mf[1] == ret_mf[2]);
	    mf->set_comp_data_bits(min_comp_bits);
	    mf->set_comp_res(min_comp_res);
	    mf->set_comp_data_size((unsigned)ceil(min_comp_bits/8.0));

	    g_hpcl_comp_buffer[i]->push_mem_fetch(mf);

	  }

	}
#endif



      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING
    	   || g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING2)
      {
		  //remap data
		  if(mf) {
			//deleted by kh(021817)
			//g_hpcl_comp->remap_nonchar_data(mf);

			//added by kh(021817)

			if(g_hpcl_comp_config.hpcl_char_preproc_en == 1) {

				double char_type_pct = g_hpcl_comp->get_char_type_pct(mf);
				//printf("mf %u char_type_pct %f\n", mf->get_request_uid(), char_type_pct);
				if(char_type_pct >= g_hpcl_comp_config.hpcl_char_type_pct_th) {

					//Convert predefined symbols.
					if(g_hpcl_comp_config.hpcl_char_symbol_conv_en == 1) {
						g_hpcl_comp->convert_char_symbols(mf);
					}
					///
					g_hpcl_comp->remap_char_data(mf);
					mf->set_chartype();

					//added by kh(030217)
					g_hpcl_data_anal->analyze_data_type(mf->get_real_data_ptr(), true, char_type_pct);
					///

				} else {
					g_hpcl_comp->remap_nonchar_data(mf);

					//added by kh(030217)
					g_hpcl_data_anal->analyze_data_type(mf->get_real_data_ptr(), false, char_type_pct);
					///
				}

			} else {
				g_hpcl_comp->remap_nonchar_data(mf);
			}
		  }

		  g_hpcl_comp_dsc_pl[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
		  mem_fetch* ret_mf = g_hpcl_comp_dsc_pl[i]->run();			//run compressor
		  if(ret_mf) {

			//added by kh(090116)
			ret_mf->set_comp_ed_byte(ret_mf->get_dsc_comp_ed_byte());
			ret_mf->set_comp_es_byte(ret_mf->get_dsc_comp_es_byte());
			///

			g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
		  }

      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_MATCHING)
      {
      	  //Stage2: LWM Compression and push result to comp_buffer
	  std::vector<mem_fetch*> ret_mf(3, NULL);
	  std::vector<unsigned> comp_bits(3,0);
	  std::vector<unsigned> remapped_comp_bits_type0(3,0);
	  std::vector<unsigned> remapped_comp_bits_type1(3,0);

	  ret_mf[0] = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	  if(ret_mf[0]) {
	    comp_bits[0] = ret_mf[0]->get_comp_data_bits(2);
	    remapped_comp_bits_type0[0] = ret_mf[0]->get_trans_comp_data_bits(2,0);
	    remapped_comp_bits_type1[0] = ret_mf[0]->get_trans_comp_data_bits(2,1);
	  }

	  ret_mf[1] = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	  if(ret_mf[1]) {
	    comp_bits[1] = ret_mf[1]->get_comp_data_bits(4);
	    remapped_comp_bits_type0[1] = ret_mf[1]->get_trans_comp_data_bits(4,0);
	    remapped_comp_bits_type1[1] = ret_mf[1]->get_trans_comp_data_bits(4,1);
	  }

	  ret_mf[2] = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	  if(ret_mf[2]) {
	    comp_bits[2] = ret_mf[2]->get_comp_data_bits(8);
	    remapped_comp_bits_type0[2] = ret_mf[2]->get_trans_comp_data_bits(8,0);
	    remapped_comp_bits_type1[2] = ret_mf[2]->get_trans_comp_data_bits(8,1);
	  }

	  if(ret_mf[0]) {
	    unsigned min_part_comp_bits = 0;
	    unsigned min_comp_res = 0;
	    enum mem_fetch::COMP_DATA_TYPE min_data_type = mem_fetch::NO_DATA_TYPE;

	    if(g_hpcl_comp_config.hpcl_dsc_lwm_opt_en == 0) {

	      g_hpcl_comp_lwm_aux->overlay_comp_data(ret_mf[0], ret_mf[0]->get_dsc_comp_data_type(), min_part_comp_bits, min_comp_res, min_data_type);

	    } else {

	      if(ret_mf[0]->get_dsc_comp_data_type() > mem_fetch::NO_DATA_TYPE) {

		std::vector<unsigned> opt_comp_bits(3,0);
		std::vector<unsigned> opt_comp_res(3,0);
		std::vector<enum mem_fetch::COMP_DATA_TYPE> opt_data_type(3,mem_fetch::NO_DATA_TYPE);
		g_hpcl_comp_lwm_aux->overlay_comp_data(ret_mf[0], mem_fetch::ORG_DATA, opt_comp_bits[0], opt_comp_res[0], opt_data_type[0]);
		g_hpcl_comp_lwm_aux->overlay_comp_data(ret_mf[0], mem_fetch::REMAPPED_DATA_1, opt_comp_bits[1], opt_comp_res[1], opt_data_type[1]);
		g_hpcl_comp_lwm_aux->overlay_comp_data(ret_mf[0], mem_fetch::REMAPPED_DATA_2, opt_comp_bits[2], opt_comp_res[2], opt_data_type[2]);

		#ifdef DATA_SEG_LWM_COMP_DEBUG
		ret_mf[0]->print_data(mem_fetch::ORG_DATA);
		ret_mf[0]->print_data(mem_fetch::REMAPPED_DATA_1);
		ret_mf[0]->print_data(mem_fetch::REMAPPED_DATA_2);

		for(int j = 0; j < 3; j++) {
		  DATA_SEG_LWM_COMP_DEBUG_PRINT("opt_comp_bits[%d] = %u, ", j, opt_comp_bits[j]);

		  if(j==0) DATA_SEG_LWM_COMP_DEBUG_PRINT("dsc_comp_bits[%d] = %u", j, ret_mf[0]->get_dsc_compressed_bits_only(mem_fetch::ORG_DATA));
		  else if(j==1) DATA_SEG_LWM_COMP_DEBUG_PRINT("dsc_comp_bits[%d] = %u", j, ret_mf[0]->get_dsc_compressed_bits_only(mem_fetch::REMAPPED_DATA_1));
		  else if(j==2) DATA_SEG_LWM_COMP_DEBUG_PRINT("dsc_comp_bits[%d] = %u", j, ret_mf[0]->get_dsc_compressed_bits_only(mem_fetch::REMAPPED_DATA_2));

		  DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");
		}
		#endif


		unsigned opt_min_overlayed_comp_bits = opt_comp_bits[0] + ret_mf[0]->get_dsc_compressed_bits_only(mem_fetch::ORG_DATA);
		unsigned opt_min_dsc_compressed_bits_only = ret_mf[0]->get_dsc_compressed_bits_only(mem_fetch::ORG_DATA);
		unsigned opt_min_part_comp_bits = opt_comp_bits[0];
		unsigned opt_min_comp_res = opt_comp_res[0];
		enum mem_fetch::COMP_DATA_TYPE opt_min_data_type = mem_fetch::ORG_DATA;
		for(int j = 1; j < 3; j++) {
		  enum mem_fetch::COMP_DATA_TYPE _data_type = mem_fetch::NO_DATA_TYPE;
		  if(j == 1)  	_data_type = mem_fetch::REMAPPED_DATA_1;
		  else if(j == 2)	_data_type = mem_fetch::REMAPPED_DATA_2;

		  unsigned _opt_overlayed_comp_bits = opt_comp_bits[j] + ret_mf[0]->get_dsc_compressed_bits_only(_data_type);
		  if(_opt_overlayed_comp_bits < opt_min_overlayed_comp_bits) {
		    opt_min_overlayed_comp_bits = _opt_overlayed_comp_bits;
		    opt_min_data_type = _data_type;
		    opt_min_dsc_compressed_bits_only = ret_mf[0]->get_dsc_compressed_bits_only(_data_type);
		    opt_min_part_comp_bits = opt_comp_bits[j];
		    opt_min_comp_res = opt_comp_res[j];
		  }
		}

		#ifdef DATA_SEG_LWM_COMP_DEBUG
		if(ret_mf[0]->get_dsc_comp_data_type() == opt_min_data_type)
		{
		  DATA_SEG_LWM_COMP_DEBUG_PRINT("1: Select the optimal data type %d (prev type %d)\n", opt_min_data_type, ret_mf[0]->get_dsc_comp_data_type());
		} else {
		  DATA_SEG_LWM_COMP_DEBUG_PRINT("2: Select the optimal data type %d (prev type %d)\n", opt_min_data_type, ret_mf[0]->get_dsc_comp_data_type());
		}
		DATA_SEG_LWM_COMP_DEBUG_PRINT("\tmin_part_comp_bits %u, dsc_compressed_bits_only %u\n", opt_min_dsc_compressed_bits_only);
		#endif

		//resave dsc_compress_bits_only here
		ret_mf[0]->set_dsc_compressed_bits_only(opt_min_dsc_compressed_bits_only);
		ret_mf[0]->set_dsc_comp_data_type(opt_min_data_type);
		min_part_comp_bits = opt_min_part_comp_bits;
		min_comp_res = opt_min_comp_res;

	      } else {
		g_hpcl_comp_lwm_aux->overlay_comp_data(ret_mf[0], ret_mf[0]->get_dsc_comp_data_type(), min_part_comp_bits, min_comp_res, min_data_type);
	      }

	    }

	    unsigned overlayed_comp_bits = min_part_comp_bits + ret_mf[0]->get_dsc_compressed_bits_only();
	    ret_mf[0]->set_comp_data_bits(overlayed_comp_bits);

	    //set LWM resolution with minimum data size
	    if(min_part_comp_bits > 0) {	//if LWM is used
	      ret_mf[0]->set_comp_res(min_comp_res);

	      if(min_comp_res == 0)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_0B_NO, 1);
	      else if(min_comp_res == 2)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	      else if(min_comp_res == 4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	      else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);

	    }
	    DATA_SEG_LWM_COMP_DEBUG_PRINT("MF %u min_part_comp_bits %u final_comp_bits %u\n", ret_mf[0]->get_request_uid(), min_part_comp_bits, overlayed_comp_bits);

	  }

	  ///ret_mf[0] ~ ret_mf[2] should be equal to each other.
	  if(ret_mf[0]) {
	    g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf[0]);
	  }
	  /// end of Stage2

	  //Stage1: Data Remapping and Data Segment Compression and insert non-fully-compressed data to LWM
      	  //Data Remapping
	  if(mf) {
      	    g_hpcl_comp->transform_cache_data_type0(mf, 4, 0);
      	    g_hpcl_comp->transform_cache_data_type1(mf, 4, 0, 1);
      	    g_hpcl_comp->transform_cache_data_type0(mf, 8, 2);
      	    g_hpcl_comp->transform_cache_data_type1(mf, 8, 2, 3);
      	  }

      	  g_hpcl_comp_dsc_pl[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  mem_fetch* out_mf = g_hpcl_comp_dsc_pl[i]->run();		//run compressor

	  if(out_mf) {
	    if(out_mf->has_uncompressed_ds() == true) {
	      DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has Uncomp DS)---\n", out_mf->get_request_uid());

	      bool is_all_ds_uncomp = true;
	      for(int m = 0; m < out_mf->get_comp_ds_status_size(); m++) {
		if(out_mf->get_comp_ds_status(m) == 1) {
		  is_all_ds_uncomp = false;
		  break;
		}
	      }
	      if(is_all_ds_uncomp == true) {
		DATA_SEG_COMP_DEBUG_PRINT("\tALL DS Uncompressed\n");
	      }

	    } else {
	      DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has No Uncomp DS)---\n", out_mf->get_request_uid());
	    }
	    for(int k = 0; k < out_mf->get_comp_ds_status_size(); k++) {
	      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d Status %u\n", k, out_mf->get_comp_ds_status(k));
	    }
	  }
	  /*
	  if(out_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(out_mff);
	  }
	  */

	  //Pass mf to LWM
	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  /// end of Stage1
      }
      //added by khkim (071216)
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_SEQ)
      {
	  //Stage2: LWM Compression and push result to comp_buffer
	  unsigned data_type_no = g_hpcl_comp_config.hpcl_data_remap_function.size()+1;
	  unsigned res_no = 3;
	  std::vector<mem_fetch*> ret_mf(res_no, NULL);
	  std::vector<std::vector<unsigned> > comp_bits;	//comp_bits[data_type][resolution_index]
	  comp_bits.resize(data_type_no);
	  for(int k = 0; k < data_type_no; k++)	comp_bits[k].resize(res_no,0);

	  ret_mf[0] = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	  if(ret_mf[0]) {
	    comp_bits[0][0] = ret_mf[0]->get_comp_data_bits(2);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][0] = ret_mf[0]->get_trans_comp_data_bits(2,k-1);
	  }

	  ret_mf[1] = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	  if(ret_mf[1]) {
	    comp_bits[0][1] = ret_mf[1]->get_comp_data_bits(4);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][1] = ret_mf[1]->get_trans_comp_data_bits(4,k-1);
	  }

	  ret_mf[2] = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	  if(ret_mf[2]) {
	    comp_bits[0][2] = ret_mf[2]->get_comp_data_bits(8);
	    for(int k = 1; k < data_type_no; k++) comp_bits[k][2] = ret_mf[2]->get_trans_comp_data_bits(8,k-1);
	  }


	  if(ret_mf[0]) {
	    unsigned min_dsm_comp_bits = ret_mf[0]->get_comp_data_bits();
	    unsigned min_dsm_comp_res = ret_mf[0]->get_dsc_comp_res();
	    unsigned min_lwm_comp_bits = 0;
	    unsigned min_lwm_comp_res = 0;
	    enum mem_fetch::COMP_DATA_TYPE min_data_type = ret_mf[0]->get_dsc_comp_data_type();

	    if(min_data_type > mem_fetch::NO_DATA_TYPE) {

	      int min_comp_bits_index = -1;
	      if(min_data_type == mem_fetch::ORG_DATA)	min_comp_bits_index = 0;
	      else {
		int remapped_data_index = ret_mf[0]->get_remapped_data_type_index(min_data_type);
		assert(remapped_data_index >= 0);
		min_comp_bits_index = remapped_data_index+1;
	      }

	      g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[min_comp_bits_index], min_lwm_comp_res, min_lwm_comp_bits);

//	      std::cout << "mf " << ret_mf[0]->get_request_uid() << std::endl;
//	      std::cout << " comp_bits_2B " << comp_bits[min_data_type][0];
//	      std::cout << " comp_bits_2B " << comp_bits[min_data_type][1];
//	      std::cout << " comp_bits_2B " << comp_bits[min_data_type][2];

	    } else {

	      if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::OPT_DATA) {
		unsigned data_type_no = ret_mf[0]->get_trans_data_no()+1;
		std::vector<unsigned> min_lwm_comp_res_candi(data_type_no,0);
		std::vector<unsigned> min_lwm_comp_bits_candi(data_type_no,0);

		/*
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[mem_fetch::ORG_DATA], min_lwm_comp_res_candi[0], min_lwm_comp_bits_candi[0]);
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[mem_fetch::REMAPPED_DATA_1], min_lwm_comp_res_candi[1], min_lwm_comp_bits_candi[1]);
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[mem_fetch::REMAPPED_DATA_2], min_lwm_comp_res_candi[2], min_lwm_comp_bits_candi[2]);
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[mem_fetch::REMAPPED_DATA_3], min_lwm_comp_res_candi[3], min_lwm_comp_bits_candi[3]);
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[mem_fetch::REMAPPED_DATA_4], min_lwm_comp_res_candi[4], min_lwm_comp_bits_candi[4]);
		*/

		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[0], min_lwm_comp_res_candi[0], min_lwm_comp_bits_candi[0]);
		for(int k = 1; k < data_type_no; k++) {
		  g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[k], min_lwm_comp_res_candi[k], min_lwm_comp_bits_candi[k]);
		}


		unsigned min_comp_bits_index = 0;
		min_lwm_comp_bits = min_lwm_comp_bits_candi[0];
		min_lwm_comp_res = min_lwm_comp_res_candi[0];
		//min_data_type = mem_fetch::ORG_DATA;
		for(int i = 1; i < min_lwm_comp_bits_candi.size(); i++) {
		  if(min_lwm_comp_bits > min_lwm_comp_bits_candi[i]) {
		    min_lwm_comp_bits = min_lwm_comp_bits_candi[i];
		    min_lwm_comp_res = min_lwm_comp_res_candi[i];
		    min_comp_bits_index = i;
		    /*
		    if(i == 1)		min_data_type = mem_fetch::REMAPPED_DATA_1;
		    else if(i == 2)	min_data_type = mem_fetch::REMAPPED_DATA_2;
		    else if(i == 3)	min_data_type = mem_fetch::REMAPPED_DATA_3;
		    else if(i == 4)	min_data_type = mem_fetch::REMAPPED_DATA_4;
		    */
		  }
		}

		if(min_comp_bits_index == 0)	min_data_type = mem_fetch::ORG_DATA;
		else 				min_data_type = ret_mf[0]->get_remapped_data_type(min_comp_bits_index-1);

	      } else {

		//#ifdef old //need to change
		if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::ORG_DATA)
		  min_data_type = mem_fetch::ORG_DATA;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_1)
		  min_data_type = mem_fetch::REMAPPED_DATA_1;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_2)
		  min_data_type = mem_fetch::REMAPPED_DATA_2;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_3)
		  min_data_type = mem_fetch::REMAPPED_DATA_3;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_4)
		  min_data_type = mem_fetch::REMAPPED_DATA_4;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_5)
		  min_data_type = mem_fetch::REMAPPED_DATA_5;
		else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_6)
		  min_data_type = mem_fetch::REMAPPED_DATA_6;
		else	assert(0);
		//#endif

		int min_comp_bits_index = -1;
		if(min_data_type == mem_fetch::ORG_DATA)	min_comp_bits_index = 0;
		else {
		  int remapped_data_index = ret_mf[0]->get_remapped_data_type_index(min_data_type);
		  assert(remapped_data_index>=0);
		  min_comp_bits_index = remapped_data_index+1;
		}
		g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[min_comp_bits_index], min_lwm_comp_res, min_lwm_comp_bits);
	      }

	      #ifdef old //need to change
	      if(min_data_type == mem_fetch::ORG_DATA) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE0_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, ORG_DATA_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_1) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE1_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA1_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_2) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE2_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA2_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_3) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE3_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA3_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_4) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE4_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_5) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE5_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else if(min_data_type == mem_fetch::REMAPPED_DATA_6) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE6_NO, 1);
		//DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	      } else assert(0);
	      #endif
	    }

	    assert(min_lwm_comp_bits > 0);
	    assert(min_dsm_comp_bits > 0);
	    if(min_dsm_comp_bits <= min_lwm_comp_bits) {
	      ret_mf[0]->set_comp_data_bits(min_dsm_comp_bits);
	      ret_mf[0]->set_comp_res(min_dsm_comp_res);
	      ret_mf[0]->set_comp_algo_type(mem_fetch::DSM_COMP);
	      ret_mf[0]->set_dsc_comp_data_type(min_data_type);

	      //since lwm is not used.

	    } else {
	      ret_mf[0]->set_comp_data_bits(min_lwm_comp_bits);
	      ret_mf[0]->set_comp_res(min_lwm_comp_res);
	      ret_mf[0]->set_comp_algo_type(mem_fetch::LWM_COMP);
	      ret_mf[0]->set_dsc_comp_data_type(min_data_type);

	      //since dsm is not used.
	      ret_mf[0]->set_dsc_comp_res(0);
	    }

	    if(ret_mf[0]->get_comp_data_bits() > ret_mf[0]->get_real_data_size()*8) {
	      ret_mf[0]->set_comp_data_bits(ret_mf[0]->get_real_data_size()*8+1);
	      ret_mf[0]->set_comp_algo_type(mem_fetch::NO_COMP);
	    }

	    #ifdef old //need to change
	    mem_fetch::COMP_ALGO_TYPE algo_type = ret_mf[0]->get_comp_algo_type();
	    unsigned min_comp_res = ret_mf[0]->get_comp_res();
	    double comp_data_ratio = (double)(ret_mf[0]->get_real_data_size()*8-ret_mf[0]->get_comp_data_bits())/(ret_mf[0]->get_real_data_size()*8);
	    if(algo_type == mem_fetch::DSM_COMP) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_NO, 1);
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_DATA_RATIO, comp_data_ratio);
	      //std::cout << "comp_data_ratio " << comp_data_ratio << std::endl;
	    } else if(algo_type == mem_fetch::LWM_COMP) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::LWM_COMP_NO, 1);
	      assert(min_comp_res > 0);
	      if(min_comp_res == 2)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	      else if(min_comp_res == 4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	      else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      else 				assert(0);
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::LWM_COMP_DATA_RATIO, comp_data_ratio);
	    } else if(algo_type == mem_fetch::NO_COMP) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_NO, 1);
	    }
	    #endif

	  }

	  if(ret_mf[0]) {
	    g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf[0]);
	  }



	  //Stage1: Data Remapping and Data Segment Compression and insert non-fully-compressed data to LWM
	  //Data Remapping
	  if(mf) {
	    g_hpcl_comp->remap_nonchar_data(mf);
	  }

	  g_hpcl_comp_dsc_pl[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  mem_fetch* out_mf = g_hpcl_comp_dsc_pl[i]->run();		//run compressor

	  if(out_mf) {
	    if(out_mf->has_uncompressed_ds() == true) {
	      DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has Uncomp DS)---\n", out_mf->get_request_uid());
	      bool is_all_ds_uncomp = true;
	      for(int m = 0; m < out_mf->get_comp_ds_status_size(); m++) {
		if(out_mf->get_comp_ds_status(m) == 1) {
		  is_all_ds_uncomp = false;
		  break;
		}
	      }
	      if(is_all_ds_uncomp == true) {
		DATA_SEG_COMP_DEBUG_PRINT("\tALL DS Uncompressed\n");
	      }
	    } else {
	      DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has No Uncomp DS)---\n", out_mf->get_request_uid());
	    }
	    for(int k = 0; k < out_mf->get_comp_ds_status_size(); k++) {
	      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d Status %u\n", k, out_mf->get_comp_ds_status(k));
	    }
	  }
	  /*
	  if(out_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(out_mff);
	  }
	  */

	  //Pass mf to LWM
	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	  /// end of Stage1


      }
      //added by khkim (071216)
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL)
      {
        //Stage1: Data Remapping and Data Segment Compression and insert non-fully-compressed data to LWM
	//Data Remapping
	if(mf) {
	  g_hpcl_comp->remap_nonchar_data(mf);
	  ///printf("%llu | mf %u | in\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid());
	}

	g_hpcl_comp_dsc_pl[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	mem_fetch* out_mf = g_hpcl_comp_dsc_pl[i]->run();	//run compressor
	if(out_mf) {
	  if(out_mf->has_uncompressed_ds() == true) {
	    DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has Uncomp DS)---\n", out_mf->get_request_uid());
	    bool is_all_ds_uncomp = true;
	    for(int m = 0; m < out_mf->get_comp_ds_status_size(); m++) {
	      if(out_mf->get_comp_ds_status(m) == 1) {
		is_all_ds_uncomp = false;
		break;
	      }
	    }
	    if(is_all_ds_uncomp == true) {
	      DATA_SEG_COMP_DEBUG_PRINT("\tALL DS Uncompressed\n");
	    }
	  } else {
	    DATA_SEG_COMP_DEBUG_PRINT("--- MF %u DS Status (Has No Uncomp DS)---\n", out_mf->get_request_uid());
	  }
	  for(int k = 0; k < out_mf->get_comp_ds_status_size(); k++) {
	    DATA_SEG_COMP_DEBUG_PRINT("\tDS%d Status %u\n", k, out_mf->get_comp_ds_status(k));
	  }

	  //printf("%llu | mf %u | out\n", (gpu_sim_cycle+gpu_tot_sim_cycle), out_mf->get_request_uid());
	}

	//Pass mf to LWM
	std::vector<unsigned> comp_bits;
	comp_bits.clear();
	mem_fetch* ret_mf = NULL;

	if(g_hpcl_comp_lwm_2B_en == true) {
	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  ret_mf = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	  if(ret_mf) comp_bits.push_back(ret_mf->get_comp_data_bits(2));
	}

	if(g_hpcl_comp_lwm_4B_en == true) {
	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  ret_mf = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	  if(ret_mf) comp_bits.push_back(ret_mf->get_comp_data_bits(4));
	}
	if(g_hpcl_comp_lwm_8B_en == true) {
	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  ret_mf = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	  if(ret_mf) comp_bits.push_back(ret_mf->get_comp_data_bits(8));
	}

	if(ret_mf) {

	  assert(out_mf == ret_mf);

	  unsigned min_dsm_comp_bits = ret_mf->get_comp_data_bits();
	  unsigned min_dsm_comp_res = ret_mf->get_dsc_comp_res();
	  enum mem_fetch::COMP_DATA_TYPE min_data_type = ret_mf->get_dsc_comp_data_type();

	  unsigned min_lwm_comp_bits = 0;
	  unsigned min_lwm_comp_res = 0;
	  g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits, min_lwm_comp_res, min_lwm_comp_bits);

	  assert(min_lwm_comp_bits > 0);
	  assert(min_dsm_comp_bits > 0);
	  if(min_dsm_comp_bits <= min_lwm_comp_bits) {
	    ret_mf->set_comp_data_bits(min_dsm_comp_bits);
	    ret_mf->set_comp_res(min_dsm_comp_res);
	    ret_mf->set_comp_algo_type(mem_fetch::DSM_COMP);
	    ret_mf->set_dsc_comp_data_type(min_data_type);

	    //since lwm is not used.

	    //added by kh(090116)
	    ret_mf->set_comp_ed_byte(ret_mf->get_dsc_comp_ed_byte());
	    ret_mf->set_comp_es_byte(ret_mf->get_dsc_comp_es_byte());
	    ///

	  } else {
	    ret_mf->set_comp_data_bits(min_lwm_comp_bits);
	    ret_mf->set_comp_res(min_lwm_comp_res);
	    ret_mf->set_comp_algo_type(mem_fetch::LWM_COMP);
	    ret_mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);

	    //min_data_type = mem_fetch::ORG_DATA;

	    //since dsm is not used.
	    ret_mf->set_dsc_comp_res(0);

	    //added by kh(090116)
	    ret_mf->set_comp_ed_byte(ret_mf->get_lwm_comp_ed_byte(min_lwm_comp_res));
	    ret_mf->set_comp_es_byte(ret_mf->get_lwm_comp_es_byte(min_lwm_comp_res));
	    ///

	  }

	  if(ret_mf->get_comp_data_bits() > ret_mf->get_real_data_size()*8) {
	    ret_mf->set_comp_data_bits(ret_mf->get_real_data_size()*8+1);
	    ret_mf->set_comp_algo_type(mem_fetch::NO_COMP);

	    //added by kh(090116)
	    ret_mf->set_comp_ed_byte(ret_mf->get_real_data_size()*8);
	    ret_mf->set_comp_es_byte(1);
	    ///
	  }

	  #ifdef DATA_REMAP_DEBUG
	  printf("----- mf %u, sm %u, wid %u, pc %u type %u starts ---- \n", ret_mf->get_request_uid(), ret_mf->get_tpc(), ret_mf->get_wid(), ret_mf->get_pc(), ret_mf->get_access_type());
	  ret_mf->print_data_all(8);
	  printf("Final: Select ");
	  ret_mf->print_dsc_comp_data_type(ret_mf->get_dsc_comp_data_type());
	  if(ret_mf->get_comp_algo_type() == mem_fetch::DSM_COMP) {
	    printf("DSM_COMP\n");
	  } else if(ret_mf->get_comp_algo_type() == mem_fetch::LWM_COMP) {
	    printf("LWM_COMP\n");
	  } else {
	    printf("NO_COMP\n");
	  }
	  printf("min_comp_data_bits %u, org_data_bits %u\n", ret_mf->get_comp_data_bits(), ret_mf->get_real_data_size()*8);
	  printf("ed_byte %u es_byte %u\n", ret_mf->get_comp_ed_byte(), ret_mf->get_comp_es_byte());
	  printf("----------------------- \n\n");
	  #endif
	}








	if(ret_mf) {
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	}








	#ifdef old
	g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor
	g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(out_mf);	//send mem_fetch to compressor

	//Stage2: LWM Compression and push result to comp_buffer
	unsigned data_type_no = 1; //g_hpcl_comp_config.hpcl_data_remap_function.size()+1;
	unsigned res_no = 3;

	std::vector<mem_fetch*> ret_mf(res_no, NULL);
	std::vector<std::vector<unsigned> > comp_bits;	//comp_bits[data_type][resolution_index]
	comp_bits.resize(data_type_no);

	for(int k = 0; k < data_type_no; k++)	comp_bits[k].resize(res_no,0);

	ret_mf[0] = g_hpcl_comp_lwm_pl_2B[i]->run();			//run compressor
	if(ret_mf[0]) {
	  comp_bits[0][0] = ret_mf[0]->get_comp_data_bits(2);
	}

	ret_mf[1] = g_hpcl_comp_lwm_pl_4B[i]->run();			//run compressor
	if(ret_mf[1]) {
	  comp_bits[0][1] = ret_mf[1]->get_comp_data_bits(4);
	}

	ret_mf[2] = g_hpcl_comp_lwm_pl_8B[i]->run();			//run compressor
	if(ret_mf[2]) {
	  comp_bits[0][2] = ret_mf[2]->get_comp_data_bits(8);
	}

	if(ret_mf[0]) {
	  unsigned min_dsm_comp_bits = ret_mf[0]->get_comp_data_bits();
	  unsigned min_dsm_comp_res = ret_mf[0]->get_dsc_comp_res();
	  enum mem_fetch::COMP_DATA_TYPE min_data_type = ret_mf[0]->get_dsc_comp_data_type();

	  unsigned min_lwm_comp_bits = 0;
	  unsigned min_lwm_comp_res = 0;
	  g_hpcl_comp_lwm_aux->select_best_compressor(comp_bits[0], min_lwm_comp_res, min_lwm_comp_bits);

	  assert(min_lwm_comp_bits > 0);
	  assert(min_dsm_comp_bits > 0);
	  if(min_dsm_comp_bits <= min_lwm_comp_bits) {
	    ret_mf[0]->set_comp_data_bits(min_dsm_comp_bits);
	    ret_mf[0]->set_comp_res(min_dsm_comp_res);
	    ret_mf[0]->set_comp_algo_type(mem_fetch::DSM_COMP);
	    ret_mf[0]->set_dsc_comp_data_type(min_data_type);

	    //since lwm is not used.

	  } else {
	    ret_mf[0]->set_comp_data_bits(min_lwm_comp_bits);
	    ret_mf[0]->set_comp_res(min_lwm_comp_res);
	    ret_mf[0]->set_comp_algo_type(mem_fetch::LWM_COMP);
	    ret_mf[0]->set_dsc_comp_data_type(mem_fetch::ORG_DATA);

	    //min_data_type = mem_fetch::ORG_DATA;

	    //since dsm is not used.
	    ret_mf[0]->set_dsc_comp_res(0);

	  }

	  if(ret_mf[0]->get_comp_data_bits() > ret_mf[0]->get_real_data_size()*8) {
	    ret_mf[0]->set_comp_data_bits(ret_mf[0]->get_real_data_size()*8+1);
	    ret_mf[0]->set_comp_algo_type(mem_fetch::NO_COMP);
	  }

	  #ifdef DATA_REMAP_DEBUG
	  printf("----- mf %u starts ---- \n", ret_mf[0]->get_request_uid());
	  ret_mf[0]->print_data_all(8);
	  printf("Final: Select ");
	  ret_mf[0]->print_dsc_comp_data_type(ret_mf[0]->get_dsc_comp_data_type());
	  if(ret_mf[0]->get_comp_algo_type() == mem_fetch::DSM_COMP) {
	    printf("DSM_COMP\n");
	  } else if(ret_mf[0]->get_comp_algo_type() == mem_fetch::LWM_COMP) {
	    printf("LWM_COMP\n");
	  } else {
	    printf("NO_COMP\n");
	  }
	  printf("min_comp_data_bits %u, org_data_bits %u\n", ret_mf[0]->get_comp_data_bits(), ret_mf[0]->get_real_data_size()*8);
	  printf("----------------------- \n\n");
	  #endif
	}

	if(ret_mf[0]) {
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf[0]);
	}
	#endif





      }

      //added by abpd (042816)
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::CPACK_WORD_MATCHING)
      {
	g_hpcl_comp_cpack_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	
	long time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
	if(mf)
		cout<<"PD:: THe time stamp is " <<time_sta<<"id "<<mf->get_request_uid()<<endl;
	mem_fetch* ret_mf = g_hpcl_comp_cpack_pl_4B[i]->run();
	//mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);
	
	if(ret_mf) {
		time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
		cout<<"PD:: Return time stamp is " <<time_sta<<"id "<<ret_mf->get_request_uid()<<endl;
			//cpack control msg ----------------------------------------
	  vector<string> update_list = g_hpcl_comp_cpack_pl_4B[i]->get_update_list();
	  /*
	  if(update_list.size()>0) {
			  
	      //cout<<"Abhishek update "<<update_list.size()<<endl;
	      //assert(0);
	      //#ifdef PAD_CTRL_WORDS

	      // create a new mf for ctrl msg
		mem_fetch *ctrl_mf = new mem_fetch(CTRL_MSG,ret_mf->get_tpc(),update_list.size(),ret_mf->get_wid(),ret_mf->get_sid());
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ctrl_mf);
	  }*/
			//-----------------------------------------------------------------------
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	}
      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::BDI_WORD_MATCHING)
      {
	    //std::cout << "g_hpcl_comp_bdi_pl_4B[i]->get_input() " << g_hpcl_comp_bdi_pl_4B[i]->get_input() << std::endl;
	    g_hpcl_comp_bdi_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
		
		long time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
		if(mf)
			cout<<"PD:: THe time stamp is " <<time_sta<<"id "<<mf->get_request_uid()<<endl;

	      mem_fetch* ret_mf = g_hpcl_comp_bdi_pl_4B[i]->run();
	      if(ret_mf) {
			time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
			cout<<"PD:: Return time stamp is " <<time_sta<<"id "<<ret_mf->get_request_uid()<<endl;

		  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
		  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      }
      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::FPC_WORD_MATCHING)
      {
	      g_hpcl_comp_fpc_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
		  long time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
		if(mf)
			cout<<"PD:: THe time stamp is " <<time_sta<<"id "<<mf->get_request_uid()<<endl;

	      mem_fetch* ret_mf = g_hpcl_comp_fpc_pl_4B[i]->run();
	      if(ret_mf) {
	      		  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
				  time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
				cout<<"PD:: Return time stamp is " <<time_sta<<"id "<<ret_mf->get_request_uid()<<endl;
	      		  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      	      }
      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::ABPD_LOCAL_WORD_MATCHING)
      {
	      g_hpcl_comp_abpd_local_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	      mem_fetch* ret_mf = g_hpcl_comp_abpd_local_pl_4B[i]->run();
	      if(ret_mf) {
	      		  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      		  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      	      }

      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::LOCAL_WORD_MATCHING2) {

	if(g_hpcl_comp_config.hpcl_comp_word_size == 2) {
	  g_hpcl_comp_lwm2_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  mem_fetch* ret_mf = g_hpcl_comp_lwm2_pl_2B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	      ret_mf->set_comp_res(2);
	  }
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) {
	  g_hpcl_comp_lwm2_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  mem_fetch* ret_mf = g_hpcl_comp_lwm2_pl_4B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	      ret_mf->set_comp_res(4);
	  }
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) {
	  g_hpcl_comp_lwm2_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  mem_fetch* ret_mf = g_hpcl_comp_lwm2_pl_8B[i]->run();		//run compressor
	  if(ret_mf) {
	      g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      ret_mf->set_comp_res(8);
	  }
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {		//hybrid compressor for 2/4/8

	  std::vector<mem_fetch*> ret_mf(3, NULL);
	  std::vector<unsigned> comp_size(3,0);

	  if(mf) {
	    g_hpcl_comp_lwm2_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[0] = g_hpcl_comp_lwm2_pl_2B[i]->run();			//run compressor
	    comp_size[0] = ret_mf[0]->get_comp_data_size();

	    g_hpcl_comp_lwm2_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[1] = g_hpcl_comp_lwm2_pl_4B[i]->run();			//run compressor
	    comp_size[1] = ret_mf[1]->get_comp_data_size();

	    g_hpcl_comp_lwm2_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	    ret_mf[2] = g_hpcl_comp_lwm2_pl_8B[i]->run();			//run compressor
	    comp_size[2] = ret_mf[2]->get_comp_data_size();

	    //std::cout << "mf " << mf->get_request_uid();
	    //std::cout << " Comp Perf " << comp_size[0] << " " << comp_size[1] << " " << comp_size[2];
	    //std::cout << std::endl;

	    //Choose the best compression result
	    unsigned min_comp_size = comp_size[0];
	    unsigned min_comp_res = 2;
	    for(int i = 1; i < comp_size.size(); i++) {
	      if(comp_size[i] < min_comp_size) {
		min_comp_size = comp_size[i];
		if(i == 1)		min_comp_res = 4;
		else if(i == 2)	min_comp_res = 8;
		else		assert(0);
	      }
	    }

	    if(min_comp_res == 2)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
	    else if(min_comp_res == 4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
	    else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	    else assert(0);

	    //assert(mf == ret_mf[0]);
	    //assert(ret_mf[0] == ret_mf[1]);
	    //assert(ret_mf[1] == ret_mf[2]);
	    mf->set_comp_data_size(min_comp_size);
	    mf->set_comp_res(min_comp_res);
	    g_hpcl_comp_buffer[i]->push_mem_fetch(mf);

	  }

	}
      }
      //added by abpd (061916)
//      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::SC2_WORD_MATCHING)
//      {
//	//std::cout << "g_hpcl_comp_bdi_pl_4B[i]->get_input() " << g_hpcl_comp_bdi_pl_4B[i]->get_input() << std::endl;
//	g_hpcl_comp_sc2_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
//	mem_fetch* ret_mf = g_hpcl_comp_sc2_pl_4B[i]->run(g_hpcl_comp_config.hpcl_sc2_mode,g_hpcl_comp_config.hpcl_sc2_iter);
//	if(ret_mf) {
//	    g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
//	    //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
//	}
//      }
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::SC2_WORD_MATCHING)
      {
	    g_hpcl_comp_sc2_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
//mem_fetch* ret_mf = g_hpcl_comp_sc2_pl_4B[i]->run(g_hpcl_comp_config.hpcl_sc2_mode);
//added in May
		mem_fetch* ret_mf = g_hpcl_comp_sc2_pl_4B[i]->run(gpu_tot_sim_insn+gpu_sim_insn,g_hpcl_comp_config.hpcl_sc2_iter_sam,g_hpcl_comp_config.hpcl_sc2_iter_enc);
	      if(ret_mf) {
		  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
		  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	      }
      }
//      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::FPH_WORD_MATCHING)
//      {
//	//std::cout << "g_hpcl_comp_bdi_pl_4B[i]->get_input() " << g_hpcl_comp_bdi_pl_4B[i]->get_input() << std::endl;
//	g_hpcl_comp_fph_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
//	mem_fetch* ret_mf = g_hpcl_comp_fph_pl_4B[i]->run(g_hpcl_comp_config.hpcl_fph_mode,g_hpcl_comp_config.hpcl_fph_iter);
//	if(ret_mf) {
//	    g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
//	    //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
//	}
//      }
//      ///
      //added by abpd(071816)
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::FPH_WORD_MATCHING)
      {
	g_hpcl_comp_fph_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
//	mem_fetch* ret_mf = g_hpcl_comp_fph_pl_4B[i]->run(g_hpcl_comp_config.hpcl_fph_mode,gpu_tot_sim_insn);
//added in May
	mem_fetch* ret_mf = g_hpcl_comp_fph_pl_4B[i]->run(gpu_tot_sim_insn+gpu_sim_insn,g_hpcl_comp_config.hpcl_fph_iter_sam,g_hpcl_comp_config.hpcl_fph_iter_enc);
	if(ret_mf) {
	  mem_fetch *ctrl_mf = new mem_fetch(CTRL_MSG,ret_mf->get_tpc(),10 /* JUST DUMMY SIZE */,ret_mf->get_wid(),ret_mf->get_sid());
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ctrl_mf);
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	}
      }
      //added by abpd(071616)
      else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::BPC_WORD_MATCHING)
      {
	//std::cout << "g_hpcl_comp_bdi_pl_4B[i]->get_input() " << g_hpcl_comp_bdi_pl_4B[i]->get_input() << std::endl;
	g_hpcl_comp_bpc_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	long time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;

	if(mf)
			cout<<"PD:: THe time stamp is " <<time_sta<<"id "<<mf->get_request_uid()<<endl;

	mem_fetch* ret_mf = g_hpcl_comp_bpc_pl_4B[i]->run();
	if(ret_mf) {
	  g_hpcl_comp_buffer[i]->push_mem_fetch(ret_mf);
	  time_sta=gpu_sim_cycle+gpu_tot_sim_cycle;
		cout<<"PD:: Return time stamp is " <<time_sta<<"id "<<ret_mf->get_request_uid()<<endl;
	  //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
	}
      }


      else assert(0);

#ifdef old
      //added by kh(050516)
      if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::LOCAL_WORD_MATCHING)
      {
	//added by kh(042116)
	//get stat of read replies to the same SM in the comp_buffer
	if(g_hpcl_comp_buffer[i]->has_pending_mem_fetch()) {
	  double single_rep_to_same_sm_rate = 0;
	  double multi_rep_to_same_sm_rate = 0;
	  double no_rep_to_same_sm_rate = 0;
	  double avg_multi_rep_no = 0;
	  g_hpcl_comp_buffer[i]->get_reply_rate_dist(single_rep_to_same_sm_rate, multi_rep_to_same_sm_rate, no_rep_to_same_sm_rate, avg_multi_rep_no);

	  if(single_rep_to_same_sm_rate == 1) {
	    //Do nothing, write replies are in the comp buffer
	  } else {
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::SINGLE_REP_TO_SAME_SM_RATE, single_rep_to_same_sm_rate, i);
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MULTI_REP_TO_SAME_SM_RATE, multi_rep_to_same_sm_rate, i);
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_REP_TO_SAME_SM_RATE, no_rep_to_same_sm_rate, i);

	    //deleted by kh(050216)
	    //g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MULTI_REP_TO_SAME_SM_NO, avg_multi_rep_no);
	    //added by kh(050216)
	    if(multi_rep_to_same_sm_rate > 0) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MULTI_REP_TO_SAME_SM_NO, avg_multi_rep_no);
//	      std::cout << "single_rep_to_same_sm_rate " << single_rep_to_same_sm_rate;
//	      std::cout << " multi_rep_to_same_sm_rate " << multi_rep_to_same_sm_rate;
//	      std::cout << " no_rep_to_same_sm_rate " << no_rep_to_same_sm_rate;
//	      std::cout << " avg_multi_rep_no " << avg_multi_rep_no;
//	      std::cout << std::endl;
	    }
	  }
	}
	///
      }
#endif



      m_memory_sub_partition[i]->pop();
    }
  }
}

void gpgpu_sim::push_mf_from_mc_to_icnt_in_crossbar()
{
  // push from memory controller to interconnect
  for (unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++) {
      mem_fetch* mf = m_memory_sub_partition[i]->top();
      if (mf) {
          unsigned response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
          if ( ::icnt_has_buffer( m_shader_config->mem2device(i), response_size ) ) {
              if (!mf->get_is_write())
                 mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);
              mf->set_status(IN_ICNT_TO_SHADER,gpu_sim_cycle+gpu_tot_sim_cycle);

              //commented-out by kh(10/30/14), Case3: MC-->ICNT
              //::icnt_push( m_shader_config->mem2device(i), mf->get_tpc(), mf, response_size );
              //added by kh(10/30/14)
              ::perf_icnt_push( m_shader_config->mem2device(i), mf->get_tpc(), mf, response_size, perf_packet_type::MC_TO_ICNT,gpu_sim_cycle,gpu_sim_cycle+gpu_tot_sim_cycle);
              ///

              //added by kh(070715)
              //See MCs actually used.
              g_MCs_in_use.insert(mf->m_MC_id);
              ///

              //added by kh(030217)
              //measure character data types when no compression is supported
              if(g_hpcl_comp->is_data_retrievable(mf) == true) {
				double char_type_pct = g_hpcl_comp->get_char_type_pct(mf);
				if(char_type_pct >= g_hpcl_comp_config.hpcl_char_type_pct_th) {
					g_hpcl_data_anal->analyze_data_type(mf->get_real_data_ptr(), true, char_type_pct);
				} else {
					g_hpcl_data_anal->analyze_data_type(mf->get_real_data_ptr(), false, char_type_pct);
				}
          	  }
              ///

              //added by kh(021617)
              //if(mf->get_is_write() == false && (mf->get_access_type() == GLOBAL_ACC_R || mf->get_access_type() == TEXTURE_ACC_R)) {
			      #ifdef old
				  assert(g_hpcl_data_reader);
				  unsigned char* real_data = mf->config_real_data(mf->get_data_size());
				  g_hpcl_data_reader->get_cache_data(mf, real_data);

				    /*
					printf("Data(%03u) = ", mf->get_real_data_size());
					//printf("Data(%03u) = ", mf->get_real_data_size());
					for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
						printf("%02x", mf->get_real_data(i));
					}
					printf("\n");
				    */

					//check data type (char or not) and redundancy source for int type
					int ret = g_hpcl_data_anal->analyze_data_type(mf->get_real_data_ptr());

					if(ret == 0) {		//char data


					  //Testing data remapping
					  #define REMAP_EXTRABIT 1
					  std::vector<unsigned char> txt_trans_data;
					  std::vector<unsigned char> lownibble_clus, highnibble_clus, extranibble_clus;
					  unsigned char lsb = 0;
					  for(unsigned i = 0; i < mf->get_real_data_size(); i = i+2) {
						unsigned char byte1 = mf->get_real_data(i);
						unsigned char byte2 = mf->get_real_data(i+1);

						unsigned char low_nibble = (byte1 & 0x0f);
						low_nibble = (low_nibble | ((byte2 & 0x0f) << 4));

						#ifdef REMAP_EXTRABIT
						unsigned char high_nibble = (byte1 & 0xf0) >> 4;
						high_nibble = high_nibble >> 1;
						high_nibble = (high_nibble | (((byte2 & 0xf0) >> 1) & 0xf0) );

						#else
						unsigned char high_nibble = (byte1 & 0xf0) >> 4;
						high_nibble = (high_nibble | (byte2 & 0xf0));
						#endif

						lownibble_clus.push_back(low_nibble);
						highnibble_clus.push_back(high_nibble);

						#ifdef REMAP_EXTRABIT
						if(i % 8 == 0) {
							lsb = 0;
						}

						unsigned char high_nibble1 = (byte1 & 0xf0) >> 4;
						unsigned char high_nibble2 = (byte2 & 0xf0) >> 4;
						int bitpos1 = i % 8;
						int bitpos2 = (i+1) % 8;
						//unsigned char adjust_bit1 = ((high_nibble1 & 0x01) << bitpos1);
						//unsigned char adjust_bit2 = ((high_nibble2 & 0x01) << bitpos2);
						//printf("high_nibble1 = 0x%02x, bitpos1 = %d, adjust_bit1 = 0x%02x\n", high_nibble1, bitpos1, adjust_bit1);
						//printf("high_nibble2 = 0x%02x, bitpos2 = %d, adjust_bit2 = 0x%02x\n", high_nibble2, bitpos2, adjust_bit2);
						lsb = lsb | ((high_nibble1 & 0x01) << bitpos1);
						lsb = lsb | ((high_nibble2 & 0x01) << bitpos2);
						//printf("lsb = 0x%02x\n", lsb);
					    /*
						//if(((i+1) % 8 == 7) && ((i+1) <= mf->get_real_data_size()/2)) {

						if(((i+1) % 8 == 7)) {
							extranibble_clus.push_back(lsb);
							printf("extra_byte = %02x\n", lsb);
						}
						*/
						#endif

					  }
					  txt_trans_data = lownibble_clus;
					  #ifdef REMAP_EXTRABIT
					  txt_trans_data.insert(txt_trans_data.end(), extranibble_clus.begin(), extranibble_clus.end());
					  #endif
					  txt_trans_data.insert(txt_trans_data.end(), highnibble_clus.begin(), highnibble_clus.end());



						static int counter = 0;
						printf("[%d] Char Data(%03u) = ", counter, mf->get_real_data_size());
						for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
							printf("%02x", mf->get_real_data(i));
						}
						printf("\n");

						printf("[%d] RemappedChar Data(%03u) = ", counter, txt_trans_data.size());
						for(int i = txt_trans_data.size()-1; i >= 0 ; i--) {
							printf("%02x", txt_trans_data[i]);
						}
						printf("\n");


						counter++;
						if(counter >= 100)	exit (EXIT_SUCCESS);


						//*/



						/*
						static int counter = 0;
						printf("[%d] Char Data(%03u) = ", counter, mf->get_real_data_size());
						for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
							printf("%02x", mf->get_real_data(i));
						}
						printf("\n");
						counter++;
						if(counter >= 100)	exit (EXIT_SUCCESS);
						*/



					}
					else if(ret == 1) {	//non-char data
						//do nothing
					}
					else if(ret < 0) {	//no-pattern non-char data

						/*
						printf("NO-PATTERN Data(%03u) = ", mf->get_real_data_size());
						//printf("Data(%03u) = ", mf->get_real_data_size());
						for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
							printf("%02x", mf->get_real_data(i));
						}
						printf("\n");
						*/
					}

				#endif
			  //}
              ///

              m_memory_sub_partition[i]->pop();
          } else {
              gpu_stall_icnt2sh++;

	      //added by kh(061616)
	      //g_hpcl_coalescing_anal->add_sample(hpcl_coalescing_anal::STALL_ICNT2SH, 1, i);
	      ///


          }
      } else {
         m_memory_sub_partition[i]->pop();
      }
  }
}


int gpgpu_sim::push_mf_to_icnt_in_crossbar(unsigned input_node, mem_fetch* mf)
{
  int ret = -1;

  if(mf) {

    //added by kh(062316)
    //set the packet size based on the compression result.
    int response_size = g_hpcl_comp_lwm_aux->compute_final_packet_size(mf);
    //if(response_size <= 0) {
    if(mf->get_is_write() == true)
      COMP_DEBUG_PRINT("%llu | WRITE | mf %u has response_size %d\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), response_size);
    else
      COMP_DEBUG_PRINT("%llu |  READ | mf %u has response_size %d\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), response_size);
    //}
    assert(response_size > 0);

    if ( ::icnt_has_buffer( input_node, response_size ) ) {
	if (!mf->get_is_write())
	   mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);

	mf->set_status(IN_ICNT_TO_SHADER,gpu_sim_cycle+gpu_tot_sim_cycle);

	// Case3: MC-->ICNT
	::perf_icnt_push( input_node, mf->get_tpc(), mf, response_size, perf_packet_type::MC_TO_ICNT,gpu_sim_cycle,gpu_sim_cycle+gpu_tot_sim_cycle);
	///

	//added by kh(070715)
	//See MCs actually used.
	g_MCs_in_use.insert(mf->m_MC_id);
	///

	mf->set_final_packet_size((int)response_size);

	//g_hpcl_comp_lwm_aux->get_stat_for_comp_data_ratio_segment(mf, mf->get_comp_res());
	g_hpcl_comp_lwm_aux->get_stat_for_compression_perf(mf);
	g_hpcl_comp_lwm_aux->delete_local_dict_info(mf);

	ret = 0;

    } else {
	gpu_stall_icnt2sh++;

	//deleted by kh(062516)
	//if mf is not sent, clear the merged mfs
	//if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_rec_comp_en == 1) {
	//  g_hpcl_comp_lwm_aux->clean_rec_comp_info(mf);
	//}
	///

	//DEBUG_PRINT("buffer has no space for mf %u!\n", mf->get_request_uid());
	//DEBUG_PRINT("buffer has space %u\n", ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i)));
	//printf("buffer has no space for mf %u!\n", mf->get_request_uid());
	//printf("buffer has space %u\n", ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i)));
	//std::cout << "stall!!! response_size " << response_size << std::endl;
    }

  }

  return ret;

}


void gpgpu_sim::pass_mf_from_rec_comp_buffer_to_icnt_in_crossbar()
{
  for(unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++)
  {
      mem_fetch* mf = NULL;
      //according to scheduling policy, we select next mem_fetch for recursive compression
      mf = g_hpcl_rec_comp_buffer[i]->get_next_mem_fetch();
      if(mf) {
	unsigned threshold = 1;
	//If there is no data in the NI buffer, send it to ICNT
	unsigned NI_buffer_occupancy = ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i));
	if(NI_buffer_occupancy <= threshold) {
	  int status = push_mf_to_icnt_in_crossbar(m_shader_config->mem2device(i), mf);
	  if(status == 0) {
	    REC_COMP_DEBUG_PRINT("%llu | Stage3 | mf %u is sent to ICNT\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid());
	    g_hpcl_rec_comp_buffer[i]->del_mem_fetch(mf);

	    //added by kh(062816)
	    /*
	    unsigned merged_mf_no = mf->get_merged_mf_no();
	    merged_mf_no += mf->get_merged_mf_to_same_SM_no();
	    if(merged_mf_no > 0) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_READ_REP_GROUP_SIZE, merged_mf_no);
	    }
	    ///
	    */

	    //added by kh(063016)
	    if(mf->get_merged_mf_no() > 0) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_INTER_SM_READ_REP_GROUP_SIZE, mf->get_merged_mf_no()+1);
	    } else if(mf->get_merged_mf_to_same_SM_no() > 0) {
	      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_INTRA_SM_READ_REP_GROUP_SIZE, mf->get_merged_mf_to_same_SM_no()+1);
	    }
	    ///


	  } else {
	      REC_COMP_DEBUG_PRINT("%llu | Stage3 | mf %u cannot be sent to ICNT\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid());
	  }
	}
      }
  }
}

void gpgpu_sim::pass_mf_from_comp_buffer_to_rec_comp_in_crossbar()
{
  for(unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++)
  {
    mem_fetch* mf = NULL;
    //according to scheduling policy, we select next mem_fetch for recursive compression
    mf = g_hpcl_comp_buffer[i]->get_next_mem_fetch();

    if(mf) {
      bool is_mf_consumed = false;
      unsigned threshold = 1;
      //If there is no data in the NI buffer, send it to ICNT
      #ifdef old
      unsigned NI_buffer_occupancy = ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i));
      if(NI_buffer_occupancy <= threshold) {
	int status = push_mf_to_icnt_in_crossbar(m_shader_config->mem2device(i), mf);
	if(status == 0) {
	  is_mf_consumed = true;
	  REC_COMP_DEBUG_PRINT("%llu | Stage2 | mf %u is sent to ICNT (Occu: %u)\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), NI_buffer_occupancy);
	} else {
	    REC_COMP_DEBUG_PRINT("%llu | Stage2 | mf %u cannot be sent to ICNT (Occu: %u)\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), NI_buffer_occupancy);
	}
      } else {	//Otherwise, send data to recursive compressor
      #endif
	if(g_hpcl_rec_comp_buffer[i]->has_comp_buffer_space() == true) {
	  g_hpcl_rec_comp_lwm_pl[i]->push_input(mf);
	  is_mf_consumed = true;
	}
      #ifdef old
      }
      #endif

      //delete mem_fetch from comp buffer
      if(is_mf_consumed == true) {
	g_hpcl_comp_buffer[i]->del_mem_fetch(mf);

	//deleted by kh(062316)
//	if(mf->has_merged_mem_fetch() == true)	{
//	  std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
//	  g_hpcl_comp_buffer[i]->delete_mfs (merged_mfs);
//	}

      }
    }

    //regardless of mf's value, this should run every cycle
    g_hpcl_rec_comp_lwm_pl[i]->run();
    //mf = g_hpcl_rec_comp_lwm_pl[i]->pop_output();
    //if(mf) {
    //  g_hpcl_rec_comp_buffer[i]->push_mem_fetch(mf);
    //  REC_COMP_DEBUG_PRINT("%llu | Stage2 | mf %u is sent to rec_comp_buffer\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid());
    //}

  }
}



void gpgpu_sim::pass_mf_from_comp_buffer_to_icnt_in_crossbar()
{
  for(unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++)
  {
      mem_fetch* mf = NULL;

      if(g_hpcl_comp_config.hpcl_comp_en == 0) {
	mf = m_memory_sub_partition[i]->top();
      } else {
	//according to scheduling policy, we select next mem_fetch for recursive compression
	mf = g_hpcl_comp_buffer[i]->get_next_mem_fetch();
	///
      }

      if (mf) {
	  unsigned response_size = 0;
#ifdef old
	  if(g_hpcl_comp_config.hpcl_comp_en == 0) {

	    response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
	    unsigned org_response_size = response_size;

	    //added by kh(041216)
	    //to implement an ideal compression methods based on hpcl_comp_ideal_comp_ratio
	    if(mf->get_is_write() == false) {
	      unsigned comp_data_size = (unsigned)ceil((double)mf->get_data_size() * (100 - g_hpcl_comp_config.hpcl_comp_ideal_comp_ratio) / 100);
	      response_size = (comp_data_size + mf->get_ctrl_size());
	      //assert(org_response_size > response_size);
	      //std::cout << "org_response_size : " << org_response_size << " response_size : " << response_size << std::endl;
	    }
	    ///

	    //added by kh(042916)
	    //for debugging
	    if(mf->get_is_write() == false) {

	      //added by kh(070716)
	      if(g_hpcl_comp->is_data_retrievable(mf) == true) {
		unsigned char* real_data = mf->config_real_data(mf->get_data_size());
		g_hpcl_comp->get_cache_data(mf, real_data);
		/*
		printf("Data(%03u) = ", mf->get_real_data_size());
		for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
		  printf("%02x ", mf->get_real_data(j));
		}
		printf("\n");
		*/
	      }
	    }
	    ///

	  } else {
#endif
	      //added by kh(062316)
	      response_size = g_hpcl_comp_lwm_aux->compute_final_packet_size(mf);
	      ///
#ifdef old
	  }
#endif
	  //std::cout << "mf " << mf->get_request_uid() << " response_size " << response_size << std::endl;
	  if ( ::icnt_has_buffer( m_shader_config->mem2device(i), response_size ) ) {
	      if (!mf->get_is_write())
		 mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);

	      mf->set_status(IN_ICNT_TO_SHADER,gpu_sim_cycle+gpu_tot_sim_cycle);


	      //commented-out by kh(10/30/14), Case3: MC-->ICNT
	      //::icnt_push( m_shader_config->mem2device(i), mf->get_tpc(), mf, response_size );
	      //added by kh(10/30/14)
	      ::perf_icnt_push( m_shader_config->mem2device(i), mf->get_tpc(), mf, response_size, perf_packet_type::MC_TO_ICNT,gpu_sim_cycle,gpu_sim_cycle+gpu_tot_sim_cycle);
	      ///

	      //added by kh(070715)
	      //See MCs actually used.
	      g_MCs_in_use.insert(mf->m_MC_id);
	      ///

#ifdef old
	      //added by kh(041616)
	      if(g_hpcl_comp_config.hpcl_comp_en == 0) {
		m_memory_sub_partition[i]->pop();
	      } else {
#endif
		g_hpcl_comp_buffer[i]->del_mem_fetch(mf);
#ifdef old
	      }
#endif
	      ///

	      mf->set_final_packet_size((int)response_size);

	      g_hpcl_comp_lwm_aux->get_stat_for_compression_perf(mf);
	      g_hpcl_comp_lwm_aux->delete_local_dict_info(mf);


	  } else {
	      gpu_stall_icnt2sh++;

	      //if mf is not sent, clear the merged mfs
	      if(g_hpcl_comp_config.hpcl_comp_en == 1) {
		g_hpcl_comp_lwm_aux->clean_rec_comp_info(mf);
	      }
	      ///

	      DEBUG_PRINT("buffer has no space for mf %u!\n", mf->get_request_uid());
	      DEBUG_PRINT("buffer has space %u\n", ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i)));
	      //printf("buffer has no space for mf %u!\n", mf->get_request_uid());
	      //printf("buffer has space %u\n", ::icnt_get_buffer_occupancy(m_shader_config->mem2device(i)));
	      //std::cout << "stall!!! response_size " << response_size << std::endl;
	  }
      } else {
#ifdef old
	if(g_hpcl_comp_config.hpcl_comp_en == 0) {
	    m_memory_sub_partition[i]->pop();
	}
#endif
      }
  }


}

void gpgpu_sim::push_mf_from_mc_to_icnt_in_crossbar_with_reply_compression()
{
  assert(g_hpcl_comp_config.hpcl_comp_algo >= hpcl_comp_config::LOCAL_WORD_MATCHING
	 && g_hpcl_comp_config.hpcl_comp_algo <= hpcl_comp_config::END_OF_COMP_ALGO);

  if(g_hpcl_comp_config.hpcl_rec_comp_en == 1) {

    //Stage3: Move rec_comp_buffer to ICNT
    pass_mf_from_rec_comp_buffer_to_icnt_in_crossbar();

    //Stage2: Move Compressed mf to rec_comp
    pass_mf_from_comp_buffer_to_rec_comp_in_crossbar();

    //Stage1: Move mf to compressor
    pass_mf_from_mem_partition_to_compressor();

  } else {

    //Stage2: Move Compressed mf to ICNT
    pass_mf_from_comp_buffer_to_icnt_in_crossbar();

    //added by kh(041616)
    //Stage1: Move mem_fetch from L2-to-ICNT queue to compressor and run compressor
    pass_mf_from_mem_partition_to_compressor();
    ///

  }

}

void gpgpu_sim::push_mf_from_mc_to_icnt_in_mesh()
{
  //added by kh(031816)
  if(g_hpcl_comp_config.hpcl_comp_en == 0
  || (g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE))
  {

    //added by kh(120515)
    // push from memory controller to interconnect
    for (unsigned m=0; m<m_memory_config->m_n_mem; m++)
    {
      std::vector<unsigned> ordered_subpartition_list;
      m_memory_partition_unit[m]->get_ordered_subpartition_index_list(ordered_subpartition_list);

      //std::cout << "ordered_subpartition_list.size() : " << ordered_subpartition_list.size() << std::endl;
      //std::cout << "m_memory_config->m_n_sub_partition_per_memory_channel : " << m_memory_config->m_n_sub_partition_per_memory_channel << std::endl;

      assert(ordered_subpartition_list.size() == m_memory_config->m_n_sub_partition_per_memory_channel);
      for (unsigned n=0; n<ordered_subpartition_list.size(); n++)
      {
	unsigned i = m*m_memory_config->m_n_sub_partition_per_memory_channel + ordered_subpartition_list[n];
	mem_fetch* mf = m_memory_sub_partition[i]->top();
	if (mf) {
	  unsigned response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
	  if ( ::icnt_has_buffer( m_shader_config->mem2device(m), response_size ) ) {
	    if (!mf->get_is_write())	mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);
	    mf->set_status(IN_ICNT_TO_SHADER,gpu_sim_cycle+gpu_tot_sim_cycle);

	    //Case3: MC-->ICNT
	    ::perf_icnt_push( m_shader_config->mem2device(m), mf->get_tpc(), mf, response_size, perf_packet_type::MC_TO_ICNT,gpu_sim_cycle,gpu_sim_cycle+gpu_tot_sim_cycle);

	    //See MCs actually used.
	    g_MCs_in_use.insert(mf->m_MC_id);
	    ///

	    m_memory_sub_partition[i]->pop();

	  } else {
	      gpu_stall_icnt2sh++;
	  }

	  unsigned next_prio_subpartition_idx = (ordered_subpartition_list[n] + 1)%m_memory_config->m_n_sub_partition_per_memory_channel;
	  m_memory_partition_unit[m]->set_next_prio_subpartition(next_prio_subpartition_idx);
	  break;

	} else {
	  m_memory_sub_partition[i]->pop();
	}
      }
    }
    ///
  }
  ///

  //added by kh(031816)
  if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo > hpcl_comp_config::GLOBAL_PRIVATE)
  {
    //Stage: Send mem_fetch to packetization
    for (unsigned m=0; m<m_memory_config->m_n_mem; m++)
    {
      std::vector<unsigned> ordered_subpartition_list;
      m_memory_partition_unit[m]->get_ordered_subpartition_index_list(ordered_subpartition_list);
      //std::cout << "ordered_subpartition_list.size() : " << ordered_subpartition_list.size() << std::endl;
      //std::cout << "m_memory_config->m_n_sub_partition_per_memory_channel : " << m_memory_config->m_n_sub_partition_per_memory_channel << std::endl;
      assert(ordered_subpartition_list.size() == m_memory_config->m_n_sub_partition_per_memory_channel);
      for (unsigned n=0; n<ordered_subpartition_list.size(); n++)
      {
	unsigned i = m*m_memory_config->m_n_sub_partition_per_memory_channel + ordered_subpartition_list[n];
	mem_fetch* mf = NULL;

	//deleted by kh(041816)
	/*
	if(g_hpcl_comp_config.hpcl_comp_word_size == 2) 	 mf = g_hpcl_comp_lwm_pl_2B[i]->top_compressed_mem_fetch();
	else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) mf = g_hpcl_comp_lwm_pl_4B[i]->top_compressed_mem_fetch();
	else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) mf = g_hpcl_comp_lwm_pl_8B[i]->top_compressed_mem_fetch();
	else assert(0);
	*/

	//added by kh(041816)
	mf = g_hpcl_comp_buffer[i]->top_mem_fetch();
	///


	if(mf) {

	  //deleted by kh(031916)
	  //unsigned response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();

	  //added by kh(031916)
	  //set the packet size based on the compression result.
	  unsigned response_size = 0;
	  if(mf->get_real_data_size() > 0) {
	    response_size = mf->get_ctrl_size()+mf->get_comp_data_size();

	    //add statistics
	    unsigned org_response_size = mf->size();
	    unsigned _flit_size = 32;
	    unsigned int n_comp_flits = response_size / _flit_size + ((response_size % _flit_size)? 1:0);
	    unsigned int n_org_flits = org_response_size / _flit_size + ((org_response_size % _flit_size)? 1:0);
	    double comp_ratio = (double)(n_org_flits-n_comp_flits) / n_org_flits;
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_RATIO, comp_ratio);
	    ///
	    //std::cout << "n_comp_flits " << n_comp_flits << " n_org_flits " << n_org_flits << " comp_ratio " << comp_ratio << std::endl;

	    //added by kh(041616)
	    double fragment_rate_per_flit = (double)(n_comp_flits*32 - response_size)/32;
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::FRAGMENT_RATE_PER_FLIT, fragment_rate_per_flit);
	    double fragment_rate_per_packet = (double)(n_comp_flits*32 - response_size)/(n_comp_flits*32);
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::FRAGMENT_RATE_PER_PACKET, fragment_rate_per_packet);

	    //std::cout << " n_comp_flits*32 " << n_comp_flits*32 << " response_size " << response_size;
	    //std::cout << " fragment_rate " << fragment_rate << std::endl;
	    if(n_comp_flits > n_org_flits) {
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_OVERHEAD_NO, 1);
	    }
	    ///

	    //added by kh(041816)
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::ORG_FLIT_NO, n_org_flits);
	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_FLIT_NO, n_comp_flits);
	    ///

	  } else {
	    response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
	  }
	  ///

	  if ( ::icnt_has_buffer( m_shader_config->mem2device(m), response_size ) ) {
	    if (!mf->get_is_write())	mf->set_return_timestamp(gpu_sim_cycle+gpu_tot_sim_cycle);
	    mf->set_status(IN_ICNT_TO_SHADER,gpu_sim_cycle+gpu_tot_sim_cycle);

	    //Case3: MC-->ICNT
	    ::perf_icnt_push( m_shader_config->mem2device(m), mf->get_tpc(), mf, response_size, perf_packet_type::MC_TO_ICNT,gpu_sim_cycle,gpu_sim_cycle+gpu_tot_sim_cycle);

	    //See MCs actually used.
	    g_MCs_in_use.insert(mf->m_MC_id);
	    ///

	    //deleted by kh(041816)
	    /*
	    if(g_hpcl_comp_config.hpcl_comp_word_size == 2) 	g_hpcl_comp_lwm_pl_2B[i]->pop_compressed_mem_fetch();
	    else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) 	g_hpcl_comp_lwm_pl_4B[i]->pop_compressed_mem_fetch();
	    else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) 	g_hpcl_comp_lwm_pl_8B[i]->pop_compressed_mem_fetch();
	    else assert(0);
	    */

	    //added by kh(041816)
	    g_hpcl_comp_buffer[i]->pop_mem_fetch();
	    ///

	  } else {
	      gpu_stall_icnt2sh++;
	  }

	  unsigned next_prio_subpartition_idx = (ordered_subpartition_list[n] + 1)%m_memory_config->m_n_sub_partition_per_memory_channel;
	  m_memory_partition_unit[m]->set_next_prio_subpartition(next_prio_subpartition_idx);
	  break;
	}
      }
    }

    /*
    //Stage: Move mem_fetch from L2-to-ICNT queue to compressor and run compressor
    for (unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++) {
      mem_fetch* mf = m_memory_sub_partition[i]->top();
      bool has_comp_buffer_space = false;

      if(g_hpcl_comp_config.hpcl_comp_word_size == 2) 	has_comp_buffer_space = g_hpcl_comp_lwm_pl_2B[i]->has_comp_buffer_space();
      else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) 	has_comp_buffer_space = g_hpcl_comp_lwm_pl_4B[i]->has_comp_buffer_space();
      else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) 	has_comp_buffer_space = g_hpcl_comp_lwm_pl_8B[i]->has_comp_buffer_space();
      else assert(0);

      if(has_comp_buffer_space == true) {

	//added by kh(030816)
	if(g_hpcl_comp_config.hpcl_comp_en == 1) {
	  //save data into mem_fetch object
	  //mem_fetch* mf = (mem_fetch*)data;
	  //unsigned node_id = g_icnt_interface->get_node_id(input);
	  //if(g_mc_placement_config.is_mc_node(node_id) == true) {
	  if(mf) {
	      unsigned char* real_data = mf->config_real_data(mf->get_data_size());
	      g_hpcl_comp->get_cache_data(mf, real_data);
	  }
      //      printf("Data = ");
      //      for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
      //      	printf("%02x", mf->get_real_data(i));
      //      }
      //      printf("\n");
      //
      //      printf("read_data_size = %u\n", mf->get_real_data_size());
	  //}
	}


	if(g_hpcl_comp_config.hpcl_comp_word_size == 2) {
	  g_hpcl_comp_lwm_pl_2B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_2B[i]->run();				//run compressor
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 4) {
	  g_hpcl_comp_lwm_pl_4B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_4B[i]->run();				//run compressor
	} else if(g_hpcl_comp_config.hpcl_comp_word_size == 8) {
	  g_hpcl_comp_lwm_pl_8B[i]->get_input()->set_mem_fetch(mf);	//send mem_fetch to compressor
	  g_hpcl_comp_lwm_pl_8B[i]->run();				//run compressor
	} else assert(0);


	m_memory_sub_partition[i]->pop();
      }
    }
    */

    //added by kh(041616)
    //Stage1: Move mem_fetch from L2-to-ICNT queue to compressor and run compressor
    pass_mf_from_mem_partition_to_compressor();
    ///
  }
  ///




}





void gpgpu_sim::cycle()
{
   //std::cout << "gpgpu_sim::cycle is called!!!" << std::endl;
   int clock_mask = next_clock_domain();

   //std::cout << "gpgpu_sim::cycle 1!!!" << std::endl;
   if (clock_mask & CORE ) {
       // shader core loading (pop from ICNT into core) follows CORE clock
      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++)
         m_cluster[i]->icnt_cycle();
   }

   //std::cout << "gpgpu_sim::cycle 2!!!" << std::endl;

   if (clock_mask & ICNT) {
      if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::GLOBAL_CROSSBAR) {
				if(g_hpcl_comp_config.hpcl_comp_en == 0)	push_mf_from_mc_to_icnt_in_crossbar();
				else 						push_mf_from_mc_to_icnt_in_crossbar_with_reply_compression();
      }

      //added by kh(041216)
      if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::MESH)
      {
	  push_mf_from_mc_to_icnt_in_mesh();
      }
      ///
   }

   //std::cout << "gpgpu_sim::cycle 3!!!" << std::endl;

   if (clock_mask & DRAM) {
      for (unsigned i=0;i<m_memory_config->m_n_mem;i++){
         m_memory_partition_unit[i]->dram_cycle(); // Issue the dram command (scheduler + delay model)
         // Update performance counters for DRAM
         m_memory_partition_unit[i]->set_dram_power_stats(m_power_stats->pwr_mem_stat->n_cmd[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_activity[CURRENT_STAT_IDX][i],
                        m_power_stats->pwr_mem_stat->n_nop[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_act[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_pre[CURRENT_STAT_IDX][i],
                        m_power_stats->pwr_mem_stat->n_rd[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_wr[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_req[CURRENT_STAT_IDX][i]);
      }
   }

   //std::cout << "gpgpu_sim::cycle 4!!!" << std::endl;

   // L2 operations follow L2 clock domain
   if (clock_mask & L2) {
      m_power_stats->pwr_mem_stat->l2_cache_stats[CURRENT_STAT_IDX].clear();

      //added by kh(041216)
      if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::GLOBAL_CROSSBAR)
      {
	for (unsigned i=0;i<m_memory_config->m_n_mem_sub_partition;i++) {
	    //move memory request from interconnect into memory partition (if not backed up)
	    //Note:This needs to be called in DRAM clock domain if there is no L2 cache in the system
	    if ( m_memory_sub_partition[i]->full() ) {
	      gpu_stall_dramfull++;
	    } else {
	      //commented out by kh(10/30/14), Case2 ICNT->MC
	      //mem_fetch* mf = (mem_fetch*) icnt_pop( m_shader_config->mem2device(i) );
	      //added by kh(10/30/14)
	      mem_fetch* mf = (mem_fetch*) perf_icnt_pop( m_shader_config->mem2device(i), perf_packet_type::ICNT_TO_MC, gpu_sim_cycle, gpu_sim_cycle+gpu_tot_sim_cycle);
	      ///
	      m_memory_sub_partition[i]->push( mf, gpu_sim_cycle + gpu_tot_sim_cycle );

	      #ifdef REQ_COAL_MODULE
	      //added by kh(042416)
	      //Support request coalescing. (Need to reimplement as a correct hardware)
	      if(mf && mf->has_merged_req_mfs() == true)	{
		std::vector<mem_fetch*>& merged_req_mfs = mf->get_merged_req_mfs();
		for(unsigned j = 0; j < merged_req_mfs.size(); j++) {
		  m_memory_sub_partition[i]->push( merged_req_mfs[j], gpu_sim_cycle + gpu_tot_sim_cycle );
		}
	      }
	      ///
	      #endif
	    }
	    m_memory_sub_partition[i]->cache_cycle(gpu_sim_cycle+gpu_tot_sim_cycle);
	    m_memory_sub_partition[i]->accumulate_L2cache_stats(m_power_stats->pwr_mem_stat->l2_cache_stats[CURRENT_STAT_IDX]);
	}
      }
      ///

      //added by kh(041216)
      if(g_hpcl_comp_config.hpcl_comp_noc_type == hpcl_comp_config::MESH)
      {
	//added by kh(120515)
	for (unsigned m=0; m<m_memory_config->m_n_mem; m++)
	{
	  mem_fetch* mf = (mem_fetch*) icnt_top_for_buffering(m_shader_config->mem2device(m));
	  for (unsigned n=0; n<m_memory_config->m_n_sub_partition_per_memory_channel; n++)
	  {
	    unsigned i = m*m_memory_config->m_n_sub_partition_per_memory_channel + n;
	    if ( m_memory_sub_partition[i]->full() ) {
	      gpu_stall_dramfull++;
	    } else {
		if(mf && mf->get_sub_partition_id() == i) {
		  m_memory_sub_partition[i]->push( mf, gpu_sim_cycle + gpu_tot_sim_cycle );
		  perf_icnt_pop( m_shader_config->mem2device(m), perf_packet_type::ICNT_TO_MC, gpu_sim_cycle, gpu_sim_cycle+gpu_tot_sim_cycle);
		}
	    }
	    m_memory_sub_partition[i]->cache_cycle(gpu_sim_cycle+gpu_tot_sim_cycle);
	    m_memory_sub_partition[i]->accumulate_L2cache_stats(m_power_stats->pwr_mem_stat->l2_cache_stats[CURRENT_STAT_IDX]);
	  }
	}
      }
      ///

   }

   //std::cout << "gpgpu_sim::cycle 5!!!" << std::endl;

   if (clock_mask & ICNT) {
      icnt_transfer();
   }


   //std::cout << "gpgpu_sim::cycle 6!!!" << std::endl;


   if (clock_mask & CORE) {

      // L1 cache + shader core pipeline stages
      m_power_stats->pwr_mem_stat->core_cache_stats[CURRENT_STAT_IDX].clear();
      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
         if (m_cluster[i]->get_not_completed() || get_more_cta_left() ) {
               m_cluster[i]->core_cycle();
               *active_sms+=m_cluster[i]->get_n_active_sms();
         }
         // Update core icnt/cache stats for GPUWattch
         m_cluster[i]->get_icnt_stats(m_power_stats->pwr_mem_stat->n_simt_to_mem[CURRENT_STAT_IDX][i], m_power_stats->pwr_mem_stat->n_mem_to_simt[CURRENT_STAT_IDX][i]);
         m_cluster[i]->get_cache_stats(m_power_stats->pwr_mem_stat->core_cache_stats[CURRENT_STAT_IDX]);
      }
      float temp=0;
      for (unsigned i=0;i<m_shader_config->num_shader();i++){
        temp+=m_shader_stats->m_pipeline_duty_cycle[i];
      }
      temp=temp/m_shader_config->num_shader();
      *average_pipeline_duty_cycle=((*average_pipeline_duty_cycle)+temp);
        //cout<<"Average pipeline duty cycle: "<<*average_pipeline_duty_cycle<<endl;


      if( g_single_step && ((gpu_sim_cycle+gpu_tot_sim_cycle) >= g_single_step) ) {
          asm("int $03");
      }
      gpu_sim_cycle++;
      if( g_interactive_debugger_enabled )
         gpgpu_debug();

      // McPAT main cycle (interface with McPAT)
#ifdef GPGPUSIM_POWER_MODEL
      if(m_config.g_power_simulation_enabled){
          mcpat_cycle(m_config, getShaderCoreConfig(), m_gpgpusim_wrapper, m_power_stats, m_config.gpu_stat_sample_freq, gpu_tot_sim_cycle, gpu_sim_cycle, gpu_tot_sim_insn, gpu_sim_insn);
      }
#endif

      issue_block2core();

      // Depending on configuration, flush the caches once all of threads are completed.
      int all_threads_complete = 1;
      if (m_config.gpgpu_flush_l1_cache) {
         for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
            if (m_cluster[i]->get_not_completed() == 0)
                m_cluster[i]->cache_flush();
            else
               all_threads_complete = 0 ;
         }
      }

      if(m_config.gpgpu_flush_l2_cache){
          if(!m_config.gpgpu_flush_l1_cache){
              for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
                  if (m_cluster[i]->get_not_completed() != 0){
                      all_threads_complete = 0 ;
                      break;
                  }
              }
          }

         if (all_threads_complete && !m_memory_config->m_L2_config.disabled() ) {
            printf("Flushed L2 caches...\n");
            if (m_memory_config->m_L2_config.get_num_lines()) {
               int dlc = 0;
               for (unsigned i=0;i<m_memory_config->m_n_mem;i++) {
                  dlc = m_memory_sub_partition[i]->flushL2();
                  assert (dlc == 0); // need to model actual writes to DRAM here
                  printf("Dirty lines flushed from L2 %d is %d\n", i, dlc  );
               }
            }
         }
      }

      /*
      //added by kh(061215)
      if (all_threads_complete) {

    	  if(g_dbg_msg_vector.size() > 0) {

    		  for(int i = 0; i < g_dbg_msg_vector.size(); i++)
			  {
    			  mem_fetch_debug::print_debug_msg(g_dbg_msg_vector[i]);
			  }
    		  g_dbg_msg_vector.clear();
			  g_dbg_msg_map.clear();
    	  }
      }
      ///
      */

      if (!(gpu_sim_cycle % m_config.gpu_stat_sample_freq)) {
         time_t days, hrs, minutes, sec;
         time_t curr_time;
         time(&curr_time);
         unsigned long long  elapsed_time = MAX(curr_time - g_simulation_starttime, 1);
         if ( (elapsed_time - last_liveness_message_time) >= m_config.liveness_message_freq ) {
            days    = elapsed_time/(3600*24);
            hrs     = elapsed_time/3600 - 24*days;
            minutes = elapsed_time/60 - 60*(hrs + 24*days);
            sec = elapsed_time - 60*(minutes + 60*(hrs + 24*days));
            printf("GPGPU-Sim uArch: cycles simulated: %lld  inst.: %lld (ipc=%4.1f) sim_rate=%u (inst/sec) elapsed = %u:%u:%02u:%02u / %s",
                   gpu_tot_sim_cycle + gpu_sim_cycle, gpu_tot_sim_insn + gpu_sim_insn,
                   (double)gpu_sim_insn/(double)gpu_sim_cycle,
                   (unsigned)((gpu_tot_sim_insn+gpu_sim_insn) / elapsed_time),
                   (unsigned)days,(unsigned)hrs,(unsigned)minutes,(unsigned)sec,
                   ctime(&curr_time));
            fflush(stdout);
            last_liveness_message_time = elapsed_time;
         }
         visualizer_printstat();
         m_memory_stats->memlatstat_lat_pw();
         if (m_config.gpgpu_runtime_stat && (m_config.gpu_runtime_stat_flag != 0) ) {
            if (m_config.gpu_runtime_stat_flag & GPU_RSTAT_BW_STAT) {
               for (unsigned i=0;i<m_memory_config->m_n_mem;i++)
                  m_memory_partition_unit[i]->print_stat(stdout);
               printf("maxmrqlatency = %d \n", m_memory_stats->max_mrq_latency);
               printf("maxmflatency = %d \n", m_memory_stats->max_mf_latency);
            }
            if (m_config.gpu_runtime_stat_flag & GPU_RSTAT_SHD_INFO)
               shader_print_runtime_stat( stdout );
            if (m_config.gpu_runtime_stat_flag & GPU_RSTAT_L1MISS)
               shader_print_l1_miss_stat( stdout );
            if (m_config.gpu_runtime_stat_flag & GPU_RSTAT_SCHED)
               shader_print_scheduler_stat( stdout, false );
         }

	  //added by kh(092416)
	  if(g_hpcl_comp_config.hpcl_l1d_cache_stat_mon_en == 1) {
	    if(!m_shader_config->m_L1D_config.disabled()) {
	      struct cache_sub_stats total_css, css;
	      total_css.clear();
	      css.clear();
	      //printf("L1D_cache:\n");
	      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++){
		m_cluster[i]->get_L1D_sub_stats(css);
		//printf("\tL1D_cache_core[%d]: Access = %d, Miss = %d, Miss_rate = %.3lf, Pending_hits = %u, Reservation_fails = %u\n",
		//	   i, css.accesses, css.misses, (double)css.misses / (double)css.accesses, css.pending_hits, css.res_fails);
		total_css += css;
	      }
	      //printf("total_cache_accesses = %u %u\n", total_css.accesses, g_prev_total_L1D_css.accesses);
	      unsigned accesses = total_css.accesses-g_prev_total_L1D_css.accesses;
	      unsigned misses = total_css.misses-g_prev_total_L1D_css.misses;
	      //printf("\tL1D_total_cache_accesses = %u\n", accesses);
	      //printf("\tL1D_total_cache_misses = %u\n", misses);
	      if(accesses > 0){
		//printf("L1D_total_cache_miss_rate_sample = %.4lf, %llu\n", (double)misses / (double)accesses, (gpu_tot_sim_cycle + gpu_sim_cycle));
		printf("L1D_total_cache_miss_rate_sample = %.4lf\n", (double)misses / (double)accesses);
	      } else {
		printf("L1D_total_cache_miss_rate_sample = 0\n");
	      }
	      g_prev_total_L1D_css = total_css;
	    }
	  }
	  ///

	  if(g_hpcl_comp_config.hpcl_adaptive_comp_en == 1) {
	    //added by kh(101816)
	    //Update L1D cache stat
	    if(!m_shader_config->m_L1D_config.disabled()) {

	      for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
		struct cache_sub_stats css;
		css.clear();
		m_cluster[i]->get_L1D_sub_stats(css);
		struct cache_sub_stats& prev_css = m_cluster[i]->get_L1D_cache_stat();
		unsigned accesses = css.accesses-prev_css.accesses;
		unsigned misses = css.misses-prev_css.misses;
		double cache_miss_rate = 0;
		if(accesses > 0){
		  cache_miss_rate = (double)misses / (double)accesses;
		  printf("L1D_total_cache_miss_rate_sample_SM%d = %.4lf, accesses %f, misses %f\n", i, (double)misses / (double)accesses, accesses, misses);
		} else {
		  printf("L1D_total_cache_miss_rate_sample_SM%d = 0\n", i);
		}
		m_cluster[i]->save_L1D_cache_stat(css);

		hpcl_dyn_comp& dyn_comp = m_cluster[i]->get_dyn_comp();
		dyn_comp.update_sample_stat(cache_miss_rate);

		//debugging
		/*
		enum hpcl_dyn_comp::dyn_comp_pred_state ret = dyn_comp.run();
		if(ret == hpcl_dyn_comp::CACHE_COMP_OFF)	printf("Kernel %s | L1D Cache Comp Off | SM %u | MID\n", dyn_comp.get_active_cache_state_kernel_name().c_str(), i);
		else						printf("Kernel %s | L1D Cache Comp On  | SM %u | MID\n", dyn_comp.get_active_cache_state_kernel_name().c_str(), i);
		*/
	      }

	    }
	    ///
	  }
      }

      if (!(gpu_sim_cycle % 20000)) {
         // deadlock detection
         if (m_config.gpu_deadlock_detect && gpu_sim_insn == last_gpu_sim_insn) {
            gpu_deadlock = true;
         } else {
            last_gpu_sim_insn = gpu_sim_insn;
         }
      }
      try_snap_shot(gpu_sim_cycle);
      spill_log_to_file (stdout, 0, gpu_sim_cycle);

   }
}


void shader_core_ctx::dump_warp_state( FILE *fout ) const
{
   fprintf(fout, "\n");
   fprintf(fout, "per warp functional simulation status:\n");
   for (unsigned w=0; w < m_config->max_warps_per_shader; w++ )
       m_warp[w].print(fout);
}

void gpgpu_sim::dump_pipeline( int mask, int s, int m ) const
{
/*
   You may want to use this function while running GPGPU-Sim in gdb.
   One way to do that is add the following to your .gdbinit file:

      define dp
         call g_the_gpu.dump_pipeline_impl((0x40|0x4|0x1),$arg0,0)
      end

   Then, typing "dp 3" will show the contents of the pipeline for shader core 3.
*/

   printf("Dumping pipeline state...\n");
   if(!mask) mask = 0xFFFFFFFF;
   for (unsigned i=0;i<m_shader_config->n_simt_clusters;i++) {
      if(s != -1) {
         i = s;
      }
      if(mask&1) m_cluster[m_shader_config->sid_to_cluster(i)]->display_pipeline(i,stdout,1,mask & 0x2E);
      if(s != -1) {
         break;
      }
   }
   if(mask&0x10000) {
      for (unsigned i=0;i<m_memory_config->m_n_mem;i++) {
         if(m != -1) {
            i=m;
         }
         printf("DRAM / memory controller %u:\n", i);
         if(mask&0x100000) m_memory_partition_unit[i]->print_stat(stdout);
         if(mask&0x1000000)   m_memory_partition_unit[i]->visualize();
         if(mask&0x10000000)   m_memory_partition_unit[i]->print(stdout);
         if(m != -1) {
            break;
         }
      }
   }
   fflush(stdout);
}

const struct shader_core_config * gpgpu_sim::getShaderCoreConfig()
{
   return m_shader_config;
}

const struct memory_config * gpgpu_sim::getMemoryConfig()
{
   return m_memory_config;
}

simt_core_cluster * gpgpu_sim::getSIMTCluster()
{
   return *m_cluster;
}

