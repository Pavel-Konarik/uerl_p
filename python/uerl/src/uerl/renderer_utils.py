from typing import Tuple
from ray.rllib.algorithms.algorithm import Algorithm
from ray.rllib.algorithms.algorithm_config import AlgorithmConfig
from ray.rllib.utils.checkpoints import get_checkpoint_info

def get_alg_for_rendering(checkpoint_path:str) -> Tuple[Algorithm, AlgorithmConfig, dict]:
    #checkpoint_path = "/mnt/projects/uerl/python/ray_results/collect_apples/three_apples/PPO_UEEnvMultiAgent_f1859_00000_0_2024-03-10_22-31-31/checkpoint_000000"

    checkpoint_info = get_checkpoint_info(checkpoint_path)


    state = Algorithm._checkpoint_info_to_algorithm_state(
                checkpoint_info=checkpoint_info,
                policy_ids=None,
                policy_mapping_fn=None,
                policies_to_train=None,
            )

    # We do not want to spawn a default environment for learner to get observation space and action space
    # It should be grabbed from policies (if not default)
    state["config"] = state["config"].environment(env=None)
    # We also dont want to use any GPUs or allocate any CPUs from ray cluster
    state["config"] = state["config"].resources(num_gpus=0, num_cpus_per_worker=0.0)
    # Disable rollouts workers
    state["config"] = state["config"].rollouts(num_rollout_workers=0)

    loaded_alg = Algorithm.from_state(state)
    return loaded_alg, state["config"], state