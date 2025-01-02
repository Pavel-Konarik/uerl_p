from __future__ import annotations
import abc
from dataclasses import dataclass, field
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from uerl.components.component_base import ActuatorBase, ActuatorConfig


class DroneMovementActuator(ActuatorBase):
    def __init__(self, config: DroneMovementActuatorConfig) -> None:
        super().__init__(config)


@dataclass
class DroneMovementActuatorConfig(ActuatorConfig):
    # Add Drone Movement specific configs here
    # Mandatory parent config
    python_class = DroneMovementActuator
    ue_class_name = "drone_movement"
