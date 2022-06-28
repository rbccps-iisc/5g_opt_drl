import pandas as pd
import numpy as np
import sumolib
import mchmm._mc as mc
import mchmm._hmm as hmm
import os,sys
import sysv_ipc


if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:   
    sys.exit("please declare environment variable 'SUMO_HOME'")

net = sumolib.net.readNet('grid.net.xml')

for route in sumolib.xml.parse_fast("grid.rou.xml", 'route', ['edges']):
    edge_ids = route.edges.split()

obs_space= edge_ids
new_obs =['B1C1', 'B2B1', 'B2A2', 'A2A1', 'A1B1', 'B1C1', 'C1C0', 'C0B0', 'B0A0','B2B1', 'B1C1', 'C1C0', 'C0B0', 'B0B1', 'B1C1', 'B2B1', 'B2A2', 'A2A1', 'A1B1', 'B1C1', 'C1C0', 'C0B0', 'B0A0']
theta = hmm.HiddenMarkovModel().from_baum_welch(new_obs, obs=obs_space, states=[0,1],pi=[])

