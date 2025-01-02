from __future__ import annotations
import abc
from dataclasses import dataclass, field
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from uerl.components.component_base import ActuatorBase, ActuatorConfig


class RobotMovementActuator(ActuatorBase):
    def __init__(self, config: RobotMovementActuatorConfig) -> None:
        super().__init__(config)


@dataclass
class RobotMovementActuatorConfig(ActuatorConfig):
    # Add Robot Movement specific configs here
    # Mandatory parent config
    python_class = RobotMovementActuator
    ue_class_name = "robot_movement"
