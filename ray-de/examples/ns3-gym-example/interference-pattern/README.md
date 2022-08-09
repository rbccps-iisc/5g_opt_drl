# Integrating and using a custom ns3-gym environment with ray\[rllib\]

This is an integration example of a ns3-gym environment so that it can be used with ray\[rllib\]. It assumes that the ns3 with opengym (e.g. [dev-version](https://github.com/tkn-tub/ns3-gym)) is installed at `$NS3_INSTALLED_DIR` e.g. at `~/Documents/installs/ns3` and the [ray-de requirements](../../requirements.txt) are already installed. The ns3-gym environment files namely:

1. mygym.h
2. mygym.cc
3. sim.cc

are copied "as is" from the [interference-pattern](https://github.com/tkn-tub/ns3-gym/tree/master/scratch/interference-pattern) example provided in the [ns3-gym github repo](https://github.com/tkn-tub/ns3-gym). *For a different custom environment, we will be required to write at least these files with our custom logic, while the ray\[rllib\] wrapper and helper files should be copied and used as it is.*

The required ray\[rllib\] wrapper and helper files provided here are:

1. update-ray-core.py
2. sample_batch.py.replace
3. gym_utils.py
4. RayRLLibNS3Env.py
5. run_config.yaml
6. rllib-agent.py

To run this example, copy this directory inside the ns3-gym scratch directory:
```
cp -r ../interference-pattern $NS3_INSTALLED_DIR/scratch/interference-pattern
cd $NS3_INSTALLED_DIR/scratch/interference-pattern
```

After copying and moving in the `interference-pattern` directory execute the following command to update the sample_batch.py module of the  ray\[rllib\] package (version 1.12.1 as mentioned in the requirements.txt):

```
python3 update-ray-core.py
```

The above execution is required only once, even for running a different environment from a different example directory but it doesn't harm to run it multiple times, so it can be run safely as many times as possible.

Ideally the ray modules should not be manually changed and required modification should be done at the user side which is being currently worked upon and hence the manual ray code update is likely a temporary hack.

Once the above setups are done, we can modify the `run_config.yaml` file with the required parameters including agent configurations such as sac.SACTrainer agent for continuous action space, num_workers, details of the model architectures etc. or other configurations such as num_train_iter, save_dir and save_freq etc.

To run the example with the current config, execute:

```
python3 rllib-agent.py
```

which compiles the ns3-gym environment (only for the first time or when there is a change in the code of the environment), trains a PPO policy for the interference-pattern problem for 2000 training iterations and then evaluates it on a single environment run achieving a cumulative reward of \~98 on an average.

To explore the various algorithms and their configs to be specified as provided by the ray\[rllib\], please take a look at the [algorithms page](https://docs.ray.io/en/latest/rllib/rllib-algorithms.html).