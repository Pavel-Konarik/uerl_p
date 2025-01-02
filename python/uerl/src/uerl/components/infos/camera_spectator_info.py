from __future__ import annotations
from uerl.components.component_base import InfoProviderBase, InfoProviderConfig
from collections import deque
from copy import deepcopy
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from dataclasses import dataclass
from PIL import Image
from uerl.components.sensors.camera_sensor import CameraSensor, CameraSensorConfig

class SpectatorCameraInfoProvider(InfoProviderBase):
    def __init__(self, config: SpectatorCameraConfig) -> None:
        super().__init__(config)
        
        self.camera_sensor = CameraSensor(config)

    def reset(self):
        self.camera_sensor.reset()

    def process_info(self, info: bytearray) -> np.ndarray:
        return self.camera_sensor.process_obs(info)


    def get_debug_image(self) -> Image.Image | None:
        """ Returns image to include in render """
        return self.camera_sensor.get_debug_image()
    



@dataclass
class SpectatorCameraConfig(CameraSensorConfig):
    
    # Mandatory parent config
    python_class = SpectatorCameraInfoProvider
    ue_class_name = "spectator_camera"
