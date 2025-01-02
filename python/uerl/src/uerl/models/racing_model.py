# type: ignore
# WIP code for custom racing model
import gymnasium as gym
from typing import Dict, List, Optional, Sequence

from ray.rllib.models.tf.tf_modelv2 import TFModelV2
from ray.rllib.models.modelv2 import restore_original_dimensions
from ray.rllib.models.tf.misc import normc_initializer
from ray.rllib.models.utils import get_activation_fn, get_filter_config
from ray.rllib.policy.sample_batch import SampleBatch
from ray.rllib.utils.framework import try_import_tf
from ray.rllib.utils.typing import ModelConfigDict, TensorType

tf1, tf, tfv = try_import_tf()


class CarRacingNetwork(TFModelV2):
    """Generic vision network implemented in ModelV2 API.

    An additional post-conv fully connected stack can be added and configured
    via the config keys:
    `post_fcnet_hiddens`: Dense layer sizes after the Conv2D stack.
    `post_fcnet_activation`: Activation function to use for this FC stack.
    """
    def __init__(
        self,
        obs_space: gym.spaces.Space,
        action_space: gym.spaces.Space,
        num_outputs: int,
        model_config: ModelConfigDict,
        name: str,
    ):
        
        super(CarRacingNetwork, self).__init__(
            obs_space, action_space, num_outputs, model_config, name
        )

        original_space = obs_space.original_space if hasattr(obs_space, "original_space") else obs_space

        self.data_format = "channels_last"

        #camera_input_shape = original_space["camera"].shape
        camera_input_shape=(84,84,1)
        camera_input = tf.keras.layers.Input(shape=camera_input_shape, name="camera_input")

        #location_input_shape = original_space["location"].shape
        location_input_shape = (3,)
        location_input = tf.keras.layers.Input(shape=location_input_shape, name="location_input")
        
        #rotation_input_shape = original_space["rotation"].shape
        rotation_input_shape = (2,)
        rotation_input = tf.keras.layers.Input(shape=rotation_input_shape, name="rotation_input")
        
        #velocity_input_shape = original_space["velocity"].shape
        location_input_shape = (3,)
        velocity_input = tf.keras.layers.Input(shape=location_input_shape, name="velocity_input")
        
        meta_data = tf.keras.layers.concatenate([location_input, rotation_input, velocity_input])

        conv1 = tf.keras.layers.Conv2D(16, [8, 8], strides=(4, 4), activation="relu", padding="same", data_format="channels_last", name="conv1")(camera_input)
        conv2 = tf.keras.layers.Conv2D(32, [4, 4], strides=(2, 2), activation="relu", padding="same", data_format="channels_last", name="conv2")(conv1)
        conv3 = tf.keras.layers.Conv2D(128, [11, 11], strides=(1, 1), activation="relu", padding="valid", data_format="channels_last", name="conv3")(conv2)
        #conv_out = tf.keras.layers.Conv2D(num_outputs, [1, 1], activation=None, padding="same", data_format="channels_last", name="conv_out")(conv3)

        conv_flatten = tf.keras.layers.Flatten()(conv3)
        camera_meta_concat = tf.keras.layers.concatenate([conv_flatten, meta_data])

        dense_1 = tf.keras.layers.Dense(64, activation="relu")(camera_meta_concat)
        dense_2 = tf.keras.layers.Dense(64, activation="relu")(dense_1)
        dense_3 = tf.keras.layers.Dense(32, activation="relu")(dense_2)
        out = tf.keras.layers.Dense(num_outputs, activation="linear")(dense_3)


        # VALUE
        conv_value_1 = tf.keras.layers.Conv2D(16, [8, 8], strides=(4, 4), activation="relu", padding="same", data_format="channels_last", name="conv_value_1")(camera_input)
        conv_value_2 = tf.keras.layers.Conv2D(32, [4, 4], strides=(2, 2), activation="relu", padding="same", data_format="channels_last", name="conv_value_2")(conv_value_1)
        conv_value_3 = tf.keras.layers.Conv2D(128, [11, 11], strides=(1, 1), activation="relu", padding="valid", data_format="channels_last", name="conv_value_3")(conv_value_2)
        #conv_value_out = tf.keras.layers.Conv2D(1, [1, 1], activation=None, padding="same", data_format="channels_last", name="conv_value_out")(conv_value_3)
        conv_value_flatten = tf.keras.layers.Flatten()(conv_value_3)
        value_camera_meta_concat = tf.keras.layers.concatenate([conv_value_flatten, meta_data])

        dense_value_1 = tf.keras.layers.Dense(64, activation="relu")(value_camera_meta_concat)
        dense_value_2 = tf.keras.layers.Dense(64, activation="relu")(dense_value_1)
        dense_value_3 = tf.keras.layers.Dense(32, activation="relu")(dense_value_2)
        value_out = tf.keras.layers.Dense(1, activation="linear")(dense_value_3)


        #value_out = tf.keras.layers.Lambda(lambda x: tf.squeeze(x, axis=[1, 2]))(
        #    value_out
        #)


        self.base_model = tf.keras.Model({"camera": camera_input, "location": location_input, "rotation": rotation_input, "velocity": velocity_input}, [out, value_out])

    def forward(
        self,
        input_dict: Dict[str, TensorType],
        state: List[TensorType],
        seq_lens: TensorType,
    ):
        #print("HERE", input_dict["obs"], type(input_dict["obs"]))
        #assert False, f"input dict {input_dict['obs']}"

        #obs = input_dict["obs"]
        #print(obs)
        #obs = restore_original_dimensions(input_dict["obs"], self.obs_space, "tf")
        #if self.data_format == "channels_first":
        #    obs = tf.transpose(obs, [0, 2, 3, 1])
        # Explicit cast to float32 needed in eager.

        if SampleBatch.OBS in input_dict and "obs_flat" in input_dict:
            orig_obs = input_dict[SampleBatch.OBS]
        else:
            orig_obs = restore_original_dimensions(input_dict[SampleBatch.OBS], self.obs_space, "tf")

        inputs = {'camera': orig_obs["camera"], 'location': orig_obs["location"], 'rotation': orig_obs["rotation"], 'velocity': orig_obs["velocity"]}

        model_out, self._value_out = self.base_model(inputs)

        #return tf.squeeze(model_out, axis=[1, 2]), state
        return model_out, state

    def value_function(self) -> TensorType:
        return tf.reshape(self._value_out, [-1])

