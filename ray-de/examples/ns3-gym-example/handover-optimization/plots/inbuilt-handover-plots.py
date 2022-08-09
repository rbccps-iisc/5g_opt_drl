import re
from pathlib import Path
from sys import argv

import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns

# first cmd arg is the Handover (HO) log file
with open(argv[1], "r+") as f:
    text = f.read()

pattern = re.compile("(?<=/NodeList/)\d+(?=/.*/LteUe)")
ue_nodes = set(int(i) for i in re.findall(pattern, text))

pattern = re.compile("(?<=/NodeList/)\d+(?=/.*/LteEnb)")
enb_nodes = set(int(i) for i in re.findall(pattern, text))

time_and_state_pattern = re.compile("Time[:\-\.\w\d\s\n]*")
time_and_state = re.findall(time_and_state_pattern, text)

def get_time_rsrp_rsrq_serving_cell(time_and_state, imsi=1):
    time_pattern = re.compile("(?<=Time:\n).*")
    state_pattern = re.compile("(?<=State:\n).*", re.DOTALL)

    time, rsrp, rsrq, serving_cell = [], [], [], []
    for ts in time_and_state:
        t = [float(i) for i in re.search(time_pattern, ts).group().split()]
        if (not time and t[imsi-1] != 0) or (time and t[imsi-1] != time[-1]):
            time.append(t[imsi-1])
            states = re.search(state_pattern, ts).group().strip().split("\n")
            state = [int(i) for i in states[imsi-1].split()]
            
            serving_cell.append(state.pop())
            rsrp.append(state[:len(state)//2])
            rsrq.append(state[len(state)//2:])

    return np.array(time), np.array(rsrp), np.array(rsrq), np.array(serving_cell)

def plot_states_over_time(time, rsrp, rsrq, serving_cell, imsi=1, save_path=None):
    fig, ax = plt.subplots()
    labels = []
    for state_name, state in (("rsrp",rsrp), ("rsrq",rsrq)):
        for i in range(state.shape[1]):
            sns.lineplot(x=time, y=state[:,i])
            labels.append(f"{state_name}-Enb-{i+1}")

    sns.lineplot(x=time, y=serving_cell)
    labels.append("Serving-Cell")

    handover_idcs = np.where(np.diff(serving_cell, prepend=serving_cell[0]))[0]
    for idx in handover_idcs:
        plt.axvline(x=time[idx], linestyle="dashed")
    
    ax.legend(labels=labels)
    ax.set_xlabel("Simulation Time (s)")
    ax.set_title(f"State Values of UE (imsi={imsi}) vs Simulation Time (s)")

    if save_path:
        fig.savefig(save_path)
    return fig, ax

def get_and_plot_states(time_and_state, imsi=1, algo="A3-rsrp", plot=True):
    time, rsrp, rsrq, serving_cell = get_time_rsrp_rsrq_serving_cell(
        time_and_state, imsi,
        )
    
    if plot:
        fig, ax = plot_states_over_time(
            time, rsrp, rsrq, serving_cell, imsi=imsi, 
            save_path=f"{algo}-states-UE-imsi-{imsi}.png",
        )
        return time, rsrp, rsrq, serving_cell, fig, ax
    return time, rsrp, rsrq, serving_cell

# extract algo name from path assuming form e.g. "A2A4-rsrq-handover.txt"
algo="-".join(Path(argv[1]).name.split("-",2)[:2])
for i in range(1, len(ue_nodes)+1):
    get_and_plot_states(time_and_state, imsi=i, algo=algo)