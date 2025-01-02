from __future__ import annotations
from uerl.components.component_base import SensorBase, SensorConfig
from collections import deque
from copy import deepcopy
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from dataclasses import dataclass, field
from PIL import Image
from uerl.types import FRotator, FVector

class VelocitySensor(SensorBase):
    def __init__(self, config: VelocitySensorConfig) -> None:
        super().__init__(config)

        # Config variables
        self.config: VelocitySensorConfig = deepcopy(config) # type: ignore

    def reset(self):
        pass

    def process_obs(self, obs: bytearray) -> np.ndarray:
        velocity = np.frombuffer(obs, dtype=np.float32).reshape((3,))
        return velocity


    


@dataclass
class VelocitySensorConfig(SensorConfig):
    should_normalise: bool = True
    zero_centered: bool = True # If to use range [-1, 1] or [0, 1] (only if should_normalise is True)
    max_velocity:float = 500.0 # Used to normalize the velocity values
    max_velocity_from_cmc:bool = True # If True max_velocity is ignored and the max velocity is taken from the CMC

    # Mandatory parent config
    python_class = VelocitySensor
    ue_class_name = "velocity_sensor"