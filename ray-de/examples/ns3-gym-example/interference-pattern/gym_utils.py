import numpy as np
from collections import OrderedDict
import gym
from gym import spaces

def change_space_dtype(
    space, 
    dtype_map=dict(uint="int", uint32="int32", uint64="int64"),
):
    if isinstance(space, spaces.Box):
        space = spaces.Box(
            low=space.low, 
            high=space.high, 
            dtype=dtype_map.get(space.dtype.name, space.dtype),
        )

    elif isinstance(space, spaces.Dict):
        space = spaces.Dict(
            {k: change_space_dtype(v, dtype_map) for k,v in space.items()}
        )

    elif isinstance(space, spaces.Tuple):
        space = spaces.Tuple((change_space_dtype(i, dtype_map) for i in space))

    elif isinstance(space, spaces.MultiDiscrete):
        space = spaces.MultiDiscrete(
            nvec=space.nvec, 
            dtype=dtype_map.get(space.dtype.name, space.dtype),
        )

    return space


def to_space_dtype(sample, space):
    if sample is not None:
        
        if isinstance(space, (spaces.Box, spaces.MultiDiscrete)):
            sample = np.array(sample, dtype=space.dtype)
        
        elif isinstance(space, spaces.Dict):
            sample = OrderedDict(
                (k, to_space_dtype(v, space.spaces[k])) for k,v in sample.items()
            )

        elif isinstance(space, spaces.Tuple):
            sample = tuple(
                to_space_dtype(j, space.spaces[i]) for i,j in enumerate(sample)
            )

    return sample