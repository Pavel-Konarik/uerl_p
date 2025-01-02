from __future__ import annotations
from dataclasses import dataclass, field
from uerl.components.component_base import RewarderBase, RewarderConfig
from uerl.types import FVector


class WorldDirectionRewarder(RewarderBase):
    def __init__(self, config: WorldDirectionRewarderConfig) -> None:
        super().__init__(config)


@dataclass
class WorldDirectionRewarderConfig(RewarderConfig):
    direction: FVector = field(default_factory=lambda: FVector(1, 0, 0))
    # Add WorldDirectionRewarderConfig specific configs here
    # Mandatory parent config
    python_class = WorldDirectionRewarder
    ue_class_name = "world_direction"
