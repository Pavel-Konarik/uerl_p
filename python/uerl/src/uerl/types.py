from enum import Enum
from dataclasses import dataclass


class RenderType(Enum):
    NO_RENDER = 1
    GPU_ON_SCREEN = 2
    GPU_OFF_SCREEN = 3


@dataclass
class FVector():
    x: float
    y: float
    z: float


@dataclass
class FRotator():
    roll: float = 0.0
    yaw: float = 0.0
    pitch: float = 0.0