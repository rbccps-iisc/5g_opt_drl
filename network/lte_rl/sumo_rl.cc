/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Piotr Gawlowicz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include "ns3/core-module.h"
#include "ns3/opengym-module.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"


#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traci-applications-module.h"
#include "ns3/network-module.h"
#include "ns3/traci-module.h"
#include "ns3/wave-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/netanim-module.h"
#include <functional>
#include <stdlib.h>

#include<string>
#include "sumo-TraCIAPI.h"


#include<math.h>
#include<map>
#include<string.h>
using namespace ns3;

//Defining lteHelper to be global so that ExecuteActions function can access this
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

//Action Map

std::map<int,double > power_map ={{0,30},{1,40},{2,50},{3,60}};

double value[6]; // 6 because Lane ID are 6 length strings

// Serving BS CellID
int cell_id;

// Defining reward to be global so that it can be accessed by GetReward function
double reward;

//Defining the enbLteDevs to be global, so that their transmit power can be varied in 'ExecuteActions' function
NetDeviceContainer enbLteDevs;
NetDeviceContainer ueLteDevs;

NS_LOG_COMPONENT_DEFINE ("OpenGym");


//Define observation space

Ptr<OpenGymSpace> MyGetObservationSpace(void)
{
 
  uint32_t  parameters = 9; // 2 Received signal powers and 6 for lane ID encoding -- 6 characters, Attached Base Station -- 2 values
  float low =  -100;
  float high = 100;
  std::vector<uint32_t> shape = {parameters,};
  std::string dtype = TypeNameGet<double> ();
  
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  
  NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
  return space;
}


//Define action space

Ptr<OpenGymSpace> MyGetActionSpace(void)
{
  
  uint32_t power_lev = 4;
  
  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (power_lev); 
  
  NS_LOG_UNCOND ("MyGetActionSpace: " << space);
  return space;
}

//Define game over condition

bool MyGetGameOver(void)
{

  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

//Calculating Distance to Base Station

double get_value(uint32_t i,uint32_t b){

        //Nodes are instatntiated such that first N base stations are created, and then 1 UE.
        Ptr<Node> bn =  ns3::NodeList::GetNode(4+i); 
        Ptr<Node> un =  ns3::NodeList::GetNode(6);
        
        Ptr<MobilityModel> b0 = bn->GetObject<MobilityModel> ();
        Ptr<MobilityModel> u0 = un->GetObject<MobilityModel> ();
       
        Vector bs = b0->GetPosition();
        
        Vector ue = u0->GetPosition();
        
        double d = pow(pow(bs.x - ue.x, 2) + pow(bs.y - ue.y, 2) +pow(bs.z - ue.z, 2) ,0.5);
        double loss = 10 * 3.9 * log10 (d);
        
        double rec_p = 30 - 38.57 -loss;
        
        return rec_p;        
}

  
//Collect observations  

Ptr<OpenGymDataContainer> MyGetObservation(void)
{
  
  uint32_t parameters = 9;
  uint32_t bs =2;
  

  std::vector<uint32_t> shape = {parameters,};
  Ptr<OpenGymBoxContainer<double> > box = CreateObject<OpenGymBoxContainer<double> >(shape);
  
  //Received Signal Power
  for (uint32_t i = 0; i<bs; i++){
        
        double val = get_value(i,bs);
        box->AddValue(val);
  }
  
  //Serving BS CellID
  box->AddValue(cell_id);
  
  //Lane ID
  for (uint32_t i = 0; i < 6; i++){ // 6 because, 2 values are RSP, 1 is Cell Id of Attached BS. Hence 6 remain out of 9
        
        box->AddValue(value[i]);
  }

     
  
  NS_LOG_UNCOND ("MyGetObservation: " << box);
  return box;
}

//Define reward function

float MyGetReward(void)
{

  return reward;
}

//Define extra info. Optional

std::string MyGetExtraInfo(void)
{
  std::string myInfo = "testInfo";
  myInfo += "|123";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}

//Execute received actions


bool MyExecuteActions(Ptr<OpenGymDataContainer> action)
{

  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  uint32_t bs = discrete->GetValue();
  
  Ptr<LteEnbNetDevice> lteEnbDev1 = enbLteDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy1 = lteEnbDev1->GetPhy ();
  
  enbPhy1->SetAttribute ("TxPower", DoubleValue (power_map[bs]));
  
  Ptr<LteEnbNetDevice> lteEnbDev2 = enbLteDevs.Get (1)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy2 = lteEnbDev2->GetPhy ();
  
  enbPhy2->SetAttribute ("TxPower", DoubleValue (power_map[bs]));
  
  NS_LOG_UNCOND ("MyExecuteActions: " << action);
  return true;
}

void ScheduleNextStateRead(double envStepTime, Ptr<TraciClient> p, Ptr<OpenGymInterface> openGym, ApplicationContainer serverapps,uint32_t prev,bool firstwrite)
{
      //For each environment step, throughput is calculated to be returned as 'reward' .

      int interval =envStepTime; // keep this same as envStepTime
      uint32_t totalPacketsThrough = DynamicCast<UdpServer>(serverapps.Get(0))->GetReceived ();
      uint32_t recent = totalPacketsThrough-prev;
      double throughput=recent*1500*8/(interval*1000000.0);
      
      reward = throughput;
      
      std::vector<std::string> idlist = p->vehicle.getIDList();
      auto itr = idlist.begin();     
      		  		 
      std::string str = p->vehicle.getLaneID(*itr);
      int n = str.length();
      
      for( int j =0; j < n; j++)
                
                value[j] = (int) str[j];
    
      //Get the Serving BS CellID
      Ptr<LteUeNetDevice> lteUeDev = ueLteDevs.Get(0)->GetObject<LteUeNetDevice> ();
      cell_id = lteUeDev->GetRrc()->GetCellId();
      
      if(firstwrite == true){
      
      		 std::ofstream file("throughput_rl.txt",std::ios_base::out);
      		 file<<p->simulation.getTime()<<" "<<str<<" "<<throughput<<"\n";
      		 firstwrite =false;  
      }
      
      else{
      		 std::ofstream file("throughput_rl.txt",std::ios_base::app); 
      		 file<<p->simulation.getTime()<<" "<<str<<" "<<throughput<<"\n";
      }
      Simulator::Schedule (Seconds(envStepTime), &ScheduleNextStateRead, envStepTime, p, openGym, serverapps,totalPacketsThrough,firstwrite);
      openGym->NotifyCurrentState();
}

int
main (int argc, char *argv[])
{
  // Parameters of the scenario
  uint32_t simSeed = 1;
  double simulationTime = 60; //seconds
  double envStepTime = 2; //seconds, ns3gym env step time interval
  uint32_t openGymPort = 5555;
  uint32_t testArg = 0;

  CommandLine cmd;
  // required parameters for OpenGym interface
  cmd.AddValue ("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
  cmd.AddValue ("simSeed", "Seed for random generator. Default: 1", simSeed);
  // optional parameters
  cmd.AddValue ("simTime", "Simulation time in seconds. Default: 10s", simulationTime);
  cmd.AddValue ("testArg", "Extra simulation argument. Default: 0", testArg);
  cmd.Parse (argc, argv);

  NS_LOG_UNCOND("Ns3Env parameters:");
  NS_LOG_UNCOND("--simulationTime: " << simulationTime);
  NS_LOG_UNCOND("--openGymPort: " << openGymPort);
  NS_LOG_UNCOND("--envStepTime: " << envStepTime);
  NS_LOG_UNCOND("--seed: " << simSeed);
  NS_LOG_UNCOND("--testArg: " << testArg);

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (simSeed);

  // Lte Part ---------------------------------------------------------------------------------------------
  
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

// Create a single RemoteHost

NodeContainer remoteHostContainer;
remoteHostContainer.Create (1);
Ptr<Node> remoteHost = remoteHostContainer.Get (0);
InternetStackHelper internet;
internet.Install (remoteHostContainer);

// Create the internet

PointToPointHelper p2ph;
p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
Ipv4AddressHelper ipv4h;
ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

// interface 0 is localhost, 1 is the p2p device
//Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


Ipv4StaticRoutingHelper ipv4RoutingHelper;
Ptr<Ipv4StaticRouting> remoteHostStaticRouting;
remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"),Ipv4Mask ("255.255.0.0"), 1);

// Path Loss Model
lteHelper->SetPathlossModelType (TypeId::LookupByName ("ns3::LogDistancePropagationLossModel"));
lteHelper->SetPathlossModelAttribute ("Exponent", DoubleValue (3.9));
lteHelper->SetPathlossModelAttribute ("ReferenceLoss", DoubleValue (38.57)); //ref. loss in dB at 1m for 2.025GHz
lteHelper->SetPathlossModelAttribute ("ReferenceDistance", DoubleValue (1));

//Handover Type
lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");

int n_bs =2;
int n_ue =1;

NodeContainer enbNodes;
NodeContainer ueNodes;

uint32_t nodeCounter (0);

enbNodes.Create (n_bs);
ueNodes.Create (n_ue);

// Install Mobility Model--------------------------------------------------------

MobilityHelper mobility;

Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
positionAlloc->Add(Vector(100,0,0));
mobility.SetPositionAllocator(positionAlloc);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (ueNodes);

MobilityHelper enb_mobility;

Ptr<ListPositionAllocator> enbpositionAlloc = CreateObject<ListPositionAllocator> ();
enbpositionAlloc->Add(Vector(600,400,0));
enbpositionAlloc->Add(Vector(900,400,0));
enb_mobility.SetPositionAllocator(enbpositionAlloc);
enb_mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
enb_mobility.Install(enbNodes);
//-------------------------------------------------------------------------------
 
// Install LTE Devices to the nodes
enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

// we install the IP stack on the UEs
//InternetStackHelper internet;
internet.Install (ueNodes);

// assign IP address to UEs
ApplicationContainer serverApps;
ApplicationContainer clientApps;

uint16_t dlPort = 1234;
for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    Ptr<NetDevice> ueLteDevice = ueLteDevs.Get (u);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevice));
    // set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting;
    ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    
    
    //PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), dlPort));
    UdpServerHelper packetSinkHelper(dlPort);
    serverApps.Add(packetSinkHelper.Install (ueNodes.Get (u)));
    serverApps.Start (Seconds (0.01));
    
    UdpClientHelper client (ueIpIface.GetAddress (0), dlPort);
    client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client.SetAttribute ("Interval", TimeValue( Seconds(0.001)));
    clientApps.Add(client.Install (remoteHost));
    clientApps.Start (Seconds (0.01));
    
    dlPort++;
  }

  // Set transmit power------------------------------------------------------------------
  
  Ptr<LteEnbNetDevice> lteEnbDev = enbLteDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  
  enbPhy->SetAttribute ("TxPower", DoubleValue (30.0));
  enbPhy->SetAttribute ("NoiseFigure", DoubleValue (1.0));
  
  Ptr<LteEnbNetDevice> lteEnbDev1 = enbLteDevs.Get (1)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy1 = lteEnbDev1->GetPhy ();
  
  enbPhy1->SetAttribute ("TxPower", DoubleValue (30.0));
  enbPhy1->SetAttribute ("NoiseFigure", DoubleValue (1.0));

  //--------------------------------------------------------------------------------------
  
      
  // Add X2 interface
  lteHelper->AddX2Interface (enbNodes);
  std::cout<<"X2 Interface Added\n";
  
  // Attach
  lteHelper->Attach (ueLteDevs, enbLteDevs.Get (0)); 

  // X2-based Handover
  //lteHelper->HandoverRequest (Seconds (15), ueLteDevs.Get (0), enbLteDevs.Get (0), enbLteDevs.Get (1));

  //Automatic Handover
  lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (20));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (1));
 
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.25)));
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.25)));


  //-------------------------------------------------------------------------------------------------------
  
  
  // OpenGym Env-------------------------------------------------------------------------------------------
  
  Ptr<OpenGymInterface> openGym = CreateObject<OpenGymInterface> (openGymPort);
  openGym->SetGetActionSpaceCb( MakeCallback (&MyGetActionSpace) );
  openGym->SetGetObservationSpaceCb( MakeCallback (&MyGetObservationSpace) );
  openGym->SetGetGameOverCb( MakeCallback (&MyGetGameOver) );
  openGym->SetGetObservationCb( MakeCallback (&MyGetObservation) );
  openGym->SetGetRewardCb( MakeCallback (&MyGetReward) );
  openGym->SetGetExtraInfoCb( MakeCallback (&MyGetExtraInfo) );
  openGym->SetExecuteActionsCb( MakeCallback (&MyExecuteActions) );
  
  //--------------------------------------------------------------------------------------------------------
  
  //--------SUMO Setup---------------------------------------------------------------------------------------------

  Ptr<TraciClient> sumoClient = CreateObject<TraciClient> ();
  sumoClient->SetAttribute ("SumoConfigPath", StringValue ("sim.sumocfg"));
  sumoClient->SetAttribute ("SumoBinaryPath", StringValue (""));    // use system installation of sumo
  sumoClient->SetAttribute ("SynchInterval", TimeValue (Seconds (0.1)));
  sumoClient->SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
  sumoClient->SetAttribute ("SumoGUI", BooleanValue (true));
  sumoClient->SetAttribute ("SumoPort", UintegerValue (3400));
  sumoClient->SetAttribute ("PenetrationRate", DoubleValue (1.0));  // portion of vehicles equipped with wifi
  sumoClient->SetAttribute ("SumoLogFile", BooleanValue (true));
  sumoClient->SetAttribute ("SumoStepLog", BooleanValue (false));
  sumoClient->SetAttribute ("SumoSeed", IntegerValue (10));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--verbose true"));
  sumoClient->SetAttribute ("SumoWaitForSocket", TimeValue (Seconds (1.0)));

  
//----------------------------------------------------------------------------------------------------------------
// callback function for node creation
  std::function<Ptr<Node> ()> setupNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= ueNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created."<< ueNodes.GetN());

      // don't create and install the protocol stack of the node at simulation time -> take from "node pool"
      Ptr<Node> includedNode = ueNodes.Get(nodeCounter);
      ++nodeCounter;// increment counter for next node

      return includedNode;
    };

  // callback function for node shutdown
  std::function<void (Ptr<Node>)> shutdownNode = [] (Ptr<Node> exNode)
    {

      // set position outside communication range
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-100.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      // NOTE: further actions could be required for a save shut down!
    };

  // start traci client with given function pointers
  sumoClient->SumoSetup (setupNode, shutdownNode);
  
  //----------------------------------------------------------------------------------------------------------------------
  AnimationInterface anim ("lena_sumo.xml"); // Mandatory
  
  Simulator::Schedule (Seconds(2), &ScheduleNextStateRead, envStepTime, sumoClient, openGym, serverApps,0,true); //key to understand timings of agent and ns3 environment.

  NS_LOG_UNCOND ("Simulation start");
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  NS_LOG_UNCOND ("Simulation stop");
      
  openGym->NotifySimulationEnd();
  Simulator::Destroy ();

}
