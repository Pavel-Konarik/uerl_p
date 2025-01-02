from enum import Enum, unique
from typing import Set, Dict
from typing import Optional, Type, Dict, Union, List, Tuple, Any
from dataclasses import asdict, dataclass, field


@unique
class ERLEnvCommandType(Enum):
    NoAction = 0
    ConfigureRequest = 1
    ConfigureOutcome = 2
    ResetRequest = 3
    ResetOutcome = 4
    StepRequest = 5
    StepOutcome = 6


@dataclass
class ResetRequest_Data():
    seed: int
    options: Dict[str, str]


@dataclass
class ActuatorData:
    Data: bytes


@dataclass
class AgentActuators:
    Actuators: Dict[str, ActuatorData] = field(default_factory=dict)


@dataclass
class Action_StepRequest:
    Agents: Dict[str, AgentActuators] = field(default_factory=dict)


def send_to_ue(ws, bytes: bytes, command_type: ERLEnvCommandType) -> None:
    command_byte = command_type.value.to_bytes(1, 'big')
    ws.send(command_byte + bytes)