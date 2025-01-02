from typing import Dict
import numpy as np
import json

import time
import uuid
import traceback

from ray.rllib.env.multi_agent_env import MultiAgentEnv
from uerl.renderer import EpisodeVideoRenderer
from uerl.ue_runner import UERunner

from ray.rllib.utils.typing import (
    MultiAgentDict, )

from typing import Set

from websocket import create_connection
import cbor2
import numpy as np
from typing import Optional, Dict, Tuple, Any
from dataclasses import asdict
import gymnasium
from copy import deepcopy
from uerl.configs import UEEnvConfig
from uerl.agent.agent_base import UEAgent
from uerl.requests import ResetRequest_Data, send_to_ue, ERLEnvCommandType, Action_StepRequest, ActuatorData, AgentActuators
from uerl.utils import find_unused_port

import logging
from websocket import WebSocket as BaseWebSocket

class CustomWebSocket(BaseWebSocket):
    def close(self, status=None, reason=None, timeout=None):
        print("Closing websocket on Python side")
        if reason:
            print(f"WebSocket closed with reason: {reason}")
        super().close(status, reason, timeout)

# Create a logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# Create a file handler with a unique filename based on the timestamp and random letters
timestamp = time.strftime("%Y%m%d-%H%M%S")
# random_str = ''.join(random.choices('abcdefghijklmnopqrstuvwxyz', k=5))
# filename = f"log_{timestamp}_{random_str}.log"
# file_handler = logging.FileHandler(filename)
# file_handler.setLevel(logging.INFO)

# Create a console handler
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.INFO)

# Create a formatter and set it for both handlers
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
#file_handler.setFormatter(formatter)
console_handler.setFormatter(formatter)

# Add the handlers to the logger
# logger.addHandler(file_handler)
logger.addHandler(console_handler)

# logger.info("Logging initialized. Log file: {}".format(filename))


class UEEnvMultiAgent(MultiAgentEnv):
    def __init__(self, config_wrapper: Dict[str, UEEnvConfig] | UEEnvConfig):
        super().__init__()
        
        logger.info("UEEnvMultiAgent.__init__")
        if isinstance(config_wrapper, dict):
            config = config_wrapper["ue_conf"]
        else:
            config = config_wrapper

        # Just to be sure, make a copy of the config in case it's re-used
        self.config: UEEnvConfig = deepcopy(config)

        # TODO: Move this to config
        self.seed = 42

        # ticks per "simulated" second. The higher the number, the higher the resolution but slower training time
        self.tickfps = config.launch_settings.tickfps

        # Identify the environment by this name
        conf_run_name = config.launch_settings.run_name
        self.run_name = conf_run_name if conf_run_name is not None else f"{time.time()}_{str(uuid.uuid4())}"

        ## Debugs
        self.output_ue_log = config.launch_settings.output_ue_log
        # This is used for connecting to an existing instance (for example Editor)
        self.specific_ip_port = config.launch_settings.specific_ip_port

        # This determines if we will use GPU (or just CPU) and if we want to see the game window
        self.render_type = config.launch_settings.render_type

        # These values are used when render type is ON_SCREEN or GPU_RENDER.
        self.window_res_x = config.launch_settings.window_res_x
        self.window_res_y = config.launch_settings.window_res_y

        # Define agents here
        self.agent_ids: Set[str] = set(config.get_all_agent_ids())
        self._agent_ids: Set[str] = self.agent_ids

        # This holds all agents. Maps agent_id -> agent object
        self.agents: Dict[str, UEAgent] = {}
        # Create agents
        for agent_conf in config.agent_configs:
            agent: UEAgent = agent_conf.instantiate()
            self.agents[agent.agent_id] = agent

        # Provide full (preferred format) observation- and action-spaces as Dicts
        self._action_space_in_preferred_format = True
        # Due to the bug in check_env in Ray 2.3.1, we need to specify both _obs and _observation
        self._observation_space_in_preferred_format = True
        self._obs_space_in_preferred_format = True

        self.observation_space = gymnasium.spaces.Dict()
        self.action_space = gymnasium.spaces.Dict()
        for agent in self.agents.values():
            self.observation_space[agent.agent_id] = agent.observation_space
            self.action_space[agent.agent_id] = agent.action_space

        self.ws_client = None

        self.engine_process = None
        if self.specific_ip_port and self.specific_ip_port[1] != -1:
            self.ip = self.specific_ip_port[0]
            self.port = self.specific_ip_port[1]
            # Check for defaults
            assert self.ip != "" and self.port != -1

            # Connect to the WebSocket server
            server_url = f"ws://{self.ip}:{self.port}"
            self.ws_client = create_connection(server_url, timeout=20, class_=CustomWebSocket)
        else:
            # Keep the ptr to the game process, so we can kill it if we exit this env
            logger.info("Starting UE5 environment")
            # Sometimes, we get already used port. Retry creating a game env n number of times (each time with new port)
            max_retries = 5
            retry_count = 0
            connected = False
            while not connected and retry_count < max_retries:
                logger.info(f"Connecting to the environment. Retrying... ({retry_count}/{max_retries})")
                try:
                    self.ip = "localhost"
                    self.port = find_unused_port()

                    self.engine_process = UERunner.run(self.config.launch_settings.map_name,
                                                       self.port,
                                                       self.render_type,
                                                       self.window_res_x,
                                                       self.window_res_y,
                                                       self.output_ue_log,
                                                       self.config.launch_settings.executable_path)
                    
                    # Connect to the WebSocket server
                    server_url = f"ws://{self.ip}:{self.port}"

                    
                    time.sleep(5)                    
                    if self.engine_process.poll() is not None:
                        raise Exception("Failed to start the environment")


                    logger.info(f"connecting to {server_url}")
                    for i in range(100):
                        try:
                            logger.info(f"connecting to {server_url} attempt {i}")
                            self.ws_client = create_connection(server_url, timeout=20)
                            connected = True
                            break
                        except Exception as e:
                            if i > 50:
                                logger.warning(f"Failed to connect to the environment. Retrying... {i} - ({retry_count}/{max_retries})")
                                logger.warning(e)

                            time.sleep(5)
                    

                except Exception as e:
                    traceback.print_exc()
                    
                    logger.error("An exception occurred", exc_info=True)
    
                    retry_count += 1
                    logger.info(
                        "Failed to connect to the environment. Retrying... ({}/{})"
                        .format(retry_count, max_retries))
                    if self.engine_process:
                        self.engine_process.send_signal(9)
                    time.sleep(1.1)

            if not connected:
                logger.error(f"Failed to connect to the environment after {max_retries} retries")
                self.close()

                raise Exception(
                    "Failed to connect to the environment after {} retries".
                    format(max_retries))

        assert self.ws_client is not None

        # Configure the ue environment
        config_success = self.send_config(self.config)
        assert config_success

        logger.info(f"UE5 environment started at {self.ip}:{self.port}")

        # Debug renderer
        self.episode_renderer = None
        if self.config.episode_renderer_config is not None:
            self.episode_renderer = EpisodeVideoRenderer.from_config(self.config.episode_renderer_config)

    def reset(
        self,
        *,
        seed: Optional[int] = None,
        options: Optional[dict] = None,
    ) -> Tuple[MultiAgentDict, MultiAgentDict]:
        seed = seed if seed is not None else 0
        options = options if options is not None else {}
        assert self.ws_client is not None, "Websocket client is not connected"

        self.seed = seed

        if self.episode_renderer:
            self.episode_renderer.reset()

        # Inform all that we reset
        for agent in self.agents.values():
            agent.reset()

        # Send reset to env
        reset_data = ResetRequest_Data(seed, options)
        reset_data_str = json.dumps(asdict(reset_data))
        reset_bytes = reset_data_str.encode('utf-8')
        send_to_ue(self.ws_client, reset_bytes, ERLEnvCommandType.ResetRequest)

        # Wait for the result containing observations and infos
        result_raw = self.ws_client.recv()
        assert isinstance(result_raw, bytes), "Expected bytes"
        result = cbor2.loads(result_raw)
        
        obs = {}
        for agent_name, agent_obs in result["Obs"].items():
            obs[agent_name] = self.process_observation(agent_name, agent_obs)
        infos = {}
        for agent_name, agent_infos in result["Infos"].items():
            infos[agent_name] = self.process_infos(agent_name, agent_infos)

        return obs, infos

    def step(self, action_dict: MultiAgentDict) -> Tuple[
        MultiAgentDict, MultiAgentDict, MultiAgentDict, MultiAgentDict, MultiAgentDict
    ]:
        assert self.ws_client is not None, "Websocket client is not connected"

        action_request: Action_StepRequest = self.construct_ue_actions(action_dict)

        # Send step to env
        request_bytes = cbor2.dumps(asdict(action_request))
        send_to_ue(self.ws_client, request_bytes,
                   ERLEnvCommandType.StepRequest)

        # Wait for observation response
        result_raw = self.ws_client.recv()
        assert isinstance(result_raw, bytes), "Expected bytes"
        result = cbor2.loads(result_raw)

        obs = {}
        for agent_name, agent_obs in result["Obs"].items():
            obs[agent_name] = self.process_observation(agent_name, agent_obs)

        rewards = {}
        for agent_name, agent_reward in result["Rewards"].items():
            rewards[agent_name] = self.process_rewards(agent_name, agent_reward)
        
        
        terminated = {}
        truncated = {}
        for agent_name, agent_term in result["Terminated"].items():
            agent_terminated, agent_truncated = self.process_terminations(agent_name, agent_term)
            terminated[agent_name] = agent_terminated
            truncated[agent_name] = agent_truncated

        terminated["__all__"] = all(terminated.values())
        truncated["__all__"] = all(truncated.values())

        infos = {}
        for agent_name, agent_infos in result["Infos"].items():
            infos[agent_name] = self.process_infos(agent_name, agent_infos)


        # Debug renderer
        if self.episode_renderer:
            self.episode_renderer.step(self.agents, terminated, truncated)


        return obs, rewards, terminated, truncated, infos

    def construct_ue_actions(self,
                             actions: MultiAgentDict) -> Action_StepRequest:
        """ actions is coming from your algorithm. You need to map the these actions
        to each agents actuator."""
        return self.map_actions_to_first_actuator(actions)

    def map_actions_to_first_actuator(
            self, actions: MultiAgentDict) -> Action_StepRequest:
        request = Action_StepRequest()
        for agent_id, agent_actions in actions.items():
            # Ensure we have only 1 actuator per agent
            assert len(self.agents[agent_id].actuators) == 1
            actuator_name = list(self.agents[agent_id].actuators.keys())[0]

            assert isinstance(
                agent_actions,
                np.ndarray), "agent_actions should be a NumPy array"

            # Convert the numpy array to bytes
            actuator_data = ActuatorData(agent_actions.tobytes())
            request.Agents[agent_id] = AgentActuators(
                {actuator_name: actuator_data})

        return request

    def close(self, shutdown=True):
        """ Disconnects this environment from the simulation.
        Args:
            shutdown (bool): if True will shut down the simulation as well.
        """
        print('close(shutdown={}, {})'.format(shutdown, self.render_type))
        if shutdown:
            if self.engine_process:
                print(self.engine_process)
                self.engine_process.send_signal(9)
                kill_request_time = time.time()

                while time.time(
                ) - kill_request_time < 6 and self.engine_process.poll() != -9:
                    time.sleep(0.05)
                if self.engine_process.poll() != -9:
                    print("Failed to close engine environment")
                self.engine_process = None

    def send_config(self, config: UEEnvConfig):
        assert self.ws_client is not None, "Websocket client is not connected"

        config_str = json.dumps(config.get_ue_config())
        config_bytes = config_str.encode('utf-8')
        send_to_ue(self.ws_client, config_bytes,
                   ERLEnvCommandType.ConfigureRequest)
        
        # Wait for the response
        response = self.ws_client.recv()
        assert isinstance(response, bytes), "Expected bytes"
        response = response.decode('utf-8')
        return response == "ok"

    def process_observation(self, agent_name: str, observations: Dict) -> Any:
        agent = self.agents[agent_name]
        return agent.process_observations(observations)

    def process_rewards(self, agent_name: str, rewards: Dict) -> float:
        agent = self.agents[agent_name]
        return agent.process_rewards(rewards)

    def process_terminations(self, agent_name: str, terminations: Dict) -> tuple[bool, bool]:
        agent = self.agents[agent_name]
        return agent.process_terminations(terminations)

    def process_infos(self, agent_name: str, info: Dict) -> Any:
        agent = self.agents[agent_name]
        return agent.process_infos(info)



class UESingleAgentEnv(UEEnvMultiAgent):
    def __init__(self, config_wrapper: Dict[str, UEEnvConfig]):
        super().__init__(config_wrapper)
        
        assert len(self.agents) == 1, "This environment is for single agent only"
        
        self.agent_id = next(iter(self.agent_ids))

        # Fix the observation and action spaces
        # HACK
        self.observation_space = self.observation_space[self.agent_id] # type: ignore
        self.action_space = self.action_space[self.agent_id] # type: ignore


    def reset(
        self,
        *,
        seed: Optional[int] = None,
        options: Optional[dict] = None,
    ) -> Tuple[MultiAgentDict, MultiAgentDict]:
        
        obs, infos = super().reset(seed=seed, options=options)
        
        return obs[self.agent_id], infos[self.agent_id]

    def step(self, action_dict: MultiAgentDict) -> Tuple[
        MultiAgentDict, MultiAgentDict, MultiAgentDict, MultiAgentDict, MultiAgentDict
    ]:
        obs, reward, terminated, truncated, infos = super().step({self.agent_id: action_dict})

        return obs[self.agent_id], reward[self.agent_id], terminated[self.agent_id], truncated[self.agent_id], infos[self.agent_id]


    


