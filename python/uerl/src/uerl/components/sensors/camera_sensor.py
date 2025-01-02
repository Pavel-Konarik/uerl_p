from __future__ import annotations
from uerl.components.component_base import SensorBase, SensorConfig
from collections import deque
from copy import deepcopy
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from dataclasses import dataclass
from PIL import Image



class CameraSensor(SensorBase):
    def __init__(self, config: CameraSensorConfig) -> None:
        super().__init__(config)

        # Config variables
        self.config: CameraSensorConfig = deepcopy(config) # type: ignore

        # Latest frame for debug rendering
        self.latest_frame: np.ndarray | None = None

        # Non config variables
        self.frames = deque(maxlen=self.config.frame_stack_size)

    def reset(self):
        self.frames.clear()

    def process_obs(self, obs: bytearray) -> np.ndarray:
        # We are transfering alpha as well, which we don't really need for now
        incomming_channels = 1 if self.config.grayscale else 4
        frame_np = np.frombuffer(obs, dtype=np.uint8).reshape(
            (self.config.width, self.config.height, incomming_channels))
        # Cut off the alpha channel and reorder from BGR to RGB
        if not self.config.grayscale:
            frame_np = frame_np[:, :, :3][:, :, ::-1]

        self.latest_frame = frame_np

        # Frame stacking
        if self.frames.maxlen and len(self.frames) != self.frames.maxlen:
            # First observation, fill buffer
            for _ in range(self.frames.maxlen):
                self.frames.append(frame_np)
        else:
            # Normal Step
            self.frames.append(frame_np)
        return np.concatenate(list(self.frames), axis=-1)



    def get_debug_image(self) -> Image.Image | None:
        """ Returns image to include in render """
        out_image = None
        if self.latest_frame is None:
            out_image = Image.fromarray(np.zeros((self.config.width, self.config.height, 3), dtype=np.uint8))
        else:
            out_image = Image.fromarray(self.latest_frame)

        return out_image
    


@dataclass
class CameraSensorConfig(SensorConfig):
    attach_to_comp_name: str
    attach_to_actor_name: str = "" # Attach to external actor
    width: int = 72
    height: int = 72
    max_distance: float = 15000.0
    fov: float = 90.0
    use_gpu: bool = False
    grayscale: bool = False
    render_owner: bool = True
    srgb: bool = False
    frame_stack_size: int = 1
    normalise: bool = False
    # Mandatory parent config
    python_class = CameraSensor
    ue_class_name = "camera"