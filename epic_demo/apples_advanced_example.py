# In this example we will launch large-scale training of 2 agents with different policies and goals (rewarders)
# It will be a Population Based Training (PBT) where we try to find optiomal hyperparameters for training
# It is advised that you have a look at collect_apples.ipynb and apples_editor_simple.py before running this example


import random
from uerl.components.rewarders.pickup_rewarder import PickupRewarderConfig
from uerl.ue_gym_env import UEEnvMultiAgent
from uerl.configs import UELaunchSettings, UEEnvConfig
from uerl.components.sensors.camera_sensor import CameraSensorConfig
from uerl.components.actuators.robot_actuator import RobotMovementActuatorConfig
from uerl.components.rewarders.world_direction_rewarder import WorldDirectionRewarderConfig
from uerl.components.terminators.step_counter_terminator import StepCounterTerminatorConfig
from uerl.components.terminators.actors_of_class_finished_terminator import ActorsOfClassFinishedTerminatorConfig
from uerl.agent.agent_base import AgentConfig, UEAgent
from uerl.types import RenderType
from ray.air import FailureConfig
from ray.rllib.algorithms.ppo import PPOConfig
from ray import tune, air
from typing import List
from ray import tune

# For periodic rendering of the training process
from ray.tune import Callback
from uerl.renderer_utils import get_alg_for_rendering
from uerl.components.infos.camera_spectator_info import SpectatorCameraConfig
from uerl.renderer import EpisodeRendererConfig, RendererViewConfig
import threading

import gymnasium
import numpy as np
from typing import Dict, Any



# Define the agents

class RobotAgent(UEAgent):
    def combine_processed_observations(self, observations: Dict[str, Any]) -> Any:
        return observations["camera_cpu"]



camera = CameraSensorConfig(name="camera",
                                attach_to_comp_name="CameraComp",
                                width=42,
                                height=42,
                                fov=70,
                                use_gpu=False,
                                grayscale=False)


actuator_conf = RobotMovementActuatorConfig(name="my_robot_actuator")
rewarder_pickup_conf = PickupRewarderConfig(name="apples_reward")
terminator_conf = StepCounterTerminatorConfig(name="my_step_terminator",
                                              max_step_count=200)

agent_obs_space = gymnasium.spaces.Box(low=0.0,
                                 high=255.0,
                                 shape=(42, 42, 3),
                                 dtype=np.uint8)

agent_action_space = gymnasium.spaces.MultiDiscrete([2, 2, 2, 2, 2])
agent1_conf = AgentConfig(
    "Agent1", RobotAgent,
    "/CybertoothML/Agents/RobotAgent/BP_RobotAgent.BP_RobotAgent_C", "Agent1",
    agent_obs_space, agent_action_space, [camera], [actuator_conf],
    [rewarder_pickup_conf], [terminator_conf], [])


rewarder_avoid_apples = PickupRewarderConfig(name="apples_reward", invert_reward=True)

agent2_conf = AgentConfig(
    "Agent2", RobotAgent,
    "/CybertoothML/Agents/RobotAgent/BP_RobotAgent.BP_RobotAgent_C", "Agent2",
    agent_obs_space, agent_action_space, [camera], [actuator_conf],
    [rewarder_avoid_apples], [terminator_conf], [])

launch_settings = UELaunchSettings(map_name="CollectApplesMap", 
                                   output_ue_log=False,
                                   render_type=RenderType.NO_RENDER,
                                   executable_path="/mnt/projects/uerl/games/collect_apples/Linux/RLExamples.sh" # This has to be a shared path on ALL NODES in the Ray cluster. We could put it on Ray store, but the overhead will be greater than using an SMB or similar
                                   )
ue_conf = UEEnvConfig(launch_settings, [agent1_conf, agent2_conf], None)
ue_ray_conf = ue_conf.create_ray_wrapper()


# Define two policies
policies = {
    "policy1": (None, agent1_conf.observation_space, agent1_conf.action_space, {}),
    "policy2": (None, agent2_conf.observation_space, agent2_conf.action_space, {})
}

def policy_mapping_fn(agent_id, episode, worker, **kwargs):
    # Make sure agent ID is valid.
    assert agent_id in ["Agent1", "Agent2"], f"ERROR: invalid agent ID {agent_id}!"
    # Map agent1 to policy1, and agent2 to policy2.
    if agent_id == "Agent1":
        return "policy1"
    return "policy2"



# Define our large-scale training config
# Let's start with 1024 workers 
config = (
    PPOConfig()
            .rollouts(num_rollout_workers=1024, create_env_on_local_worker=False, num_envs_per_worker=1)
            .resources(num_gpus=2, num_cpus_for_local_worker=2, num_cpus_per_worker=1, custom_resources_per_worker = {"rollout_space": 1})
            .training(  lr = 3e-4,  #  3e-4, #  3e-4,
                        train_batch_size = 1024 * 200 * 3, # Allow each worker to do 3 full epsiodes (each episode is 200 steps)
                        sgd_minibatch_size = 1024,          # type: ignore
                        num_sgd_iter = 30,  # type: ignore
                        gamma = 0.99,     # type: ignore
                        lambda_ = 0.98, # type: ignore
                        clip_param = 0.2,   # type: ignore
                        vf_clip_param = 10, # type: ignore
                        entropy_coeff = 0.0, # type: ignore
                        vf_loss_coeff = 0.5, # type: ignore
                        grad_clip = 0.5, # max_grad_norm in SB3
                        kl_target = 0.01, # type: ignore
                        model={
                            "post_fcnet_hiddens": [64, 64],
                        }
            )
            .environment(env=UEEnvMultiAgent, env_config=ue_ray_conf)
            .multi_agent(policies=policies, policy_mapping_fn=policy_mapping_fn)
)








class RenderVideoCallback(Callback):
    def on_checkpoint(
        self,
        iteration: int,
        trials,
        trial,
        checkpoint,
        **info,
    ):

        # Run the rendering process in a separate thread
        threading.Thread(target=self._render_video, args=(checkpoint, iteration, trial)).start()

    def _render_video(self, checkpoint, iteration: int, trial):
        alg, alg_config, state = get_alg_for_rendering(checkpoint.path)

        save_video_to_tensorboard = True
        
        tensorboard_logdir=None
        tensorboard_trial_name=None
        step_count=0
        if save_video_to_tensorboard:
            tensorboard_logdir="/mnt/projects/uerl/python/ray_results2/collect_apples"
            tensorboard_trial_name=trial.relative_logdir
            
            step_count = iteration # Use iteration as a backup if global_timestep is not found
            try:
                first_policy_key = next(iter(state["worker"]["policy_states"]))
                step_count = state["worker"]["policy_states"][first_policy_key]["global_timestep"]
            except:
                pass


        spectator_camera_cpu_conf = SpectatorCameraConfig(name="spectator_camera_cpu",
                                        attach_to_comp_name="",
                                        attach_to_actor_name="SpecCameraActor",
                                        width=512,
                                        height=512,
                                        fov=70,
                                        use_gpu=False,
                                        grayscale=False)

        alg_config.env_config["ue_conf"].agent_configs[0].infos_configs.append(spectator_camera_cpu_conf)

        overview_view = RendererViewConfig(
            title="Overview",
            agent_id="Agent1",
            base_image_comp="spectator_camera_cpu",
        )

        agent1_view = RendererViewConfig(
            title="Agent 1",
            agent_id="Agent1",
            base_image_comp="camera_cpu",
            obs_image_comp="camera_cpu",
            display_texts_components=["apples_reward"]
        )

        agent2_view = RendererViewConfig(
            title="Agent 2",
            agent_id="Agent2",
            base_image_comp="camera_cpu",
            obs_image_comp="camera_cpu",
            display_texts_components=["apples_reward"]
        )

        episode_renderer_config = EpisodeRendererConfig(
            views=[overview_view, agent1_view, agent2_view],
            tensorboard_logdir=tensorboard_logdir,
            tensorboard_trial_name=tensorboard_trial_name,
            step_count=step_count,
        )

        alg_config.env_config["ue_conf"].episode_renderer_config = episode_renderer_config

        env = UEEnvMultiAgent(alg_config.env_config)

        obs, info = env.reset()
        while True:
            actions = alg.compute_actions(obs)
            obs, rewards, terminate, truncated, infos = env.step(actions) # type: ignore
            if terminate["__all__"] or truncated["__all__"]:
                break

        env.close()


# Postprocess the perturbed config to ensure it's still valid
def explore(config):
    # ensure we run at least one sgd iter
    if config["num_sgd_iter"] < 1:
        config["num_sgd_iter"] = 1
    return config

hyperparam_mutations = {
        "lambda": lambda: random.uniform(0.7, 1.0),
        "clip_param": lambda: random.uniform(0.01, 0.5),
        "lr": [1e-3, 5e-4, 1e-4, 5e-5, 1e-5],
        "num_sgd_iter": lambda: random.randint(1, 50),
        #"sgd_minibatch_size": lambda: random.randint(50, 1000),
        #"train_batch_size": lambda: random.randint(1000, 5000),
    }


from ray.tune.schedulers import PopulationBasedTraining

pbt = PopulationBasedTraining(
    time_attr="training_iteration",
    perturbation_interval=20,
    resample_probability=0.25,
    # Specifies the mutations of these hyperparams
    hyperparam_mutations=hyperparam_mutations,
    custom_explore_fn=explore,
)


tune.Tuner(
        "PPO",
        tune_config=tune.TuneConfig(
            metric="episode_reward_mean",
            mode="max",
            scheduler=pbt,
            num_samples=-1,
            max_concurrent_trials=32,
            reuse_actors=False
        ),
        run_config=air.RunConfig(
            name="collect_apples",
            stop={"episode_reward_mean": 800},
            verbose=3,
            checkpoint_config=air.CheckpointConfig(
                checkpoint_score_attribute="mean_accuracy",
                num_to_keep=3,
                checkpoint_frequency=20,
            ),
            storage_path="/mnt/projects/uerl/python/ray_results/collect_apples", # This also need to be shared across all nodes
            callbacks=[RenderVideoCallback()],
        ),
        param_space=config,
    ).fit()