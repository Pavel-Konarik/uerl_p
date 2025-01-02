#!/usr/bin/env python3
"""
Demonstrates how to load a trained policy and run it in an UE Editor environment.
"""

import argparse
from ray import tune
from uerl.ue_gym_env import UEEnvMultiAgent
from ray.rllib.policy.policy import Policy
import os


def parse_args():
    """
    Parse command-line arguments for IP and port settings.
    """
    parser = argparse.ArgumentParser(
        description="Run trained agent on CollectApples map."
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

    absolute_path = os.path.abspath("./epic_demo/trained_models/collect_apples_traces")
    analysis = tune.ExperimentAnalysis(absolute_path)
    trial = analysis.get_best_trial(metric="episode_reward_mean", mode="max")
    assert trial, "No trials found"
    checkpoint = analysis.get_best_checkpoint(trial=trial, metric="episode_reward_mean", mode="max")
    assert checkpoint, "No checkpoints found"

    env_config = trial.config["env_config"]
    env_config["ue_conf"].launch_settings.specific_ip_port = (args.ip, args.port)

    env = UEEnvMultiAgent(env_config)

    policy1 = Policy.from_checkpoint(checkpoint)
    if isinstance(policy1, dict):
        policy1 = policy1["policy1"]

    for i in range(100):
        obs, infos = env.reset()
        while True:
            actions = {}

            actions["Agent1"] = policy1.compute_single_action(obs["Agent1"])[0]

            obs, rewards, terminated, truncated, infos = env.step(actions)
            done = terminated['__all__'] or truncated['__all__']
            if done:
                print(f"Episode {i} done")
                break


if __name__ == "__main__":
    main()