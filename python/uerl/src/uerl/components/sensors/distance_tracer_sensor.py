from __future__ import annotations
from uerl.components.component_base import SensorBase, SensorConfig
from collections import deque
from copy import deepcopy
from typing import Optional, Type, Dict, Union, List, Tuple, Any
import numpy as np
from dataclasses import dataclass, field
from PIL import Image
from uerl.types import FRotator, FVector

class DistanceTracerSensor(SensorBase):
    def __init__(self, config: DistanceTracerSensorConfig) -> None:
        super().__init__(config)

        # Config variables
        self.config: DistanceTracerSensorConfig = deepcopy(config) # type: ignore

        # Latest frame for debug rendering
        self.latest_frame: np.ndarray | None = None

        # Non config variables (frame stacking)
        self.frames = deque(maxlen=self.config.frame_stack_size)

    def reset(self):
        self.latest_frame = None
        self.frames.clear()

    def process_obs(self, obs: bytearray) -> np.ndarray:
        
        frame_np = np.frombuffer(obs, dtype=np.float32).reshape((self.config.num_traces,))
        self.latest_frame = frame_np

        # Frame stacking
        if self.frames.maxlen and len(self.frames) != self.frames.maxlen:
            # First observation, fill buffer
            for _ in range(self.frames.maxlen):
                self.frames.append(frame_np)
        else:
            # Normal Step
            self.frames.append(frame_np)

        if self.config.output_stacked_shape:
            stacked_frames = np.stack(self.frames, axis=1)
            return stacked_frames

        return np.concatenate(list(self.frames), axis=-1)



    def get_debug_image(self) -> Image.Image | None:
        """ Returns image to include in render """
        if self.latest_frame is None:
            # Create a white image for the case with no data
            return Image.fromarray(np.full((1, self.config.num_traces), 255, dtype=np.uint8))

        # Normalize the latest_frame values if they are not normalized
        if not self.config.should_normalise:
            frame_normalized = self.latest_frame / self.config.trace_length
        else:
            frame_normalized = self.latest_frame

        # Rescale to 0-255 for image representation
        frame_scaled = np.uint8(frame_normalized * 255)

        # Create a grayscale image where each trace is a column
        image_array = np.tile(frame_scaled, (self.config.num_traces, 1))

        # Convert to a PIL image
        out_image = Image.fromarray(image_array)

        return out_image
    


@dataclass
class DistanceTracerSensorConfig(SensorConfig):
    num_traces: int = 10
    half_angle: float = 90.0
    trace_length: float = 3700.0
    should_normalise: bool = True
    trace_channel: str = "visibility"
    rotation: FRotator = field(default_factory=lambda: FRotator(0, 0, 0))
    location: FVector = field(default_factory=lambda: FVector(0, 0, 0))
    invert_values: bool = True # Invert the values so that a higher distance corresponds to a darker shade
    frame_stack_size: int = 1
    output_stacked_shape: bool = False # If true, the output shape will be (num_traces, frame_stack_size) otherwise (num_traces * frame_stack_size)

    # Mandatory parent config
    python_class = DistanceTracerSensor
    ue_class_name = "distance_tracer"

if __name__ == "__main__":
    # Test code
    config = DistanceTracerSensorConfig(name="Test1", num_traces=10, frame_stack_size=5, output_stacked_shape=False)
    sensor = DistanceTracerSensor(config)

    # Adding 10 "frames" to the sensor
    for i in range(10):
        # Generating dummy data for testing
        dummy_data = bytearray(np.random.rand(config.num_traces).astype(np.float32))
        a = sensor.process_obs(dummy_data)

    print("Number of frames stored:", len(sensor.frames))
    print("Latest frame:", sensor.latest_frame)