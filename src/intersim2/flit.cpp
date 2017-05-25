// $Id: flit.cpp 5188 2012-08-30 00:31:31Z dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*flit.cpp
 *
 *flit struct is a flit, carries all the control signals that a flit needs
 *Add additional signals as necessary. Flits has no concept of length
 *it is a singluar object.
 *
 *When adding objects make sure to set a default value in this constructor
 */

#include "booksim.hpp"
#include "flit.hpp"

stack<Flit *> Flit::_all;
stack<Flit *> Flit::_free;

ostream& operator<<( ostream& os, const Flit& f )
{
  os << "  Flit ID: " << f.id << " (" << &f << ")" 
     << " Packet ID: " << f.pid
     << " Type: " << f.type 
     << " Head: " << f.head
     << " Tail: " << f.tail << endl;
  os << "  Source: " << f.src << "  Dest: " << f.dest << " Intm: "<<f.intm<<endl;
  os << "  Creation time: " << f.ctime << " Injection time: " << f.itime << " Arrival time: " << f.atime << " Phase: "<<f.ph<< endl;
  os << "  VC: " << f.vc << endl;
  return os;
}

Flit::Flit() 
{  
  Reset();
}  

void Flit::Reset() 
{  
  type      = ANY_TYPE ;
  vc        = -1 ;
  cl        = -1 ;
  head      = false ;
  tail      = false ;
  ctime     = -1 ;
  itime     = -1 ;
  atime     = -1 ;
  id        = -1 ;
  pid       = -1 ;
  hops      = 0 ;
  watch     = false ;
  record    = false ;
  intm = 0;
  src = -1;
  dest = -1;
  pri = 0;
  intm =-1;
  ph = -1;
  data = 0;

  //added by kh(061616)
  replica_dests.clear();
  replica_flits.clear();
  ///

  //added by kh(061916)
  is_multicast_flit = false;
  ///

  //added by kh(062016)
  org_pid = -1;
  ///
}  

Flit * Flit::New() {
  Flit * f;
  if(_free.empty()) {
    f = new Flit;
    _all.push(f);
  } else {
    f = _free.top();
    f->Reset();
    _free.pop();
  }
  return f;
}

void Flit::Free() {
  _free.push(this);
}

void Flit::FreeAll() {
  while(!_all.empty()) {
    delete _all.top();
    _all.pop();
  }
}


//added by kh(061816)
unsigned Flit::GetReplicaDestNo() {
  return replica_dests.size();
}

int Flit::GetReplicaDest(int index) {
  return replica_dests[index];
}

void Flit::AddReplicaDest(int dest) {
  replica_dests.push_back(dest);
}

unsigned Flit::GetReplicaFlitNo() {
  return replica_flits.size();
}

Flit* Flit::GetReplicaFlit(int index) {
  assert(replica_flits.size() > index);
  return replica_flits[index];
}

void Flit::AddReplicaFlit(Flit* flit) {
  replica_flits.push_back(flit);
}

///

//added by kh(061816)
void Flit::DelReplicaDest(int index) {
  assert(index >= 0);
  replica_dests.erase(replica_dests.begin()+index);
}

void Flit::DelReplicaFlit(int index) {
  assert(index >= 0);
  replica_flits.erase(replica_flits.begin()+index);
}


bool Flit::get_is_multicast_flit() {
  return is_multicast_flit;
}

void Flit::set_is_multicast_flit() {
  is_multicast_flit = true;
}

void Flit::SetReplicaFlit(int index, Flit* flit) {
  replica_flits[index] = flit;
}

void Flit::SetReplicaDest(int index, int dest) {
  replica_dests[index] = dest;
}

void Flit::ClearReplicaDest() {
  replica_dests.clear();
}
void Flit::ClearReplicaFlit() {
  replica_flits.clear();
}

bool Flit::HasReplicaFlit(Flit* flit) {
  for(unsigned i = 0; i < replica_flits.size(); i++) {
    if(replica_flits[i] && replica_flits[i] == flit) {
	return true;
    }
  }
  return false;
}

bool Flit::AreAllReplicaFlitsNull() {
  for(unsigned i = 0; i < replica_flits.size(); i++) {
    if(replica_flits[i]) {
      return false;
    }
  }
  return true;
}

