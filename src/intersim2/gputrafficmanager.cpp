// Copyright (c) 2009-2013, Tor M. Aamodt, Dongdong Li, Ali Bakhoda
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

#include <sstream>
#include <fstream>
#include <limits> 

#include "gputrafficmanager.hpp"
#include "interconnect_interface.hpp"
#include "globals.hpp"

//added by kh(070715)
#include "../gpgpu-sim/user-defined.h"
#include "../gpgpu-sim/mem_fetch.h"
extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
///

//added by kh(121715)
extern struct mc_placement_config g_mc_placement_config;


//added by kh(122815)
#include "../gpgpu-sim/hpcl_network_anal.h"
extern hpcl_network_anal* g_hpcl_network_anal;
///

//added by kh(062116)
#include "../gpgpu-sim/hpcl_user_define_stmts.h"
///

GPUTrafficManager::GPUTrafficManager( const Configuration &config, const vector<Network *> &net)
:TrafficManager(config, net)
{
  // The total simulations equal to number of kernels
  _total_sims = 0;
  
  _input_queue.resize(_subnets);
  for ( int subnet = 0; subnet < _subnets; ++subnet) {
    _input_queue[subnet].resize(_nodes);
    for ( int node = 0; node < _nodes; ++node ) {
      _input_queue[subnet][node].resize(_classes);
    }
  }

  //added by kh(122815)
  assert(g_hpcl_network_anal);
  g_hpcl_network_anal->create(_subnets, _net[0]->GetChannels().size(), _nodes, _vcs);
  ///

  //added by kh(062016)
  //set separate input queue for multicast
  _multicast_input_queue.resize(_nodes);
  int _max_multicast_group_size = config.GetInt( "max_multicast_group_size_per_router" );
  for (int node = 0; node < _nodes; ++node) {
    _multicast_input_queue[node].resize(_max_multicast_group_size);
  }
  ///
}

GPUTrafficManager::~GPUTrafficManager()
{
}

void GPUTrafficManager::Init()
{
  //added by kh(061715)
  _tot_sim_time = _tot_sim_time + _time;

  _time = 0;
  _sim_state = running;
  _ClearStats( );
  
  //added by kh(122815)
  g_hpcl_network_anal->clear();
}

void GPUTrafficManager::_RetireFlit( Flit *f, int dest )
{
  _deadlock_timer = 0;
  
  assert(_total_in_flight_flits[f->cl].count(f->id) > 0);
  _total_in_flight_flits[f->cl].erase(f->id);
  
  if(f->record) {
    assert(_measured_in_flight_flits[f->cl].count(f->id) > 0);
    _measured_in_flight_flits[f->cl].erase(f->id);
  }
  
  if ( f->watch ) {
  //if(1) {
    *gWatchOut << GetSimTime() << " | "
    << "node" << dest << " | "
    << "Retiring flit " << f->id
    << " (packet " << f->pid
    << ", src = " << f->src
    << ", dest = " << f->dest
	//added by kh(062515)
	<< ", const_dest = " << f->const_dest
	<< ", hops = " << f->hops
    << ", flat = " << f->atime - f->itime
    << ", subnetwork = " << f->subnetwork
    << ", class = " << f->cl
    << ", priority = " << f->pri
	<< ")." << endl;
  }
  

  if ( f->head && ( f->dest != dest ) ) {
    ostringstream err;
    err << "Flit " << f->id << " arrived at incorrect output " << dest;
    Error( err.str( ) );
  }
  
  if((_slowest_flit[f->cl] < 0) ||
     (_flat_stats[f->cl]->Max() < (f->atime - f->itime)))
    _slowest_flit[f->cl] = f->id;
  
  _flat_stats[f->cl]->AddSample( f->atime - f->itime);
  if(_pair_stats){
    _pair_flat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - f->itime );
  }
  
  if ( f->tail ) {
    Flit * head;
    if(f->head) {
      head = f;
    } else {
      map<int, Flit *>::iterator iter = _retired_packets[f->cl].find(f->pid);
      assert(iter != _retired_packets[f->cl].end());
      head = iter->second;
      _retired_packets[f->cl].erase(iter);
      assert(head->head);
      assert(f->pid == head->pid);
    }
    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | "
      << "node" << dest << " | "
      << "Retiring packet " << f->pid
      << " (plat = " << f->atime - head->ctime
      << ", nlat = " << f->atime - head->itime
      << ", frag = " << (f->atime - head->atime) - (f->id - head->id) // NB: In the spirit of solving problems using ugly hacks, we compute the packet length by taking advantage of the fact that the IDs of flits within a packet are contiguous.
      << ", src = " << head->src
      << ", dest = " << head->dest
      << ")." << endl;
    }
   
// GPGPUSim: Memory will handle reply, do not need this
#if 0
    //code the source of request, look carefully, its tricky ;)
    if (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST) {
      PacketReplyInfo* rinfo = PacketReplyInfo::New();
      rinfo->source = f->src;
      rinfo->time = f->atime;
      rinfo->record = f->record;
      rinfo->type = f->type;
      _repliesPending[dest].push_back(rinfo);
    } else {
      if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY  ){
        _requestsOutstanding[dest]--;
      } else if(f->type == Flit::ANY_TYPE) {
        _requestsOutstanding[f->src]--;
      }
      
    }
#endif

    if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY  ){
      _requestsOutstanding[dest]--;
    } else if(f->type == Flit::ANY_TYPE) {
      ostringstream err;
      err << "Flit " << f->id << " cannot be ANY_TYPE" ;
      Error( err.str( ) );
    }
    
    // Only record statistics once per packet (at tail)
    // and based on the simulation state
    if ( ( _sim_state == warming_up ) || f->record ) {
      
      _hop_stats[f->cl]->AddSample( f->hops );
      
      if((_slowest_packet[f->cl] < 0) ||
         (_plat_stats[f->cl]->Max() < (f->atime - head->itime)))
        _slowest_packet[f->cl] = f->pid;
      _plat_stats[f->cl]->AddSample( f->atime - head->ctime);
      _nlat_stats[f->cl]->AddSample( f->atime - head->itime);
      _frag_stats[f->cl]->AddSample( (f->atime - head->atime) - (f->id - head->id) );
      
      if(_pair_stats){
        _pair_plat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - head->ctime );
        _pair_nlat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - head->itime );
      }
    }
    
    if(f != head) {
      head->Free();
    }
    
    /*
    //added by kh(070215)
    double zeroload_lat = ((f->hops-1)*2 + (f->hops-1) + 1 + f->flit_no);
    _p_zeroload_lat[f->subnetwork][f->const_dest] += zeroload_lat;

    //deleted by kh(092415)
    //_p_dynamic_lat[f->subnetwork][f->const_dest] += ((f->atime-head->itime) - zeroload_lat);

    //added by kh(092415)
    _p_dynamic_lat[f->subnetwork][f->const_dest] += ((f->tatime-head->itime) - zeroload_lat);
    _p_ejection_lat[f->subnetwork][f->const_dest] += (f->atime-f->tatime);
    ///

    //std::cout << "fid: " << f->id << ", dest: " << f->const_dest << ", (head->itime-head->ctime) = " << (head->itime-head->ctime) << std::endl;
    _p_queing_lat[f->subnetwork][f->const_dest] += (head->itime-head->ctime);
    _p_packet_lat[f->subnetwork][f->const_dest] += (f->atime - head->ctime);
    _p_hop_no[f->subnetwork][f->const_dest] += f->hops;
    _p_no[f->subnetwork][f->const_dest]++;
    ///
    */

    //added by kh(122915)
    double zeroload_lat = 1 + (f->hops-1)*3 + (f->hops-1) + f->flit_no;
    double contention_lat = (f->atime-head->itime) - zeroload_lat;
    if(f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST) {
	//unsigned long long head_flit_inj_time = ((mem_fetch*)f->data)->m_status_timestamp[HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM];
	//unsigned long long tail_flit_arr_time = ((mem_fetch*)f->data)->m_status_timestamp[TAIL_FLIT_ARRIVING_IN_VC_IN_MEM];
	//std::cout << "req_zeroload_lat = " << zeroload_lat << " f->flit_no = " << f->flit_no << std::endl;

	//std::cout << "add_sample(REQ_ZEROLOAD_LAT) calls" << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_ZEROLOAD_LAT, zeroload_lat);

	//std::cout << "add_sample(REQ_CONTENTION_LAT) calls" << std::endl;
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, (tail_flit_arr_time-head_flit_inj_time) - zeroload_lat);
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, (f->tatime-head->itime) - zeroload_lat);
	//std::cout << "req_contention_lat = " << contention_lat << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, contention_lat);

	//((mem_fetch*)f->data)->req_zeroload_lat = (unsigned long long)zeroload_lat;
	//((mem_fetch*)f->data)->req_dynamic_lat = (unsigned long long)((tail_flit_arr_time-head_flit_inj_time) - zeroload_lat);
    } else if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY) {

	//std::cout << "rep_zeroload_lat = " << zeroload_lat << " f->flit_no = " << f->flit_no << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_ZEROLOAD_LAT, zeroload_lat);
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_CONTENTION_LAT, (f->tatime-head->itime) - zeroload_lat);
	//std::cout << "rep_contention_lat = " << contention_lat << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_CONTENTION_LAT, contention_lat);

	//((mem_fetch*)f->data)->rep_zeroload_lat = (unsigned long long)zeroload_lat;
	//((mem_fetch*)f->data)->rep_dynamic_lat = (unsigned long long)((f->tatime-head->itime) - zeroload_lat);
    }
    ///







  }
  
  if(f->head && !f->tail) {
    _retired_packets[f->cl].insert(make_pair(f->pid, f));
  } else {
    f->Free();
  }
}
int  GPUTrafficManager::_IssuePacket( int source, int cl )
{
  return 0;
}

//TODO: Remove stype?
void GPUTrafficManager::_GeneratePacket(int source, int stype, int cl, int time, int subnet, int packet_size, const Flit::FlitType& packet_type, void* const data, int dest)
{
  assert(stype!=0);
  
  //  Flit::FlitType packet_type = Flit::ANY_TYPE;
  int size = packet_size; //input size
  int pid = _cur_pid++;
  assert(_cur_pid);
  int packet_destination = dest;
  bool record = false;
  bool watch = gWatchOut && (_packets_to_watch.count(pid) > 0);
  
  // In GPGPUSim, the core specified the packet_type and size
  
#if 0
  if(_use_read_write[cl]){
    if(stype > 0) {
      if (stype == 1) {
        packet_type = Flit::READ_REQUEST;
        size = _read_request_size[cl];
      } else if (stype == 2) {
        packet_type = Flit::WRITE_REQUEST;
        size = _write_request_size[cl];
      } else {
        ostringstream err;
        err << "Invalid packet type: " << packet_type;
        Error( err.str( ) );
      }
    } else {
      PacketReplyInfo* rinfo = _repliesPending[source].front();
      if (rinfo->type == Flit::READ_REQUEST) {//read reply
        size = _read_reply_size[cl];
        packet_type = Flit::READ_REPLY;
      } else if(rinfo->type == Flit::WRITE_REQUEST) {  //write reply
        size = _write_reply_size[cl];
        packet_type = Flit::WRITE_REPLY;
      } else {
        ostringstream err;
        err << "Invalid packet type: " << rinfo->type;
        Error( err.str( ) );
      }
      packet_destination = rinfo->source;
      time = rinfo->time;
      record = rinfo->record;
      _repliesPending[source].pop_front();
      rinfo->Free();
    }
  }
#endif
  
  if ((packet_destination <0) || (packet_destination >= _nodes)) {
    ostringstream err;
    err << "Incorrect packet destination " << packet_destination
    << " for stype " << packet_type;
    Error( err.str( ) );
  }
  
  if ( ( _sim_state == running ) ||
      ( ( _sim_state == draining ) && ( time < _drain_time ) ) ) {
    record = _measure_stats[cl];
  }
  
  int subnetwork = subnet;
  //                ((packet_type == Flit::ANY_TYPE) ?
  //                    RandomInt(_subnets-1) :
  //                    _subnet[packet_type]);
  
  if ( watch ) {
    *gWatchOut << GetSimTime() << " | "
    << "node" << source << " | "
    << "Enqueuing packet " << pid
    << " at time " << time
    << "." << endl;
  }
  
  for ( int i = 0; i < size; ++i ) {
    Flit * f  = Flit::New();
    f->id     = _cur_id++;
    assert(_cur_id);
    f->pid    = pid;
    f->watch  = watch | (gWatchOut && (_flits_to_watch.count(f->id) > 0));
    f->subnetwork = subnetwork;
    f->src    = source;
    f->ctime  = time;
    f->record = record;
    f->cl     = cl;
    f->data = data;
    
    _total_in_flight_flits[f->cl].insert(make_pair(f->id, f));
    if(record) {
      _measured_in_flight_flits[f->cl].insert(make_pair(f->id, f));
    }
    
    if(gTrace){
      cout<<"New Flit "<<f->src<<endl;
    }
    f->type = packet_type;
    
    if ( i == 0 ) { // Head flit
      f->head = true;
      //packets are only generated to nodes smaller or equal to limit
      f->dest = packet_destination;

      //added by kh(062515)
      f->const_dest = packet_destination;
      ///

      //added by kh(121715)
      if(g_mc_placement_config.is_sm_node(source)) {
    	  ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM, _time);
      } else if(g_mc_placement_config.is_mc_node(source)) {
    	  ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM, _time);
    	  //added by kh(122815)
    	  //double req_mc_lat = _time - ((mem_fetch*)f->data)->get_timestamp(PACKET_EJECTED_FROM_NI_OUTPUT_BUF_IN_MEM);
    	  double req_mc_lat = _time - ((mem_fetch*)f->data)->get_timestamp(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM);
    	  //std::cout << "mf " << ((mem_fetch*)f->data)->get_request_uid() << " req_mc_lat : " << req_mc_lat << std::endl;
    	  if(req_mc_lat < 0 || req_mc_lat > 5000)
    	  {
//	    std::cout << "test_req_mc_lat " << req_mc_lat;
//    	    std::cout << " is from " << _time << std::endl;
//	    std::cout << " to ";
//	    std::cout << ((mem_fetch*)f->data)->get_timestamp(PACKET_EJECTED_FROM_NI_OUTPUT_BUF_IN_MEM);
//	    std::cout << std::endl;
    	  }
    	  assert(req_mc_lat > 0);

    	  //assert(req_mc_lat>=0 && req_mc_lat <= 5000);
    	  g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_MC_LAT, req_mc_lat);
    	  ///
      }
      ///

      //added by kh(092315)
      //xy-yx hybrid routing
      //if(source >= 0 && source < SM_NUM && (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST)) {
      //added by kh(121715)
      if(g_mc_placement_config.is_sm_node(source) && (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST)) {
    	  if(source%2 == 0) 	f->route_type = Flit::XY_ROUTE;
    	  else			f->route_type = Flit::YX_ROUTE;

    	  //if(source%2 == 0) 	f->route_type = Flit::YX_ROUTE;
    	  //else					f->route_type = Flit::XY_ROUTE;

    	  //srand(GetSimTime());
    	  //if(m_route_hist[source]%2 == 0) 	f->route_type = Flit::XY_ROUTE;
    	  //else								f->route_type = Flit::YX_ROUTE;
    	  //m_route_hist[source] = (m_route_hist[source] == 0)? 1 : 0;
      }
      ///

    } else {
      f->head = false;
      f->dest = -1;

      //added by kh(062515)
      f->const_dest = packet_destination;
      ///
    }
    switch( _pri_type ) {
      case class_based:
        f->pri = _class_priority[cl];
        assert(f->pri >= 0);
        break;
      case age_based:
        f->pri = numeric_limits<int>::max() - time;
        assert(f->pri >= 0);
        break;
      case sequence_based:
        f->pri = numeric_limits<int>::max() - _packet_seq_no[source];
        assert(f->pri >= 0);
        break;
      default:
        f->pri = 0;
    }
    if ( i == ( size - 1 ) ) { // Tail flit
      f->tail = true;

      //added by kh(070715)
      f->flit_no = packet_size;
      ///

    } else {
      f->tail = false;
    }
    
    f->vc  = -1;
    
    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | "
      << "node" << source << " | "
      << "Enqueuing flit " << f->id
      << " (packet " << f->pid
      << ") at time " << time
      << "." << endl;
    }
    
    //added by kh(061816)
    //Replication
    static std::vector<int> replica_pids;
    //static std::vector<mem_fetch*> replica_dests;
    static mem_fetch* org_mf = NULL;
    if(i == 0) {
      //Collect all dests of coalesced read replies for multicast
      mem_fetch* mf = (mem_fetch*) data;

      if(mf) {
	MCT_DEBUG_PRINT("mf %u -- merged_mf_no %d \n", mf->get_request_uid(), mf->get_merged_mf_no());
      }

      //added by kh(062516)
      if(mf->get_merged_mf_no() > 0) {
      //deleted by kh(062516)
      //if(mf->get_coalesced_read_reply_mf_no() > 0) {
	org_mf = mf;

	MCT_DEBUG_PRINT("Flit %d MF %u -- # of replicas %d \n", f->id, mf->get_request_uid(), mf->get_merged_mf_no()+1);

	//Make a replica for the first mf
	int replica_dest = f->dest;
	int replica_pid = _cur_pid++;
	Flit* replica_flit = ReplicateFlit(f, -1, f->dest, mf, replica_pid);
	f->AddReplicaFlit(replica_flit);
	f->AddReplicaDest(replica_dest);
	replica_pids.push_back(replica_pid);
	MCT_DEBUG_PRINT("\tsource %d | replica flit %d (pid %d) to dest %d is replicated for mf %u\n", source, replica_flit->id, replica_pid, f->dest, mf->get_request_uid());

	//deleted by kh(062516)
	//for(int j = 0; j < org_mf->get_coalesced_read_reply_mf_no(); j++) {
	//added by kh(062516)
	for(int j = 0; j < org_mf->get_merged_mf_no(); j++) {
	  //int replica_dest = org_mf->get_coalesced_read_reply_mf(j)->get_tpc();
	  //deleted by kh(062516)
	  //mem_fetch* replica_mf = org_mf->get_coalesced_read_reply_mf(j);
	  //added by kh(062516)
	  mem_fetch* replica_mf = org_mf->get_merged_mf(j);
	  replica_dest = g_icnt_interface->get_icnt_node_id(replica_mf->get_tpc());
	  replica_pid = _cur_pid++;
	  assert(replica_mf);
	  Flit* replica_flit = ReplicateFlit(f, -1, replica_dest, replica_mf, replica_pid);
	  f->AddReplicaFlit(replica_flit);
	  f->AddReplicaDest(replica_dest);
	  replica_pids.push_back(replica_pid);
	  MCT_DEBUG_PRINT("\tsource %d | replica flit %d (pid %d) to dest %d is replicated for mf %u\n", source, replica_flit->id, replica_pid, replica_dest, replica_mf->get_request_uid());

	  //added by kh(062216)
	  //network timestamp
	  assert(g_mc_placement_config.is_mc_node(source));
	  replica_mf->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM, _time);

	  //mfs with SECONDARY_MISS did not access the memory. So, req_mc_lat is zero.
	  double req_mc_lat = 0;
	  g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_MC_LAT, req_mc_lat);
	  ///
	}

	//remove original flit from pool
	assert(_total_in_flight_flits[f->cl].count(f->id) > 0);
	_total_in_flight_flits[f->cl].erase(f->id);

	if(f->record) {
	  assert(_measured_in_flight_flits[f->cl].count(f->id) > 0);
	  _measured_in_flight_flits[f->cl].erase(f->id);
	}
	///

      }
      ///
    } else {

      if(org_mf) {

	//deleted by kh(062516)
	//if(org_mf->get_coalesced_read_reply_mf_no() > 0) {
	//added by kh(062516)
	if(org_mf->get_merged_mf_no() > 0) {

	  MCT_DEBUG_PRINT("Flit %d -- # of replicas %d \n", f->id, org_mf->get_merged_mf_no()+1);

	  //Make a replica for the first mf
	  int replica_dest = f->const_dest;
	  int replica_pid = replica_pids[0];
	  Flit* replica_flit = ReplicateFlit(f, -1, f->const_dest, org_mf, replica_pid);
	  f->AddReplicaFlit(replica_flit);
	  MCT_DEBUG_PRINT("\tsource %d | replica flit %d (pid %d) to dest %d is replicated for mf %u\n", source, replica_flit->id, replica_pid, replica_dest, org_mf->get_request_uid());
	  //deleted by kh(062516)
	  //for(int j = 0; j < org_mf->get_coalesced_read_reply_mf_no(); j++) {
	  //  mem_fetch* replica_mf = org_mf->get_coalesced_read_reply_mf(j);
	  //added by kh(062516)
	  for(int j = 0; j < org_mf->get_merged_mf_no(); j++) {
	    mem_fetch* replica_mf = org_mf->get_merged_mf(j);
	    assert(replica_mf);
	    replica_dest = g_icnt_interface->get_icnt_node_id(replica_mf->get_tpc());
	    replica_pid = replica_pids[j+1];
	    replica_flit = ReplicateFlit(f, -1, replica_dest, replica_mf, replica_pid);
	    f->AddReplicaFlit(replica_flit);
	    MCT_DEBUG_PRINT("\tsource %d | replica flit %d (pid %d) to dest %d is replicated for mf %u\n", source, replica_flit->id, replica_pid, replica_dest, replica_mf->get_request_uid());
	  }
//	  if(i == ( size - 1 )) {
//	    org_mf = NULL;
//	    replica_pids.clear();
//	    MCT_DEBUG_PRINT("\treplication is done!!\n");
//	    //assert(0);
//	  }

	  //remove original flit from pool
	  assert(_total_in_flight_flits[f->cl].count(f->id) > 0);
	  _total_in_flight_flits[f->cl].erase(f->id);

	  if(f->record) {
	    assert(_measured_in_flight_flits[f->cl].count(f->id) > 0);
	    _measured_in_flight_flits[f->cl].erase(f->id);
	  }
	  ///

	}
      }
    }
    ///

    _input_queue[subnet][source][cl].push_back( f );

    //added by kh(062016)
    //Store all replica flits into _multicast_input_queue
    for(unsigned j = 0; j < f->GetReplicaFlitNo(); j++) {
      _multicast_input_queue[source][j].push_back(f->GetReplicaFlit(j));
    }
    ///

    if(i == ( size - 1 )) {
      org_mf = NULL;
      replica_pids.clear();
      //MCT_DEBUG_PRINT("\treplication is done!!\n");
      //assert(0);
    }
  }
}

void GPUTrafficManager::_Step()
{
  bool flits_in_flight = false;
  for(int c = 0; c < _classes; ++c) {
    flits_in_flight |= !_total_in_flight_flits[c].empty();
  }
  if(flits_in_flight && (_deadlock_timer++ >= _deadlock_warn_timeout)){
    _deadlock_timer = 0;
    cout << "WARNING: Possible network deadlock.\n";

    //added by kh(062016)
    for(int c = 0; c < _classes; ++c) {
    	std::cout << "class : " << c << std::endl;
    	map<int, Flit *>::iterator it = _total_in_flight_flits[c].begin();
    	for(; it != _total_in_flight_flits[c].end(); ++it)
    	{
    		std::cout << "flit " << it->second->id << " is not retired!" << std::endl;
    	}
    }
    ///
  }
  
  //std::cout << "hi 1" << std::endl;

  vector<map<int, Flit *> > flits(_subnets);
  for ( int subnet = 0; subnet < _subnets; ++subnet ) {
    for ( int n = 0; n < _nodes; ++n ) {
      Flit * const f = _net[subnet]->ReadFlit( n );
      if ( f ) {
        if(f->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Ejecting flit " << f->id
          << " (packet " << f->pid << ")"
          << " from VC " << f->vc
          << "." << endl;
        }
        g_icnt_interface->WriteOutBuffer(subnet, n, f);
      }
      
      g_icnt_interface->Transfer2BoundaryBuffer(subnet, n);
      Flit* const ejected_flit = g_icnt_interface->GetEjectedFlit(subnet, n);
      if (ejected_flit) {
        if(ejected_flit->head)
          assert(ejected_flit->dest == n);
        if(ejected_flit->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Ejected flit " << ejected_flit->id
          << " (packet " << ejected_flit->pid
          << " VC " << ejected_flit->vc << ")"
          << "from ejection buffer." << endl;
        }
        flits[subnet].insert(make_pair(n, ejected_flit));
        if((_sim_state == warming_up) || (_sim_state == running)) {
          ++_accepted_flits[ejected_flit->cl][n];
          if(ejected_flit->tail) {
            ++_accepted_packets[ejected_flit->cl][n];
          }
        }
      }
    
      // Processing the credit From the network
      Credit * const c = _net[subnet]->ReadCredit( n );
      if ( c ) {
#ifdef TRACK_FLOWS
        for(set<int>::const_iterator iter = c->vc.begin(); iter != c->vc.end(); ++iter) {
          int const vc = *iter;
          assert(!_outstanding_classes[n][subnet][vc].empty());
          int cl = _outstanding_classes[n][subnet][vc].front();
          _outstanding_classes[n][subnet][vc].pop();
          assert(_outstanding_credits[cl][subnet][n] > 0);
          --_outstanding_credits[cl][subnet][n];
        }
#endif
        _buf_states[n][subnet]->ProcessCredit(c);
        c->Free();
      }
    }
    _net[subnet]->ReadInputs( );
  }

  //std::cout << "hi 2" << std::endl;

// GPGPUSim will generate/inject packets from interconnection interface
#if 0
  if ( !_empty_network ) {
    _Inject();
  }
#endif
  
  for(int subnet = 0; subnet < _subnets; ++subnet) {
    
    for(int n = 0; n < _nodes; ++n) {
      
      Flit * f = NULL;
      
      BufferState * const dest_buf = _buf_states[n][subnet];
      
      int const last_class = _last_class[n][subnet];
      
      int class_limit = _classes;
      
      if(_hold_switch_for_packet) {
        list<Flit *> const & pp = _input_queue[subnet][n][last_class];
        if(!pp.empty() && !pp.front()->head &&
           !dest_buf->IsFullFor(pp.front()->vc)) {
          f = pp.front();
          assert(f->vc == _last_vc[n][subnet][last_class]);
          
          // if we're holding the connection, we don't need to check that class
          // again in the for loop
          --class_limit;
        }
      }
      
      //added by kh(061916)
      vector<Flit*> flits_to_VC;
      Flit * org_cf = NULL;

      //std::cout << "hi 2_1" << std::endl;

      for(int i = 1; i <= class_limit; ++i) {
        
        int const c = (last_class + i) % _classes;
        
        list<Flit *> const & pp = _input_queue[subnet][n][c];
        
        if(pp.empty()) {
          continue;
        }
        
        //deleted by kh(061916)
        //Flit * const cf = pp.front();
        //added by kh(061916)
        Flit * cf = pp.front();
        assert(cf);
        assert(cf->cl == c);
        
        assert(cf->subnetwork == subnet);
        
        if(f && (f->pri >= cf->pri)) {
          continue;
        }
        

        if(cf->GetReplicaFlitNo() == 0)	{
          f = DecideInputPortVC(subnet, n, c, dest_buf, cf);
   	  flits_to_VC.push_back(f);
        } else {

	  for(unsigned k = 0; k < cf->GetReplicaFlitNo(); k++) {

	    Flit* replica_cf = NULL;
	    if(!_multicast_input_queue[n][k].empty())	replica_cf = _multicast_input_queue[n][k].front();

	    if(replica_cf) {

	      if(replica_cf->watch) {
		*gWatchOut << GetSimTime() << " | "
				<< "node" << n << " | "
				<< "org_pid " << replica_cf->org_pid << " flit " << replica_cf->id << std::endl;
	      }

	      //Flits of a new packet arrive at _input_queue, before all replicated flits are sent,
	      if(replica_cf->org_pid != cf->pid) {
		/*
		std::cout << GetSimTime() << " | "
			      << "node" << n << " | "
			      << "org_pid " << replica_cf->org_pid << " replica_flit " << replica_cf->id << std::endl;

		std::cout << GetSimTime() << " | "
			      << "node" << n << " | "
			      << "pid " << cf->pid << " flit " << cf->id << std::endl;
		*/
		//Do nothing!. Wait until all previous replicas are sent.
		//std::cout << "replica_cf " << replica_cf->id << " is passed!!! due to front flit " << cf->id << std::endl;
		replica_cf = NULL;
	      } else {
		replica_cf = DecideInputPortVC(subnet, n, c, dest_buf, replica_cf, k);
	      }
	    }

	    flits_to_VC.push_back(replica_cf);
	    /*
	    if(replica_cf) {
	      std::cout << "replica_cf " << replica_cf->id << " vc " << replica_cf->vc << std::endl;
	    }
	    */
	  }

        }
      }
      
      //std::cout << "hi 2_2" << std::endl;

      Flit* first_flit = NULL;
      for(unsigned j = 0; j < flits_to_VC.size(); j++)
      {
        f = flits_to_VC[j];
        
	if(f) {

	  assert(f->subnetwork == subnet);

	  int const c = f->cl;

	  if(f->head) {

	    if (_lookahead_routing) {
	      if(!_noq) {
		const FlitChannel * inject = _net[subnet]->GetInject(n);
		const Router * router = inject->GetSink();
		assert(router);
		int in_channel = inject->GetSinkPort();
		_rf(router, f, in_channel, &f->la_route_set, false);
		if(f->watch) {
		  *gWatchOut << GetSimTime() << " | "
		  << "node" << n << " | "
		  << "Generating lookahead routing info for flit " << f->id
		  << "." << endl;
		}
	      } else if(f->watch) {
		*gWatchOut << GetSimTime() << " | "
		<< "node" << n << " | "
		<< "Already generated lookahead routing info for flit " << f->id
		<< " (NOQ)." << endl;
	      }
	    } else {
	      f->la_route_set.Clear();
	    }

	    //added by kh(061916)
	    //std::cout << "flit " << f->id << " takes buffer of VC " << f->vc << std::endl;

	    dest_buf->TakeBuffer(f->vc);
	    _last_vc[n][subnet][c] = f->vc;

	    //added by kh(061816)
	    //std::cout << "flit " << f->id << " f->GetReplicaFlitNo() " << f->GetReplicaFlitNo() << std::endl;
	    /*
	    for(unsigned r = 0; r < f->GetReplicaFlitNo(); r++) {
	      Flit* replicaFlit = f->GetReplicaFlit(r);
	      dest_buf->TakeBuffer(replicaFlit->vc);
	      std::cout << "\treplicaFlit " << replicaFlit->id << " vc " << replicaFlit->vc << " takes buffer " << std::endl;
	    }
	    ///
	    */
	  }

	  _last_class[n][subnet] = c;

	  //deleted by kh(062016)
	  //_input_queue[subnet][n][c].pop_front();

	  //added by kh(062016)
	  if(f->get_is_multicast_flit() == false)	_input_queue[subnet][n][c].pop_front();
	  else {
	    _multicast_input_queue[n][j].pop_front();

	    //replicas are sent in different speed.
	    //need to search all flits in _input_queue.
	    list<Flit *> & pp = _input_queue[subnet][n][c];
	    list<Flit *>::iterator it = pp.begin();
	    bool found = false;
	    for(; it != pp.end(); ++it) {
	      Flit* org_flit = (*it);
	      for(unsigned k = 0; k < org_flit->GetReplicaFlitNo(); k++) {
		if(org_flit->GetReplicaFlit(k) == f) {
		  org_flit->SetReplicaFlit(k, NULL);
		  found = true; break;
		}
	      }
	      if(found == true) break;
	    }
	    assert(found == true);
	    /*
	    Flit* org_flit = _input_queue[subnet][n][c].front();
	    bool found = false;
	    for(unsigned k = 0; k < org_flit->GetReplicaFlitNo(); k++) {
	      if(org_flit->GetReplicaFlit(k) == f) {
		org_flit->SetReplicaFlit(k, NULL);
		found = true;
	      }
	    }
	    */

	    Flit* org_front_flit = _input_queue[subnet][n][c].front();
	    assert(org_front_flit);
	    if(org_front_flit->AreAllReplicaFlitsNull() == true) {
	      //org_flit->ClearReplicaDest();
	      //org_flit->ClearReplicaFlit();
		org_front_flit->Free();
	      _input_queue[subnet][n][c].pop_front();
	    }
	  }

  #ifdef TRACK_FLOWS
	  ++_outstanding_credits[c][subnet][n];
	  _outstanding_classes[n][subnet][f->vc].push(c);
  #endif

	  dest_buf->SendingFlit(f);

	  /*
	  //added by kh(061816)
	  //for credit maintenance
	  //std::cout << "flit " << f->id << " f->GetReplicaFlitNo() " << f->GetReplicaFlitNo() << std::endl;
	  for(unsigned r = 0; r < f->GetReplicaFlitNo(); r++) {
	    Flit* replicaFlit = f->GetReplicaFlit(r);
	    dest_buf->SendingFlit(replicaFlit);
	    std::cout << "\treplicaFlit " << replicaFlit->id << " send flit " << std::endl;
	  }
	  ///
	  */
	  if(_pri_type == network_age_based) {
	    f->pri = numeric_limits<int>::max() - _time;
	    assert(f->pri >= 0);
	  }

	  if(f->watch) {
	    *gWatchOut << GetSimTime() << " | "
	    << "node" << n << " | "
	    << "Injecting flit " << f->id
	    << " into subnet " << subnet
	    << " at time " << _time
	    << " with priority " << f->pri
	    << "." << endl;
	  }
	  f->itime = _time;

	  //deleted by kh(062016)
	  /*
	  // Pass VC "back"
	  if(!_input_queue[subnet][n][c].empty() && !f->tail) {
	    Flit * const nf = _input_queue[subnet][n][c].front();
	    nf->vc = f->vc;
	  }
	  */
	  //added by kh(062016)
	  if(f->get_is_multicast_flit() == false) {
	    if(!_input_queue[subnet][n][c].empty() && !f->tail) {
	      Flit * const nf = _input_queue[subnet][n][c].front();
	      nf->vc = f->vc;
	    }
	  } else {
	    //pass VC back for replica body/tail flits
	    if(!_multicast_input_queue[n][j].empty() && !f->tail) {
	      Flit * const nf = _multicast_input_queue[n][j].front();
	      nf->vc = f->vc;

	      MCT_DEBUG_PRINT("pass VC %d back from current rep_flit %d to next rep_flit %d\n",
		      f->vc, f->id, nf->id);
	    }
	  }

	  if((_sim_state == warming_up) || (_sim_state == running)) {
	    ++_sent_flits[c][n];

	    //added by kh(070215)
	    ++_sent_flits_net[subnet][n];


	    if(f->head) {
	      ++_sent_packets[c][n];

	      //added by kh(070916)
	      if(f->type < Flit::ANY_TYPE) {
		//added by kh(121715)
		if(g_mc_placement_config.is_sm_node(n)) {
		    ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM, _time);

		    //added by kh(122815)
		    double req_queuing_lat_in_sm = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM);
		    g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_QUEUING_LAT_IN_SM, req_queuing_lat_in_sm);
		    ///

		} else if(g_mc_placement_config.is_mc_node(n)) {
		    ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM, _time);
		    //added by kh(122815)
		    double rep_queuing_lat_in_mc = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM);
		    g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_QUEUING_LAT_IN_MC, rep_queuing_lat_in_mc);
		    ///

		}
	      }
	      ///
	    }
	  }

  #ifdef TRACK_FLOWS
	  ++_injected_flits[c][n];
  #endif

	  //deleted by kh(062016)
	  //_net[subnet]->WriteFlit(f, n);

	  //added by kh(062016)
	  if(first_flit == NULL) {
	    _net[subnet]->WriteFlit(f, n);
	    first_flit = f;
	  } else {
	    first_flit->AddReplicaFlit(f);
	  }
	  ///
	} //end of if(f)

      } // end of for(unsigned j = 0; j < flits_to_VC.size(); j++)


    }
  }

  //std::cout << "hi 3" << std::endl;

  //Send the credit To the network
  for(int subnet = 0; subnet < _subnets; ++subnet) {
    for(int n = 0; n < _nodes; ++n) {
      map<int, Flit *>::const_iterator iter = flits[subnet].find(n);
      if(iter != flits[subnet].end()) {
        Flit * const f = iter->second;

        f->atime = _time;

        //added by kh(070715)
        if(f->tail) {
	  //added by kh(030816)
	  if(f->type < Flit::ANY_TYPE) {
	    //added by kh(121715)
	    if(g_mc_placement_config.is_sm_node(n)) {
	      //std::cout << "mf's id: " << ((mem_fetch*)f->data)->get_request_uid() << ", time: " << _time << std::endl;
	      ((mem_fetch*)f->data)->set_status(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_SM, _time);
	      //added by kh(122815)
	      double rep_net_lat = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM);
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_NET_LAT, rep_net_lat);
	      ///

	    } else if(g_mc_placement_config.is_mc_node(n)) {
	      ((mem_fetch*)f->data)->set_status(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM, _time);

	      //added by kh(122815)
	      double req_net_lat = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM);
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_NET_LAT, req_net_lat);
	      ///
	    }
	  }
        }
        ///

        if(f->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Injecting credit for VC " << f->vc
          << " into subnet " << subnet
          << "." << endl;
        }
        Credit * const c = Credit::New();
        c->vc.insert(f->vc);
        _net[subnet]->WriteCredit(c, n);
        
#ifdef TRACK_FLOWS
        ++_ejected_flits[f->cl][n];
#endif
        
        _RetireFlit(f, n);
      }
    }
    flits[subnet].clear();
    // _InteralStep here
    _net[subnet]->Evaluate( );					//run router's pipeline
    _net[subnet]->WriteOutputs( );				//send flits..
  }

  //std::cout << "hi 4" << std::endl;

  ++_time;
  assert(_time);
  if(gTrace){
    cout<<"TIME "<<_time<<endl;
  }
  
}


//added by kh(061816)
Flit* GPUTrafficManager::ReplicateFlit(Flit* f, int vc, int dest, void* data, int packet_id)
{
  Flit * replica  = Flit::New();
  replica->id = _cur_id++;

  //inherit data from flit
  //replica->pid = f->pid;
  replica->pid = packet_id; //_cur_pid++;	//consider replicated flits are from different packets.
  replica->watch = f->watch;
  replica->subnetwork = f->subnetwork;
  replica->src    = f->src;
  replica->ctime  = f->ctime;
  replica->record = f->record;
  replica->cl     = f->cl;

  _total_in_flight_flits[replica->cl].insert(make_pair(replica->id, replica));
  if(replica->record) {
    _measured_in_flight_flits[replica->cl].insert(make_pair(replica->id, replica));
  }
  replica->type = f->type;
  replica->head = f->head;
  //replica->dest = f->dest;

  replica->pri = f->pri;
  replica->tail = f->tail;

  replica->vc  = vc;
  replica->data = data;

  replica->const_dest = dest;
  if(f->head == true)	replica->dest = dest;

  replica->set_is_multicast_flit();
  replica->org_pid = f->pid;

  //replica->watch = true;
  //added by kh(062216)
  replica->flit_no = f->flit_no;
  ///

  return replica;
}

Flit* GPUTrafficManager::DecideInputPortVC(int subnet, int n, int c, BufferState * const dest_buf, Flit* in_flit, int replica_index)
{
  Flit* cf = in_flit;
  Flit* out_flit = NULL;

  //KH: routing output port is decided here.
  if(cf->head && cf->vc == -1) { // Find first available VC

    OutputSet route_set;
    _rf(NULL, cf, -1, &route_set, true);
    set<OutputSet::sSetElement> const & os = route_set.GetSet();
    assert(os.size() == 1);
    OutputSet::sSetElement const & se = *os.begin();
    assert(se.output_port == -1);
    int vc_start = se.vc_start;
    int vc_end = se.vc_end;
    int vc_count = vc_end - vc_start + 1;
    if(_noq) {
      assert(_lookahead_routing);
      const FlitChannel * inject = _net[subnet]->GetInject(n);
      const Router * router = inject->GetSink();
      assert(router);
      int in_channel = inject->GetSinkPort();


      // NOTE: Because the lookahead is not for injection, but for the
      // first hop, we have to temporarily set cf's VC to be non-negative
      // in order to avoid seting of an assertion in the routing function.
      cf->vc = vc_start;
      _rf(router, cf, in_channel, &cf->la_route_set, false);
      cf->vc = -1;

      if(cf->watch) {
	*gWatchOut << GetSimTime() << " | "
	<< "node" << n << " | "
	<< "Generating lookahead routing info for flit " << cf->id
	<< " (NOQ)." << endl;
      }
      set<OutputSet::sSetElement> const sl = cf->la_route_set.GetSet();
      assert(sl.size() == 1);
      int next_output = sl.begin()->output_port;
      vc_count /= router->NumOutputs();
      vc_start += next_output * vc_count;
      vc_end = vc_start + vc_count - 1;
      assert(vc_start >= se.vc_start && vc_start <= se.vc_end);
      assert(vc_end >= se.vc_start && vc_end <= se.vc_end);
      assert(vc_start <= vc_end);
    }
    if(cf->watch) {
      *gWatchOut << GetSimTime() << " | " << FullName() << " | "
      << "Finding output VC for flit " << cf->id
      << ":" << endl;
    }

    if(replica_index == -1) {
      for(int i = 1; i <= vc_count; ++i) {
	int const lvc = _last_vc[n][subnet][c];
	int const vc =
	(lvc < vc_start || lvc > vc_end) ?
	vc_start :
	(vc_start + (lvc - vc_start + i) % vc_count);
	assert((vc >= vc_start) && (vc <= vc_end));
	if(!dest_buf->IsAvailableFor(vc)) {
	  if(cf->watch) {
	    *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	    << "  Output VC " << vc << " is busy." << endl;
	  }
	} else {
	  if(dest_buf->IsFullFor(vc)) {
	    if(cf->watch) {
	      *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	      << "  Output VC " << vc << " is full." << endl;
	    }
	  }
	  else {
	    if(cf->watch) {
	      *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	      << "  Selected output VC " << vc << "." << endl;
	    }
	    cf->vc = vc;
	    //first_flit_vc = cf->vc;
	    break;
	  }
	}
      }
    } else {

      int org_flit_vc = 0;	//Need to change to support multiple VCs
      int replica_vc = dest_buf->GetMulticastVC(org_flit_vc, replica_index);

      if(!dest_buf->IsAvailableFor(replica_vc)) {
	if(cf->watch) {
	  *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	  << "  Output VC " << replica_vc << " is busy." << endl;
	}
      } else {
	if(dest_buf->IsFullFor(replica_vc)) {
	  if(cf->watch) {
	    *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	    << "  Output VC " << replica_vc << " is full." << endl;
	  }
	}
	else {
	  if(cf->watch) {
	    *gWatchOut << GetSimTime() << " | " << FullName() << " | "
	    << "  Selected output VC " << replica_vc << "." << endl;
	  }
	  cf->vc = replica_vc;
	  //break;
	}
      }

    }
  }

  if(cf->vc == -1) {
    if(cf->watch) {
      *gWatchOut << GetSimTime() << " | " << FullName() << " | "
      << "No output VC found for flit " << cf->id
      << "." << endl;
    }
  } else {
    if(dest_buf->IsFullFor(cf->vc)) {
      if(cf->watch) {
	*gWatchOut << GetSimTime() << " | " << FullName() << " | "
	<< "Selected output VC " << cf->vc
	<< " is full for flit " << cf->id
	<< "." << endl;
      }
    } else {
      out_flit = cf;
      assert(out_flit->vc >= 0);
    }
  }

  return out_flit;
}

///


