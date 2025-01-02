from __future__ import annotations
import abc
from dataclasses import dataclass, field
from typing import Generic, Optional, Type, Dict, TypeVar, Union, List, Tuple, Any
import numpy as np
from PIL import Image

T = TypeVar('T', bound='AgentComponentBase')
C = TypeVar('C', bound='AgentComponentConfig')

class AgentComponentBase(abc.ABC, Generic[C]):
    def __init__(self, config: C) -> None:
        super().__init__()
        self.config: C = config

    @property
    def name(self) -> str:
        return self.config.name

    @property
    def python_class(self) -> Type[AgentComponentBase]:
        return self.config.python_class

    @property
    def ue_class_name(self) -> str:
        return self.config.ue_class_name

    def reset(self) -> None:
        """Triggered upon resetting environment before observation."""
        pass

    def get_debug_texts(self) -> tuple[str, ...]:
        """ Returns list of images to include in render and list of debug texts """
        return ()

    def get_debug_image(self) -> Image.Image | None:
        """ Returns image to include in render """
        return None
    


class SensorBase(AgentComponentBase['SensorConfig']):
    def __init__(self, config: SensorConfig) -> None:
        super().__init__(config)

    @abc.abstractmethod
    def process_obs(self, obs: bytearray) -> np.ndarray:
        """Take a bytearray from UE counterpart as input and return a numpy array."""
        pass


class ActuatorBase(AgentComponentBase['ActuatorConfig']):
    def __init__(self, config: ActuatorConfig) -> None:
        super().__init__(config)


class RewarderBase(AgentComponentBase['RewarderConfig']):
    def __init__(self, config: RewarderConfig) -> None:
        super().__init__(config)

    def process_reward(self, reward: float) -> float:
        """Optionally modify reward that came from UE."""
        return reward


class TerminatorBase(AgentComponentBase['TerminatorConfig']):
    def __init__(self, config: TerminatorConfig) -> None:
        super().__init__(config)

    def process_termination(self, termination: Dict) -> tuple[bool, bool]:
        """Optionally modify termination and truncation that came from UE."""
        return termination["Terminated"], termination["Truncated"]

class InfoProviderBase(AgentComponentBase):
    def __init__(self, config: AgentComponentConfig) -> None:
        """ We leave the config as a generic AgentComponentConfig, as we don't need to access any specific fields.
        This allows us to use sensors, such as camera as info providers. """
        super().__init__(config)

    def process_info(self, info: bytearray) -> Any:
        """Take a bytearray from UE counterpart as input and return any processed info."""
        return info


@dataclass
class AgentComponentConfig(Generic[T]):
    name: str
    python_class: Type[T] = field(default=None, init=False) # type: ignore Always overridden by subclass
    ue_class_name: str = field(default="unset_class", init=False)

    def instantiate(self) -> T:
        return self.python_class(self)


@dataclass
class SensorConfig(AgentComponentConfig[SensorBase]):
    pass


@dataclass
class ActuatorConfig(AgentComponentConfig[ActuatorBase]):
    pass


@dataclass
class RewarderConfig(AgentComponentConfig[RewarderBase]):
    pass


@dataclass
class TerminatorConfig(AgentComponentConfig[TerminatorBase]):
    pass


@dataclass
class InfoProviderConfig(AgentComponentConfig):
    pass
