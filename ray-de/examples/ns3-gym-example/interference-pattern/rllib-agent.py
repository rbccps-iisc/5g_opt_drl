from tqdm import tqdm
import gym
from RayRLLibNS3Env import RayRLLibNS3Env

import torch
import ray
from ray.tune.logger import pretty_print

import yaml
from copy import deepcopy
from deepmerge import Merger
from importlib import import_module


with open("run_config.yaml", "r") as f:
    config = yaml.safe_load(f)

ray.init(**config["ray_config"])

merger = Merger(
    [(dict, ["merge"])], # list of tuples of type and their merge strategies
    ["override"], # fallback merge strategies for all other types
    ["override"], # merge strategies in case of type conflicts
)

agent_name = config["agent_config"].pop("agent")
agent_module, agent_trainer = agent_name.split(".")
agent_module = import_module(f"ray.rllib.agents.{agent_module}")

agent_trainer = agent_module.__dict__[agent_trainer]
agent_config = deepcopy(agent_module.DEFAULT_CONFIG)
agent_config = merger.merge(agent_config, config["agent_config"])

agent = agent_trainer(env=RayRLLibNS3Env, config=agent_config)

for i in tqdm(range(config["other_config"]["num_train_iter"])):
    result = agent.train()
    if (i+1)%config["other_config"]["save_freq"] == 0:
        print(pretty_print(result))

        checkpoint_path = agent.save(config["other_config"]["save_dir"])
        print(checkpoint_path)

agent.cleanup()


print("Evaluating on an environment run")
env = RayRLLibNS3Env()
agent = agent_trainer(env=RayRLLibNS3Env, config=agent_config)
agent.restore(checkpoint_path)

state = env.reset()
# env.render()
done = False
cumulative_reward = 0

while not done:
    action = agent.compute_single_action(state)
    state, reward, done, info = env.step(action)
    # env.render()
    cumulative_reward += reward

print("Cumulative reward achieved:", cumulative_reward)
env.close()