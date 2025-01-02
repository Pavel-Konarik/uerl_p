from __future__ import annotations
from uerl.components.component_base import SensorBase, SensorConfig
from copy import deepcopy
from dataclasses import dataclass
import numpy as np
from uerl.types import FVector


class LocationSensor(SensorBase):
    def __init__(self, config: LocationSensorConfig) -> None:
        super().__init__(config)

        # Config variables
        self.config: LocationSensorConfig = deepcopy(config) # type: ignore

    def process_obs(self, obs: bytearray) -> np.ndarray:
        return np.frombuffer(obs, dtype=np.float32)

@dataclass
class LocationSensorConfig(SensorConfig):
    normalise: bool
    minimum_location: FVector = FVector(0, 0, 0)
    maximum_location: FVector = FVector(0, 0, 0)
    # Mandatory parent config
    python_class = LocationSensor
    ue_class_name = "location"