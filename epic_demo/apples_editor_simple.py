#!/usr/bin/env python3
"""
Train a simple apple collecting RobotAgent in an Unreal Engine environment using PPO.
This script by default direclty connects to an existing UE instance, but can also launch a new one.
"""

import argparse
import gymnasium
import numpy as np
from typing import Dict, Any

# Ray/RLlib imports
from ray import tune, air
from ray.rllib.algorithms.ppo import PPOConfig

# UE4 Extended RL (uerl) imports
from uerl.agent.agent_base import AgentConfig, UEAgent
from uerl.components.sensors.distance_tracer_sensor import DistanceTracerSensorConfig
from uerl.ue_gym_env import UEEnvMultiAgent
from uerl.configs import UELaunchSettings, UEEnvConfig

# Components
from uerl.components.actuators.robot_actuator import RobotMovementActuatorConfig
from uerl.components.rewarders.pickup_rewarder import PickupRewarderConfig
from uerl.components.terminators.step_counter_terminator import StepCounterTerminatorConfig

class RobotAgent(UEAgent):
    """
    Custom RobotAgent class that merges or processes observations 
    from multiple sensors in a desired format. This can be used to split
    camera sensor to CNN and other sensors to LSTM, for example.
    """
    def combine_processed_observations(self, observations: Dict[str, Any]) -> Any:
        # Example for retrieving just the CPU camera image:
        return observations["apple_distances"]


def parse_args():
    """
    Parse command-line arguments for IP and port settings.
    """
    parser = argparse.ArgumentParser(
        description="Train a RobotAgent on an Unreal Engine environment using PPO."
    )
    parser.add_argument(
        "--ip",
        type=str,
        default="192.168.1.200",
        help="IP address for the existing UE instance or for launching a new one."
    )
    parser.add_argument(
        "--port",
        type=int,
        default=33333,
        help="Port number to connect to the UE instance."
    )
    return parser.parse_args()


def main():
    args = parse_args()

    # ----------------------------------------------------------------------------
    # 1) Setup configurations for Sensors, Actuators, Rewarders, and Terminators
    # The whole experiment is defined in here.
    # Each Python class has a corresponding class in UE5
    # ----------------------------------------------------------------------------

    apple_distances = DistanceTracerSensorConfig(name="apple_distances",
                                            trace_channel="mlinterest1",
                                            num_traces=30,
                                            half_angle=45.0)

    actuator_conf = RobotMovementActuatorConfig(name="my_robot_actuator")
    rewarder_pickup_conf = PickupRewarderConfig(name="apples_reward")

    terminator_conf = StepCounterTerminatorConfig(
        name="my_step_terminator",
        max_step_count=500
    )

    # ----------------------------------------------------------------------------
    # 2) Define Observation and Action Spaces
    # ----------------------------------------------------------------------------
    agent_obs_space = gymnasium.spaces.Box(low=0.0,
                        high=1.0,
                        shape=(apple_distances.num_traces,),
                        dtype=np.float32)
    agent_action_space = gymnasium.spaces.MultiDiscrete([2, 2, 2, 2, 2])

    # ----------------------------------------------------------------------------
    # 3) Configure the Agent, you can configure multiple agents
    # ----------------------------------------------------------------------------
    agent_conf = AgentConfig(
        agent_id = "Agent1",
        python_class = RobotAgent,
        agent_ue_class = "/CybertoothML/Agents/RobotAgent/BP_RobotAgent.BP_RobotAgent_C",
        spawn_point_name = "Agent1",
        observation_space = agent_obs_space,
        action_space = agent_action_space,
        sensor_configs = [apple_distances],
        actuator_configs = [actuator_conf],
        rewarder_configs = [rewarder_pickup_conf],
        terminator_configs = [terminator_conf],
        infos_configs = []
    )

    # ----------------------------------------------------------------------------
    # 4) Launch / Connect to Unreal Engine
    # ----------------------------------------------------------------------------
    # If ip/port is set, we connect to an existing UE instance. If not, we launch a new one.
    launch_settings = UELaunchSettings(
        map_name="CollectApplesMap",
        specific_ip_port=(args.ip, args.port),
    )

    # UEEnvConfig is the high-level configuration for the UE environment, it
    # contains all the necessary configurations for the environment and the agents.
    ue_conf = UEEnvConfig(launch_settings, [agent_conf], None)

    # If you want to play as an agent with this configuration, paste the following line in the UE console:
    print("Use the following UE console command to join as a player:\n")
    print(ue_conf.get_ue_human_mode_command())

    # ----------------------------------------------------------------------------
    # 5) Ray Tune requires the environment configuration to be passed as a dictionary, cleanest way is to wrap it in a dictionary
    # ----------------------------------------------------------------------------
    env_conf = {
        "ue_conf": ue_conf
    }

    # ----------------------------------------------------------------------------
    # 6) Define Policies for RLlib
    # We can have shared or separate policies for each agent
    # ----------------------------------------------------------------------------
    policies = {
        "policy1": (
            None,  # Use the default PPO Torch or TF policy
            agent_conf.observation_space,
            agent_conf.action_space,
            {}
        ),
    }

    # Define an agent->policy mapping function 
    def policy_mapping_fn(agent_id, episode, worker, **kwargs):
        if agent_id == "Agent1":
            return "policy1"
        raise ValueError(f"Unexpected agent_id: {agent_id}")

    # ----------------------------------------------------------------------------
    # 7) PPO Configuration
    # ----------------------------------------------------------------------------
    config = (
        PPOConfig()
        .rollouts(
            num_rollout_workers=1,
            create_env_on_local_worker=False,
            num_envs_per_worker=1
        )
        .resources(
            num_gpus=0.0,
            num_cpus_per_worker=1.0
        )
        .training(                  # This version of Ray is type hinting a generic training config, 
                                    # which is missing PPO specific parameters, therefore we need to # mark it with type: ignore 
            lr=3e-4,                                # type: ignore
            train_batch_size=terminator_conf.max_step_count * 2, # Training iteration every 2 worker episodes
            sgd_minibatch_size=64,                  # type: ignore
            num_sgd_iter=30,                        # type: ignore
            gamma=0.99,                             # type: ignore
            lambda_=0.98,                           # type: ignore
            clip_param=0.2,                         # type: ignore
            vf_clip_param=10.0,                     # type: ignore
            entropy_coeff=0.0,                      # type: ignore
            vf_loss_coeff=0.5,                      # type: ignore
            grad_clip=0.5,                          # type: ignore
            kl_target=0.01,                         # type: ignore
            model={
                "use_lstm": False,
            }
        )
        .environment(
            env=UEEnvMultiAgent,
            env_config=env_conf
        )
        .multi_agent(
            policies=policies,
            policy_mapping_fn=policy_mapping_fn
        )
    )

    # ----------------------------------------------------------------------------
    # 8) Launch Training using tune.Tuner
    # ----------------------------------------------------------------------------
    tuner = tune.Tuner(
        "PPO",
        tune_config=tune.TuneConfig(
            num_samples=10,
            max_concurrent_trials=1,
        ),
        run_config=air.RunConfig(
            name="three_apples",
            stop={"episode_reward_mean": 600},
            verbose=3,
            checkpoint_config=air.CheckpointConfig(
                checkpoint_score_attribute="episode_reward_mean",
                num_to_keep=5,
                checkpoint_frequency=150,
            ),
            storage_path = "~/ray_results/",
        ),
        param_space=config,
    )

    results = tuner.fit()
    print("Training completed. Results object:\n", results)




if __name__ == "__main__":
    main()
