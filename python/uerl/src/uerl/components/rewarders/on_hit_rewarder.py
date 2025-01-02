from __future__ import annotations
from dataclasses import dataclass, field
from uerl.components.component_base import RewarderBase, RewarderConfig
from uerl.types import FVector


class OnHitRewarder(RewarderBase):
    def __init__(self, config: OnHitRewarderConfig) -> None:
        super().__init__(config)


@dataclass
class OnHitRewarderConfig(RewarderConfig):
    on_hit_reward: float = -10.0
    # Add OnHitRewarderConfig specific configs here
    # Mandatory parent config
    python_class = OnHitRewarder
    ue_class_name = "on_hit_rewarder"
