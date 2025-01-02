from __future__ import annotations
from uerl.components.component_base import SensorBase, SensorConfig
from copy import deepcopy
from dataclasses import dataclass
import numpy as np


class RotationSensor(SensorBase):
    def __init__(self, config: RotationSensorConfig) -> None:
        super().__init__(config)

        # Config variables
        self.config: RotationSensorConfig = deepcopy(config) # type: ignore

    def process_obs(self, obs: bytearray) -> np.ndarray:
        return np.frombuffer(obs, dtype=np.float32)
    
@dataclass
class RotationSensorConfig(SensorConfig):
    cyclic: bool = True
    only_z: bool = True
    zero_centered: bool = False
    # Mandatory parent config
    python_class = RotationSensor
    ue_class_name = "rotation"
