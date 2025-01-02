# type: ignore
# Custom implementation of the IMPALA model (WIP)
import ray
import numpy as np
import gymnasium as gym
from gymnasium.spaces import Box, Discrete, MultiDiscrete
import logging
import tree  # pip install dm_tree
from typing import Dict, List, Optional, Type, Tuple

from ray.rllib.models.modelv2 import ModelV2
from ray.rllib.models.tf.tf_modelv2 import TFModelV2
from ray.rllib.policy.rnn_sequencing import add_time_dimension
from ray.rllib.policy.sample_batch import SampleBatch
from ray.rllib.policy.view_requirement import ViewRequirement
from ray.rllib.utils.annotations import override, DeveloperAPI
from ray.rllib.utils.framework import try_import_tf
from ray.rllib.utils.spaces.space_utils import get_base_struct_from_space
from ray.rllib.utils.tf_utils import flatten_inputs_to_1d_tensor, one_hot
from ray.rllib.utils.typing import ModelConfigDict, TensorType
from ray.rllib.models.tf.recurrent_net import RecurrentNetwork


tf1, tf, tfv = try_import_tf()
logger = logging.getLogger(__name__)




class MyImpalaModel(RecurrentNetwork):
    """An LSTM wrapper serving as an interface for ModelV2s that set use_lstm."""

    def __init__(
        self,
        obs_space: gym.spaces.Space,
        action_space: gym.spaces.Space,
        num_outputs: int,
        model_config: ModelConfigDict,
        name: str,
    ):

        super(MyImpalaModel, self).__init__(
            obs_space, action_space, None, model_config, name
        )
        # At this point, self.num_outputs is the number of nodes coming
        # from the wrapped (underlying) model. In other words, self.num_outputs
        # is the input size for the LSTM layer.
        # If None, set it to the observation space.
        if self.num_outputs is None:
            self.num_outputs = 512 #int(np.product(self.obs_space.shape))

        self.cell_size = 256

        self.action_space_struct = get_base_struct_from_space(self.action_space)
        self.action_dim = 0

        for space in tree.flatten(self.action_space_struct):
            if isinstance(space, Discrete):
                self.action_dim += space.n
            elif isinstance(space, MultiDiscrete):
                self.action_dim += np.sum(space.nvec)
            elif space.shape is not None:
                self.action_dim += int(np.product(space.shape))
            else:
                self.action_dim += int(len(space))


        # Define input layers.
        input_layer = tf.keras.layers.Input(
            shape=(None, self.num_outputs), name="inputs"
        )

        # Set self.num_outputs to the number of output nodes desired by the
        # caller of this constructor.
        self.num_outputs = num_outputs

        state_in_h = tf.keras.layers.Input(shape=(self.cell_size,), name="h")
        state_in_c = tf.keras.layers.Input(shape=(self.cell_size,), name="c")
        seq_in = tf.keras.layers.Input(shape=(), name="seq_in", dtype=tf.int32)

        # Preprocess observation with a hidden layer and send to LSTM cell
        lstm_out, state_h, state_c = tf.keras.layers.LSTM(
            self.cell_size, return_sequences=True, return_state=True, name="lstm"
        )(
            inputs=input_layer,
            mask=tf.sequence_mask(seq_in),
            initial_state=[state_in_h, state_in_c],
        )

        # Postprocess LSTM output with another hidden layer and compute values
        logits = tf.keras.layers.Dense(
            self.num_outputs, activation=tf.keras.activations.linear, name="logits"
        )(lstm_out)
        values = tf.keras.layers.Dense(1, activation=None, name="values")(lstm_out)

        # Create the RNN model
        self._rnn_model = tf.keras.Model(
            inputs=[input_layer, seq_in, state_in_h, state_in_c],
            outputs=[logits, values, state_h, state_c],
        )
        # Print out model summary in INFO logging mode.
        if logger.isEnabledFor(logging.INFO):
            self._rnn_model.summary()


        ############################################
        ## Vision

        input_visual = tf.keras.layers.Input(shape=(72, 72, 3), name="observations")
        visual_conv_1 = tf.keras.layers.Conv2D(32, (8, 8), strides=(4, 4), activation='relu')(input_visual)
        visual_conv_2 = tf.keras.layers.Conv2D(64, (4, 4), strides=(2, 2), activation='relu')(visual_conv_1)
        visual_conv_3 = tf.keras.layers.Conv2D(64, (3, 3), strides=(1, 1), activation='relu')(visual_conv_2)
        flatten_visual = tf.keras.layers.Flatten(data_format="channels_last")(visual_conv_3)
        visual_dense = tf.keras.layers.Dense(512, activation="relu", name="visual_dense")(flatten_visual)

        self.vision_net = tf.keras.Model(input_visual, visual_dense)

      

    @override(RecurrentNetwork)
    def forward(
        self,
        input_dict: Dict[str, TensorType],
        state: List[TensorType],
        seq_lens: TensorType,
    ) -> Tuple[TensorType, List[TensorType]]:
        assert seq_lens is not None
        # Push obs through "unwrapped" net's `forward()` first.

        obs = input_dict["obs"]
        

        wrapped_out = self.vision_net(obs)

        print(wrapped_out.shape)

        # Push everything through our LSTM.
        input_dict["obs_flat"] = wrapped_out
        return super().forward(input_dict, state, seq_lens)

    @override(RecurrentNetwork)
    def forward_rnn(
        self, inputs: TensorType, state: List[TensorType], seq_lens: TensorType
    ) -> Tuple[TensorType, List[TensorType]]:
        model_out, self._value_out, h, c = self._rnn_model([inputs, seq_lens] + state)
        return model_out, [h, c]

    @override(ModelV2)
    def get_initial_state(self) -> List[np.ndarray]:
        return [
            np.zeros(self.cell_size, np.float32),
            np.zeros(self.cell_size, np.float32),
        ]

    @override(ModelV2)
    def value_function(self) -> TensorType:
        return tf.reshape(self._value_out, [-1])



if __name__ == "__main__":

    action_space = MultiDiscrete([2, 2, 2, 2])  # MultiBinary(4) 
    observation_space =  Box(low=0.0, high=1.0, shape=(72,), dtype=np.float32)
    print(observation_space.shape)
    num_outputs = 1

    model = MyImpalaModel(observation_space, action_space, num_outputs, {}, "MyModel")