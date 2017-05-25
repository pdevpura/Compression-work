// $Id: power_module.cpp 5188 2012-08-30 00:31:31Z dub $

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

#include "power_module.hpp"
#include "booksim_config.hpp"
#include "buffer_monitor.hpp"
#include "switch_monitor.hpp"
#include "iq_router.hpp"

Power_Module::Power_Module(Network * n , const Configuration &config)
  : Module( 0, "power_module" ){

  
  string pfile = config.GetStr("tech_file");
  PowerConfig pconfig;
  pconfig.ParseFile(pfile);

  net = n;
  output_file_name = config.GetStr("power_output_file");
  classes = config.GetInt("classes");
  channel_width = (double)config.GetInt("channel_width");
  channel_sweep = (double)config.GetInt("channel_sweep");

  numVC = (double)config.GetInt("num_vcs");
  depthVC  = (double)config.GetInt("vc_buf_size");

  //////////////////////////////////Constants/////////////////////////////
  //wire length in (mm)
  wire_length = pconfig.GetFloat("wire_length");
  //////////Metal Parameters////////////
  // Wire left/right coupling capacitance [ F/mm ]
  Cw_cpl = pconfig.GetFloat("Cw_cpl"); 
  // Wire up/down groudn capacitance      [ F/mm ]
  Cw_gnd = pconfig.GetFloat("Cw_gnd");
  Cw = 2.0 * Cw_cpl + 2.0 * Cw_gnd ;
  Rw = pconfig.GetFloat("Rw");
  // metal pitch [mm]
  MetalPitch = pconfig.GetFloat("MetalPitch"); 
  
  //////////Device Parameters////////////
  
  LAMBDA =  pconfig.GetFloat("LAMBDA")  ;       // [um/LAMBDA]
  Cd     =  pconfig.GetFloat("Cd");           // [F/um] (for Delay)
  Cg     =  pconfig.GetFloat("Cg");           // [F/um] (for Delay)
  Cgdl   =  pconfig.GetFloat("Cgdl");           // [F/um] (for Delay)
  
  Cd_pwr =  pconfig.GetFloat("Cd_pwr") ;           // [F/um] (for Power)
  Cg_pwr =  pconfig.GetFloat("Cg_pwr") ;           // [F/um] (for Power)
			       
  IoffN  = pconfig.GetFloat("IoffN");            // [A/um]
  IoffP  = pconfig.GetFloat("IoffP");            // [A/um]
  // Leakage from bitlines, two-port cell  [A]
  IoffSRAM = pconfig.GetFloat("IoffSRAM");  
  // [Ohm] ( D1=1um Inverter)
  R        = pconfig.GetFloat("R");                  
  // [F]   ( D1=1um Inverter - for Power )
  Ci_delay = (1.0 + 2.0) * ( Cg + Cgdl );   
  // [F]   ( D1=1um Inverter - for Power )
  Co_delay = (1.0 + 2.0) * Cd ;              


  Ci = (1.0 + 2.0) * Cg_pwr ;
  Co = (1.0 + 2.0) * Cd_pwr ;

  Vdd    = pconfig.GetFloat("Vdd");
  FO4    = R * ( 3.0 * Cd + 12 * Cg + 12 * Cgdl);		     
  tCLK   = 20 * FO4;
  fCLK   = 1.0 / tCLK;              

  H_INVD2=(double)pconfig.GetInt("H_INVD2");
  W_INVD2=(double)pconfig.GetInt("W_INVD2") ;
  H_DFQD1=(double)pconfig.GetInt("H_DFQD1");
  W_DFQD1= (double)pconfig.GetInt("W_DFQD1");
  H_ND2D1= (double)pconfig.GetInt("H_ND2D1");
  W_ND2D1=(double)pconfig.GetInt("W_ND2D1");
  H_SRAM=(double)pconfig.GetInt("H_SRAM");
  W_SRAM=(double)pconfig.GetInt("W_SRAM");

  ChannelPitch = 2.0 * MetalPitch ;
  CrossbarPitch = 2.0 * MetalPitch ;

  // Jiayi, DSENT power model ***************
  dsent = config.GetInt("dsent_model");

  energy_per_buffwrite = config.GetFloat("energy_per_buffwrite");
  energy_per_buffread = config.GetFloat("energy_per_buffread");
  energy_traverse_xbar = config.GetFloat("energy_traverse_xbar");
  energy_per_arbitratestage1 = config.GetFloat("energy_per_arbitratestage1");
  energy_per_arbitratestage2 = config.GetFloat("energy_per_arbitratestage2");
  energy_distribute_clk = config.GetFloat("energy_distribute_clk");
  energy_rr_link_traversal = config.GetFloat("energy_rr_link_traversal");
  energy_rs_link_traversal = config.GetFloat("energy_rs_link_traversal");

  input_leak = config.GetFloat("input_leak");
  switch_leak = config.GetFloat("switch_leak");
  xbar_leak = config.GetFloat("xbar_leak");
  xbar_sel_dff_leak = config.GetFloat("xbar_sel_dff_leak");
  clk_tree_leak = config.GetFloat("clk_tree_leak");
  pipeline_reg0_leak = config.GetFloat("pipeline_reg0_leak");
  pipeline_reg1_leak = config.GetFloat("pipeline_reg1_leak");
  pipeline_reg2_part_leak = config.GetFloat("pipeline_reg2_part_leak");
  rr_link_leak = config.GetFloat("rr_link_leak");
  rs_link_leak = config.GetFloat("rs_link_leak");

  frequency = config.GetFloat("frequency");
  // *********************************
}

Power_Module::~Power_Module(){


}


//////////////////////////////////////////////
//Channels
//////////////////////////////////////////////

void Power_Module::calcChannel(const FlitChannel* f){
  double channelLength = f->GetLatency()* wire_length;
  wire const this_wire = wireOptimize(channelLength);
  double const & K = this_wire.K;
  double const & N = this_wire.N;
  double const & M = this_wire.M;
  //area
  channelArea += areaChannel(K,N,M);

  //activity factor;
  const vector<int> temp = f->GetActivity();
  vector<double> a(classes);
  for(int i = 0; i< classes; i++){

    a[i] = ((double)temp[i])/totalTime;
  }

  //power calculation
  double const bitPower = powerRepeatedWire(channelLength, K,M,N);

  channelClkPower += powerWireClk(M,channel_width);
  for(int i = 0; i< classes; i++){
    channelWirePower += bitPower * a[i]*channel_width;
    channelDFFPower += powerWireDFF(M, channel_width, a[i]);
  }
  channelLeakPower+= powerRepeatedWireLeak(K,M,N)*channel_width;
}

wire const & Power_Module::wireOptimize(double L){
  map<double, wire>::iterator iter = wire_map.find(L);
  if(iter == wire_map.end()){
    
    double W = 64;
    double bestMetric =  100000000 ;
    double bestK = -1;
    double bestM = -1;
    double bestN = -1;
    for (double K = 1.0 ; K < 10 ; K+=0.1 ) {
      for (double N = 1.0 ; N < 40 ; N += 1.0 ) {
	for (double M = 1.0 ; M < 40.0 ; M +=1.0 ) {
	  double l = 1.0 * L/( N * M) ;
	  
	  double k0 = R * (Co_delay + Ci_delay) ;
	  double k1 = R/K * Cw + K * Rw * Ci_delay ;
	  double k2 = 0.5 * Rw * Cw ;
	  double Tw = k0 + (k1 * l) + k2 * (l * l) ;
	  double alpha = 0.2 ;
	  double power = alpha * W * powerRepeatedWire( L, K, M, N) + powerWireDFF( M, W, alpha ) ;
	  double metric = M * M * M * M * power ;
	  if ( (N*Tw) < (0.8 * tCLK) ) {
	    if ( metric < bestMetric ) {
	      bestMetric = metric ;
	      bestK = K ;
	      bestM = M ;
	      bestN = N ;
	    }
	  }
	}
      }
    }
    cout<<"L = "<<L<<" K = "<<bestK<<" M = "<<bestM<<" N = "<<bestN<<endl;
    
    wire const temp = {L, bestK, bestM, bestN};
    iter = wire_map.insert(make_pair(L, temp)).first;
  }
  return iter->second;
}

double Power_Module::powerRepeatedWire(double L, double K, double M, double N){
  
  double segments = 1.0 * M * N ;
  double Ca = K * (Ci + Co) + Cw * (L/segments) ;
  double Pa = 0.5 * Ca * Vdd * Vdd * fCLK;
  return Pa * M * N  ;

}

double Power_Module::powerRepeatedWireLeak (double K, double M, double N){
  double Pl = K * 0.5 * ( IoffN + 2.0 * IoffP ) * Vdd  ;
  return Pl * M * N ;

}

double Power_Module:: powerWireClk (double M, double W){
  // number of clock wires running down one repeater bank
  double columns = H_DFQD1 * MetalPitch /  ChannelPitch ;

  // length of clock wire
  double clockLength = W * ChannelPitch ;
  double Cclk = (1 + 5.0/16.0 * (1+Co_delay/Ci_delay)) * (clockLength * Cw * columns +W * Ci_delay);

  return M * Cclk * (Vdd * Vdd) * fCLK ;

}

double Power_Module::powerWireDFF(double M, double W, double alpha){
  double Cdin = 2 * 0.8 * (Ci + Co) + 2 * ( 2.0/3.0 * 0.8 * Co )  ;
  double Cclk = 2 * 0.8 * (Ci + Co) + 2 * ( 2.0/3.0 * 0.8 * Cg_pwr) ;
  double Cint = (alpha * 0.5) * Cdin + alpha * Cclk ;
  
  return Cint * M * W * (Vdd*Vdd) * fCLK ;
}


///////////////////////////////////////////////////////////////
//Memory
//////////////////////////////////////////////////////////////
void Power_Module::calcBuffer(const BufferMonitor *bm){
  double depth = numVC * depthVC  ;
  double Pleak = powerMemoryBitLeak( depth ) * channel_width ;
  //area

  const vector<int> reads = bm->GetReads();
  const vector<int> writes = bm->GetWrites();
  for(int i = 0; i<bm->NumInputs(); i++){
    inputArea += areaInputModule( depth );
    inputLeakagePower += Pleak ;
    for(int j = 0; j< classes; j++){
      double ar = ((double)reads[i* classes+j])/totalTime;
      double aw = ((double)writes[i* classes+j])/totalTime;
      if(ar>1 ||aw >1){
	cout<<"activity factor is greater than one, soemthing is stomping memory\n"; exit(-1);
      }
      double Pwl =  powerWordLine( channel_width, depth) ;
      double Prd = powerMemoryBitRead( depth ) * channel_width ;
      double Pwr = powerMemoryBitWrite( depth ) * channel_width ; 
      inputReadPower    += ar * ( Pwl + Prd ) ;
      inputWritePower   += aw * ( Pwl + Pwr ) ;
    }
  }
}


double Power_Module::powerWordLine(double memoryWidth, double memoryDepth){
  // wordline capacitance
  double Ccell = 2 * ( 4.0 * LAMBDA ) * Cg_pwr +  6 * MetalPitch * Cw ;     
  double Cwl = memoryWidth * Ccell ; 

  // wordline circuits
  double Warray = 8 * MetalPitch + memoryDepth ;
  double x = 1.0 + (5.0/16.0) * (1 + Co/Ci)  ;
  double Cpredecode = x * (Cw * Warray  * Ci) ;
  double Cdecode    = x * Cwl ;

  // bitline circuits
  double Harray =  6 * memoryWidth * MetalPitch ;
  double y = (1 + 0.25) * (1 + Co/Ci) ;
  double Cprecharge = y * ( Cw * Harray + 3 * channel_width * Ci ) ;
  double Cwren      = y * ( Cw * Harray + 2 * channel_width * Ci ) ;

  double Cbd = Cprecharge + Cwren ;
  double Cwd = 2 * Cpredecode + Cdecode ;

  return ( Cbd + Cwd ) * Vdd * Vdd * fCLK ;
  
}

double Power_Module::powerMemoryBitRead(double memoryDepth){
  // bitline capacitance
  double Ccell  = 4.0 * LAMBDA * Cd_pwr + 8 * MetalPitch * Cw ; 
  double Cbl    = memoryDepth * Ccell ;
  double Vswing = Vdd  ;
  return ( Cbl ) * ( Vdd * Vswing ) * fCLK ;
}

double Power_Module:: powerMemoryBitWrite(double memoryDepth){
  // bitline capacitance
  double Ccell  = 4.0 * LAMBDA * Cd_pwr + 8 * MetalPitch * Cw ; 
  double Cbl    = memoryDepth * Ccell ;

  // internal capacitance
  double Ccc    = 2 * (Co + Ci) ;

  return (0.5 * Ccc * (Vdd*Vdd)) + ( Cbl ) * ( Vdd * Vdd ) * fCLK ;
}

double Power_Module::powerMemoryBitLeak(double memoryDepth ){
  
  return memoryDepth * IoffSRAM * Vdd ;
}

///////////////////////////////////////////////////////////////
//switch
//////////////////////////////////////////////////////////////

void Power_Module::calcSwitch(const SwitchMonitor* sm){

  switchArea += areaCrossbar(sm->NumInputs(), sm->NumOutputs());
  outputArea += areaOutputModule(sm->NumOutputs());
  switchPowerLeak += powerCrossbarLeak(channel_width, sm->NumInputs(), sm->NumOutputs());

  const vector<int> activity = sm->GetActivity();
  vector<double> type_activity(classes);

  for(int i = 0; i<sm->NumOutputs(); i++){
    for(int k = 0; k<classes; k++){
      type_activity[k] = 0;
    }
    for(int j = 0; j<sm->NumInputs(); j++){
      for(int k  = 0; k<classes; k++){
	double a = activity[k+classes*(i+sm->NumOutputs()*j)];
	a = a/totalTime;
	if(a>1){
	  cout<<"Switcht activity factor is greater than 1!!!\n";exit(-1);
	}
	double Px = powerCrossbar(channel_width, sm->NumInputs(),sm->NumOutputs(),j,i);
	switchPower += a*channel_width*Px;
	switchPowerCtrl += a *powerCrossbarCtrl(channel_width,  sm->NumInputs(),sm->NumOutputs());
	type_activity[k]+=a;
      }
    }
    outputPowerClk += powerWireClk( 1, channel_width ) ;
    for(int k = 0; k<classes; k++){
      outputPower += type_activity[k] * powerWireDFF( 1, channel_width, 1.0 ) ;
      outputCtrlPower += type_activity[k] * powerOutputCtrl(channel_width ) ;
    }
  }

}

double Power_Module::powerCrossbar(double width, double inputs, double outputs, double from, double to){
  // datapath traversal power
  double Wxbar = width * outputs * CrossbarPitch ;
  double Hxbar = width * inputs  * CrossbarPitch ;

  // wires
  double CwIn  = Wxbar * Cw ;
  double CwOut = Hxbar * Cw ;

  // cross-points
  double Cxi = (1.0/16.0) * CwOut ;
  double Cxo = 4.0 * Cxi * (Co_delay/Ci_delay) ;

  // drivers
  double Cti = (1.0/16.0) * CwIn ;
  double Cto = 4.0 * Cti * (Co_delay/Ci_delay) ;

  double CinputDriver = 5.0/16.0 * (1 + Co_delay/Ci_delay) * (0.5 * Cw * Wxbar + Cti) ;

  // total switched capacitance
  
  //this maybe missing +Cto
  double Cin  = CinputDriver + CwIn + Cti + (outputs * Cxi) ;
  if ( to < outputs/2 ) {
    Cin -= ( 0.5 * CwIn + outputs/2 * Cxi) ;
  }
  //this maybe missing +cti
  double Cout = CwOut + Cto + (inputs * Cxo) ;
  if ( from < inputs/2) {
    Cout -= ( 0.5 * CwOut + (inputs/2 * Cxo)) ;
  }
  return 0.5 * (Cin + Cout) * (Vdd * Vdd * fCLK) ;
}


double Power_Module::powerCrossbarCtrl(double width, double inputs, double outputs){
 
  // datapath traversal power
  double Wxbar = width * outputs * CrossbarPitch ;
  double Hxbar = width * inputs  * CrossbarPitch ;

  // wires
  double CwIn  = Wxbar * Cw ;

  // drivers
  double Cti  = (5.0/16.0) * CwIn ;

  // need some estimate of how many control wires are required
  double Cctrl  = width * Cti + (Wxbar + Hxbar) * Cw  ; 
  double Cdrive = (5.0/16.0) * (1 + Co_delay/Ci_delay) * Cctrl ;

  return (Cdrive + Cctrl) * (Vdd*Vdd) * fCLK ;
  
}

double Power_Module::powerCrossbarLeak (double width, double inputs, double outputs){
  // datapath traversal power
    double Wxbar = width * outputs * CrossbarPitch ;
    double Hxbar = width * inputs  * CrossbarPitch ;

    // wires
    double CwIn  = Wxbar * Cw ;
    double CwOut = Hxbar * Cw ;
    // cross-points
    double Cxi = (1.0/16.0) * CwOut ;
    // drivers
    double Cti  = (1.0/16.0) * CwIn ;

    return 0.5 * (IoffN + 2 * IoffP)*width*(inputs*outputs*Cxi+inputs*Cti+outputs*Cti)/Ci;
}

//////////////////////////////////////////////////////////////////
//output module
//////////////////////////////////////////////////////////////////
double Power_Module:: powerOutputCtrl(double width) {

    double Woutmod = channel_width * ChannelPitch ;
    double Cen     = Ci ;

    double Cenable = (1 + 5.0/16.0)*(1.0+Co/Ci)*(Woutmod* Cw + width* Cen) ;

    return Cenable * (Vdd*Vdd) * fCLK ;
    
}

//////////////////////////////////////////////////////////////////
//area
//////////////////////////////////////////////////////////////////

double Power_Module:: areaChannel (double K, double N, double M){

    double Adff = M * W_DFQD1 * H_DFQD1 ;
    double Ainv = M * N * ( W_INVD2 + 3 * K) * H_INVD2 ;

    return channel_width * (Adff + Ainv) * MetalPitch * MetalPitch ;
}

double Power_Module:: areaCrossbar(double Inputs, double Outputs) {
    return (Inputs * channel_width * CrossbarPitch) * (Outputs * channel_width * CrossbarPitch) ;
}

double Power_Module:: areaInputModule(double Words) {
    double Asram =  ( channel_width * H_SRAM ) * (Words * W_SRAM) ;
    return Asram * (MetalPitch * MetalPitch) ;
}

double Power_Module:: areaOutputModule(double Outputs) {
    double Adff = Outputs * W_DFQD1 * H_DFQD1 ;
    return channel_width * Adff * MetalPitch * MetalPitch ;
}

void Power_Module::run(){
  totalTime = GetSimTime();
  channelWirePower=0;
  channelClkPower=0;
  channelDFFPower=0;
  channelLeakPower=0;
  inputReadPower=0;
  inputWritePower=0;
  inputLeakagePower=0;
  switchPower=0;
  switchPowerCtrl=0;
  switchPowerLeak=0;
  outputPower=0;
  outputPowerClk=0;
  outputCtrlPower=0;
  channelArea=0;
  switchArea=0;
  inputArea=0;
  outputArea=0;
  maxInputPort = 0;
  maxOutputPort = 0;

  vector<FlitChannel *> inject = net->GetInject();
  vector<FlitChannel *> eject = net->GetEject();
  vector<FlitChannel *> chan = net->GetChannels();
  
  // Jiayi
  if (dsent == 0) {

    for(int i = 0; i<net->NumNodes(); i++){
      calcChannel(inject[i]);
    }
  
    for(int i = 0; i<net->NumNodes(); i++){
      calcChannel(eject[i]);
    }
  
    for(int i = 0; i<net->NumChannels();i++){
      calcChannel(chan[i]);
    }
  
    vector<Router*> routers = net->GetRouters();
    for(size_t i = 0; i < routers.size(); i++){
      IQRouter* temp = dynamic_cast<IQRouter*>(routers[i]);
      const BufferMonitor * bm = temp->GetBufferMonitor();
      calcBuffer(bm);
      const SwitchMonitor * sm = temp->GetSwitchMonitor();
      calcSwitch(sm);
    }
    
    double totalpower =  channelWirePower+channelClkPower+channelDFFPower+channelLeakPower+ inputReadPower+inputWritePower+inputLeakagePower+ switchPower+switchPowerCtrl+switchPowerLeak+outputPower+outputPowerClk+outputCtrlPower;
    double totalarea =  channelArea+switchArea+inputArea+outputArea;
    cout<< "-----------------------------------------\n" ;
    cout << "Booksim Embedded Power Model\n";

    cout<< "- OCN Power Summary\n" ;
    cout<< "- Completion Time:         "<<totalTime <<"\n" ;
    cout<< "- Flit Widths:            "<<channel_width<<"\n" ;
    cout<< "- Channel Wire Power:      "<<channelWirePower <<"\n" ;
    cout<< "- Channel Clock Power:     "<<channelClkPower <<"\n" ;
    cout<< "- Channel Retiming Power:  "<<channelDFFPower <<"\n" ;
    cout<< "- Channel Leakage Power:   "<<channelLeakPower <<"\n" ;
  
    cout<< "- Input Read Power:        "<<inputReadPower <<"\n" ;
    cout<< "- Input Write Power:       "<<inputWritePower <<"\n" ;
    cout<< "- Input Leakage Power:     "<<inputLeakagePower <<"\n" ;
  
    cout<< "- Switch Power:            "<<switchPower <<"\n" ;
    cout<< "- Switch Control Power:    "<<switchPowerCtrl <<"\n" ;
    cout<< "- Switch Leakage Power:    "<<switchPowerLeak <<"\n" ;
  
    cout<< "- Output DFF Power:        "<<outputPower <<"\n" ;
    cout<< "- Output Clk Power:        "<<outputPowerClk <<"\n" ;
    cout<< "- Output Control Power:    "<<outputCtrlPower <<"\n" ;
    cout<< "- Total Power:             "<<totalpower <<"\n";
    cout<< "-----------------------------------------\n" ;
    cout<< "\n" ;
    cout<< "-----------------------------------------\n" ;
    cout<< "- OCN Area Summary\n" ;
    cout<< "- Channel Area:  "<<channelArea<<"\n" ;
    cout<< "- Switch  Area:  "<<switchArea<<"\n" ;
    cout<< "- Input  Area:   "<<inputArea<<"\n" ;
    cout<< "- Output  Area:  "<<outputArea<<"\n" ;
    cout<< "- Total Area:    "<<totalarea<<endl;
    cout<< "-----------------------------------------\n" ;

  } else {

    // link power
    double link_dynamic_energy = 0;
    double link_dynamic_power = 0;
    double link_leakage_power = 0;
    // buffer power
    double buf_read_energy = 0;
    double buf_write_energy = 0;
    double buf_dynamic_power = 0;
    double buf_read_power = 0;
    double buf_write_power = 0;
    double buf_leakage_power = 0;
    // Switch allocator power
    double sw_dynamic_energy = 0;
    double sw_dynamic_power = 0;
    double sw_leakage_power = 0;
    // crossbar power
    double xbar_dynamic_energy = 0;
    double xbar_dynamic_power = 0;
    double xbar_leakage_power = 0;
    // clock power
    double clk_dynamic_power = 0;
    double clk_leakage_power = 0;    

    // accumulate network energy
    netEnergyStats &net_energy_stats = net->GetNetEnergyStats();

    // inject and eject link power
    for(int i = 0; i<net->NumNodes(); i++){
      // inject and eject channel: site-to-router link
      const FlitChannel * f_inject = inject[i];
      const FlitChannel * f_eject = eject[i];
      // activity factor for inject
      const vector<int> temp_inject = f_inject->GetActivity();
      const vector<int> temp_eject = f_eject->GetActivity();
      vector<double> a_inject(classes);
      vector<double> a_eject(classes);
      for (int j = 0; j < classes; ++j) {
        a_inject[j] = ((double)temp_inject[j]);
        a_eject[j] = ((double)temp_eject[j]);
        link_dynamic_energy += (a_inject[j] + a_eject[j]) * energy_rs_link_traversal;
      }
      link_leakage_power += rs_link_leak * 2; // one inject and one eject      
    }

    // network link power
    for (int i = 0; i < net->NumChannels(); ++i) {
      const FlitChannel * f = chan[i];
      const vector<int> temp = f->GetActivity();
      vector<double> a(classes);
      for (int j = 0; j < classes; ++j) {
        a[j] = ((double)temp[j]);
        link_dynamic_energy += a[j] * energy_rr_link_traversal;
      }
      link_leakage_power += rr_link_leak;
    }    

    // router power
    vector<Router*> routers = net->GetRouters();
    for (size_t r = 0; r < routers.size(); ++r) {
      IQRouter* temp = dynamic_cast<IQRouter*>(routers[r]);

      // buffer
      const BufferMonitor * bm = temp->GetBufferMonitor();
      const vector<int> reads = bm->GetReads();
      const vector<int> writes = bm->GetWrites();
      for (int i = 0; i < bm->NumInputs(); ++i) {
        buf_leakage_power += input_leak + (pipeline_reg0_leak + pipeline_reg1_leak) * channel_width; 
        for (int j = 0; j < classes; ++j) {
          double ar = ((double)reads[i * classes+j]);
          double aw = ((double)writes[i * classes+j]);
          buf_read_energy += ar * energy_per_buffread;
          buf_write_energy += aw * energy_per_buffwrite;
        }
      }
      
      // switch allocator and crossbar
      const SwitchMonitor * sm = temp->GetSwitchMonitor();
      sw_leakage_power += switch_leak;
      xbar_leakage_power += xbar_leak + xbar_sel_dff_leak;
      const vector<int> activity = sm->GetActivity();
      vector<double> type_activity(classes);
      for (int i = 0; i < sm->NumOutputs(); ++i) {
        xbar_leakage_power += pipeline_reg2_part_leak * channel_width;
        for (int j = 0; j < sm->NumInputs(); ++j) {
          for (int k = 0; k < classes; ++k) {
            double a = activity[k+classes*(i+sm->NumOutputs()*j)];
            sw_dynamic_energy += a * (energy_per_arbitratestage1+energy_per_arbitratestage2);
            xbar_dynamic_energy += a * energy_traverse_xbar;
          }
        }
      }

      // clock power
      clk_dynamic_power += energy_distribute_clk * frequency;
      clk_leakage_power += clk_tree_leak;
    }

    net_energy_stats.tot_time += totalTime;
    double total_run_time = net_energy_stats.tot_time / frequency;
    double kernel_run_time = totalTime / frequency;

    // accumulate network energy

    // link
    double temp = net_energy_stats.link_dynamic_energy; 
    net_energy_stats.link_dynamic_energy = link_dynamic_energy;
    net_energy_stats.link_leakage_energy += link_leakage_power * kernel_run_time;
    link_dynamic_power = (link_dynamic_energy - temp) / kernel_run_time;

    // buffer
    temp = net_energy_stats.buf_rd_energy;
    net_energy_stats.buf_rd_energy = buf_read_energy;
    buf_read_power = (buf_read_energy - temp) / kernel_run_time;

    temp = net_energy_stats.buf_wt_energy;
    net_energy_stats.buf_wt_energy = buf_write_energy;
    buf_write_power = (buf_write_energy - temp) / kernel_run_time;

    net_energy_stats.buf_leakage_energy += buf_leakage_power * kernel_run_time;

    // switch
    temp = net_energy_stats.sw_dynamic_energy;
    net_energy_stats.sw_dynamic_energy = sw_dynamic_energy;
    net_energy_stats.sw_leakage_energy += sw_leakage_power * kernel_run_time;
    sw_dynamic_power = (sw_dynamic_energy - temp) / kernel_run_time;

    // xbar
    temp = net_energy_stats.xbar_dynamic_energy;
    net_energy_stats.xbar_dynamic_energy = xbar_dynamic_energy;
    net_energy_stats.xbar_leakage_energy += xbar_leakage_power * kernel_run_time;
    xbar_dynamic_power = (xbar_dynamic_energy - temp) / kernel_run_time;

    // clk
    net_energy_stats.clk_dynamic_energy += clk_dynamic_power * kernel_run_time;
    net_energy_stats.clk_leakage_energy += clk_leakage_power * kernel_run_time;

    // router and network energy
    net_energy_stats.ComputeTotalEnergy();

    buf_dynamic_power = buf_read_power + buf_write_power;  
    double total_dynamic_power = buf_dynamic_power + sw_dynamic_power + xbar_dynamic_power + clk_dynamic_power + link_dynamic_power;
    double total_leakage_power = buf_leakage_power + sw_leakage_power + xbar_leakage_power + clk_leakage_power + link_leakage_power;
    double total_power = total_dynamic_power + total_leakage_power;


    cout<< "-----------------------------------------\n" ;
    cout << "DSENT Power Model\n";
  
    cout<< "- Kernel OCN Energy Summary\n" ;
    cout<< "- Completion Time (cycle):  "<<totalTime <<"\n" ;
    cout<< "- Flit Widths:              "<<channel_width<<"\n" ;
    cout<< "---\n"; 

    cout<< "- Link Dynamic Energy:       "<<link_dynamic_power*kernel_run_time<<"\n";
    cout<< "- Link Leakage Energy:       "<<link_leakage_power*kernel_run_time<<"\n";
    cout<< "---\n";

    cout<< "- Buffer Read Energy:        "<<buf_read_power*kernel_run_time<<"\n" ;
    cout<< "- Buffer Write Energy:       "<<buf_write_power*kernel_run_time <<"\n" ;
    cout<< "- Buffer Dynamic Energy:     "<<buf_dynamic_power*kernel_run_time <<"\n" ;
    cout<< "- Buffer Leakage Energy:     "<<buf_leakage_power*kernel_run_time <<"\n" ;
    cout<< "---\n"; 

    cout<< "- Switch Dynamic Energy:     "<<sw_dynamic_power*kernel_run_time <<"\n" ;
    cout<< "- Switch Leakage Energy:     "<<sw_leakage_power*kernel_run_time <<"\n" ;
    cout<< "---\n"; 
 
    cout<< "- Crossbar Dynamic Energy:   "<<xbar_dynamic_power*kernel_run_time <<"\n" ;
    cout<< "- Crossbar Leakage Energy:   "<<xbar_leakage_power*kernel_run_time <<"\n" ;
    cout<< "---\n"; 
 
    cout<< "- Clk Dynamic Energy:        "<<clk_dynamic_power*kernel_run_time <<"\n" ;
    cout<< "- Clk Static Energy:         "<<clk_leakage_power*kernel_run_time <<"\n" ;
    cout<< "---\n"; 

    cout<< "- NoC Power (Including links):\n";
    cout<< "- Total Dynamic Energy:      "<<total_dynamic_power*kernel_run_time <<"\n";
    cout<< "- Total Leakage Energy:      "<<total_leakage_power*kernel_run_time <<"\n";
    cout<< "- Total Energy:              "<<total_power*kernel_run_time <<"\n";
    cout<< "- Leakage Portion:          "<<total_leakage_power*100/total_power << "%\n";
    cout<< "---\n";

    cout<< "- Routers Energy (Excluding links):\n";
    cout<< "- Total Dynamic Energy:      "<<(total_dynamic_power-link_dynamic_power)*kernel_run_time <<"\n";
    cout<< "- Total Leakage Energy:      "<<(total_leakage_power-link_leakage_power)*kernel_run_time <<"\n";
    cout<< "- Total Energy:              "<<(total_power-link_dynamic_power-link_leakage_power)*kernel_run_time <<"\n";
    cout<< "- Leakage Portion:          "<<(total_leakage_power-link_leakage_power)*100/(total_power-link_dynamic_power-link_leakage_power) << "%\n";
    cout<< "-----------------------------------------\n" ;
  
    cout<< "-----------------------------------------\n" ;
    cout<< "- Application Total OCN Energy Summary\n" ;
    cout<< "- Completion Time (cycle):  "<<net_energy_stats.tot_time <<"\n" ;
    cout<< "- Flit Widths:              "<<channel_width<<"\n" ;
    cout<< "---\n"; 

    cout<< "- Link Dynamic Energy:       "<<net_energy_stats.link_dynamic_energy<<"\n";
    cout<< "- Link Leakage Energy:       "<<net_energy_stats.link_leakage_energy<<"\n";
    cout<< "---\n";

    cout<< "- Buffer Read Energy:        "<<net_energy_stats.buf_rd_energy<<"\n" ;
    cout<< "- Buffer Write Energy:       "<<net_energy_stats.buf_wt_energy<<"\n" ;
    cout<< "- Buffer Leakage Energy:     "<<net_energy_stats.buf_leakage_energy<<"\n" ;
    cout<< "---\n"; 

    cout<< "- Switch Dynamic Energy:     "<<net_energy_stats.sw_dynamic_energy<<"\n" ;
    cout<< "- Switch Leakage Energy:     "<<net_energy_stats.sw_leakage_energy<<"\n" ;
    cout<< "---\n"; 
 
    cout<< "- Crossbar Dynamic Energy:   "<<net_energy_stats.xbar_dynamic_energy<<"\n" ;
    cout<< "- Crossbar Leakage Energy:   "<<net_energy_stats.xbar_leakage_energy<<"\n" ;
    cout<< "---\n"; 
 
    cout<< "- Clk Dynamic Energy:        "<<net_energy_stats.clk_dynamic_energy<<"\n" ;
    cout<< "- Clk Static Energy:         "<<net_energy_stats.clk_leakage_energy<<"\n" ;
    cout<< "---\n"; 

    cout<< "- NoC  (Including links):\n";
    cout<< "- Total Dynamic Energy:      "<<net_energy_stats.tot_net_dynamic_energy<<"\n";
    cout<< "- Total Leakage Energy:      "<<net_energy_stats.tot_net_leakage_energy<<"\n";
    cout<< "- Total Energy:              "<<net_energy_stats.tot_net_energy<<"\n";
    cout<< "- Leakage Portion:          "<<net_energy_stats.tot_net_leakage_energy*100/net_energy_stats.tot_net_energy << "%\n";
    cout<< "---\n";

    cout<< "- Routers Power (Excluding links):\n";
    cout<< "- Total Dynamic Power:      "<<net_energy_stats.tot_rt_dynamic_energy / total_run_time <<"\n";
    cout<< "- Total Leakage Power:      "<<net_energy_stats.tot_rt_leakage_energy / total_run_time <<"\n";
    cout<< "- Total Power:              "<<net_energy_stats.tot_rt_energy / total_run_time <<"\n";
    cout<< "- Leakage Portion:          "<<net_energy_stats.tot_rt_leakage_energy*100/net_energy_stats.tot_rt_energy << "%\n";
    cout<< "-----------------------------------------\n" ;
  }




}
