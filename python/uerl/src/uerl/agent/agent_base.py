from __future__ import annotations
import abc
from typing import Optional, Sequence, Type, Dict, Union, List, Tuple, Any
from dataclasses import asdict, dataclass, field
import gymnasium as gym
from uerl.components.component_base import AgentComponentBase, AgentComponentConfig, SensorBase, SensorConfig, ActuatorBase, ActuatorConfig, RewarderBase, RewarderConfig, TerminatorBase, TerminatorConfig, InfoProviderBase, InfoProviderConfig


@dataclass
class AgentConfig():
    agent_id: str
    python_class: Type[UEAgent]
    agent_ue_class: str
    spawn_point_name: str
    observation_space: gym.Space
    action_space: gym.Space
    sensor_configs: List[SensorConfig]
    actuator_configs: List[ActuatorConfig]
    rewarder_configs: List[RewarderConfig]
    terminator_configs: List[TerminatorConfig]
    infos_configs: Sequence[AgentComponentConfig]

    def instantiate(self) -> UEAgent:
        return self.python_class(self)


class UEAgent():
    def __init__(self, agent_config: AgentConfig) -> None:
        self.config = agent_config

        self.sensors: Dict[str, SensorBase] = {}
        self.actuators: Dict[str, ActuatorBase] = {}
        self.rewarders: Dict[str, RewarderBase] = {}
        self.terminators: Dict[str, TerminatorBase] = {}
        self.infos: Dict[str, InfoProviderBase] = {}

        self.all_components: Dict[str, AgentComponentBase] = {}


        # Create sensor objects from configs
        for sensor_config in self.config.sensor_configs:
            sensor = sensor_config.instantiate()
            self.sensors[sensor.name] = sensor

        # Create actuator objects from configs
        for actuator_config in self.config.actuator_configs:
            actuator = actuator_config.instantiate()
            self.actuators[actuator.name] = actuator

        # Create rewarders objects from configs
        for rewarder_config in self.config.rewarder_configs:
            rewarder = rewarder_config.instantiate()
            self.rewarders[rewarder.name] = rewarder

        # Create terminators objects from configs
        for terminator_config in self.config.terminator_configs:
            terminator = terminator_config.instantiate()
            self.terminators[terminator.name] = terminator

        # Create infos objects from configs
        for info_config in self.config.infos_configs:
            info = info_config.instantiate()
            self.infos[info.name] = info
        

        # Populate self.all_components
        self.all_components = {**self.sensors, **self.actuators, **self.rewarders, **self.terminators, **self.infos}

    @property
    def agent_id(self) -> str:
        return self.config.agent_id

    @property
    def python_class(self) -> Type[UEAgent]:
        return self.config.python_class

    @property
    def agent_ue_class(self) -> str:
        return self.config.agent_ue_class

    @property
    def spawn_point_name(self) -> str:
        return self.config.spawn_point_name

    @property
    def observation_space(self) -> gym.Space[Any]:
        return self.config.observation_space

    @property
    def action_space(self) -> gym.Space[Any]:
        return self.config.action_space

    def get_component_by_name(self, name: str) -> AgentComponentBase | None:
        return self.all_components.get(name, None)

    def get_all_components(self) -> Sequence[AgentComponentBase]:
        return list(self.sensors.values()) + list(
            self.actuators.values()) + list(self.rewarders.values()) + list(
                self.terminators.values()) + list(self.infos.values())

    def reset(self):
        # Inform the components we reset
        for comp in self.get_all_components():
            comp.reset()

    def process_observations(
            self, observations: Dict[str, Dict[str, Dict[str, bytearray]]]) -> Any:
        """On each step, we receive observations from UE, which is 
        in form of Dict[sensor_name, bytes data]. It gets routed into
        this function, which routes each observation to to correct sensor 
        and then optionally combines the sensor processed outputs.

        Example input:
        {'Sensors': {'camera': {'Data': b'\xff\xcc'}}}}
        "Sensors" and "Data" are constant, sadly remnants of how UE struct -> bytes work
        
        Args:
            observations (Dict[str, bytes]): initial raw observation from UE for this agent

        Returns:
            Any: Any observation your model handles. It should match the agents observation_space
        """

        observations_sensors = observations["Sensors"]
        # Store the processed observations (sensor_name -> Any processed observation)
        processed_obs: Dict[str, Any] = {}
        # Route observation to correct sensor
        for sensor_name, obs in observations_sensors.items():
            sensor_obs = self.sensors[sensor_name].process_obs(obs["Data"])
            processed_obs[sensor_name] = sensor_obs

        # Optionally combine the observations to fit the model input
        combined_obs = self.combine_processed_observations(processed_obs)
        return combined_obs

    def combine_processed_observations(self, observations: Dict[str,
                                                                Any]) -> Any:
        return observations

    def process_rewards(
            self, rewards: Dict[str, Dict[str, Dict[str, float]]]) -> float:

        rewarders = rewards["Rewarders"]

        # Store the processed rewards (reward_name -> Any processed reward)
        processed_rewards: Dict[str, Any] = {}
        # Route reward to correct rewarder
        for rewarder_name, reward in rewarders.items():
            reward_value = self.rewarders[rewarder_name].process_reward(reward["Reward"])
            processed_rewards[rewarder_name] = reward_value
        
        # Sum all rewards
        summed_rewards = sum(processed_rewards.values())
        return summed_rewards

    def process_terminations(
            self, terminations: Dict[str, Dict[str, Dict[str, float]]]) -> tuple[bool, bool]:
        
        actual_terminations = terminations["Terminators"]
        
        processed_terminations: Dict[str, Any] = {}
        processed_truncations: Dict[str, Any] = {}
        
        for terminator_name, termination in actual_terminations.items():
            termination_value, truncation_value = self.terminators[terminator_name].process_termination(termination)
            processed_terminations[terminator_name] = termination_value
            processed_truncations[terminator_name] = truncation_value
        
        any_termination = any(processed_terminations.values())
        any_truncation = any(processed_truncations.values())
        return any_termination, any_truncation



    def process_infos(self, infos) -> Any:
        """
        Example input:
        {'InfoProviders': {'camera': {'Data': b'\xff\xcc'}}}}
        "InfoProviders" and "Data" are constant, sadly remnants of how UE struct -> bytes work

        """
        infos = infos["InfoProviders"]
        # Store the processed infos (info_provider_name -> Any processed info)
        processed_infos: Dict[str, Any] = {}
        # Route infos to correct info provider
        for info_name, info in infos.items():
            processed_info = self.infos[info_name].process_info(info["Data"])
            processed_infos[info_name] = processed_info

        return processed_infos
