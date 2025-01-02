from __future__ import annotations
import json
from uerl.agent.agent_base import AgentConfig
from uerl.renderer import EpisodeRendererConfig
from uerl.types import RenderType
from typing import Optional, Type, Dict, Union, List, Tuple, Any

from dataclasses import asdict, dataclass


@dataclass
class UELaunchSettings():
    map_name: str
    render_type: RenderType = RenderType.NO_RENDER
    no_sound: bool = True
    run_name: Optional[str] = "run_name_not_set"
    tickfps: int = 20
    specific_ip_port: Optional[Tuple[str, int]] = ("", -1)
    # If forcing to use a specific IP, make sure to use .rollouts(num_rollout_workers=1, create_env_on_local_worker=False, num_envs_per_worker=1)
    window_res_x: int = 320
    window_res_y: int = 240
    executable_path: str = ""

    # Debugs
    output_ue_log: bool = False


def remove_key(d, key_to_delete):
    if isinstance(d, list):
        for item in d:
            remove_key(item, key_to_delete)
        return

    if not isinstance(d, dict):
        return

    if key_to_delete in d:
        del d[key_to_delete]

    for key in d:
        remove_key(d[key], key_to_delete)


@dataclass
class UEEnvConfig():
    launch_settings: UELaunchSettings
    agent_configs: List[AgentConfig]
    episode_renderer_config: EpisodeRendererConfig | None

    def get_all_agent_ids(self) -> List[str]:
        return [config.agent_id for config in self.agent_configs]

    def get_ue_config(self) -> Dict:
        ue_config = asdict(self)
        # We cannot json serialise python classes and enums
        remove_key(ue_config, "python_class")
        remove_key(ue_config, "render_type")
        remove_key(ue_config, "action_space")
        remove_key(ue_config, "observation_space")
        return ue_config

    def get_ue_human_mode_command(self) -> str:
        return f"ML.HumanStart {json.dumps(self.get_ue_config())}"