from __future__ import annotations
from dataclasses import dataclass, field
from uerl.components.component_base import RewarderBase, RewarderConfig
from uerl.types import FVector


class TargetDistanceRewarder(RewarderBase):
    def __init__(self, config: TargetDistanceRewarderConfig) -> None:
        super().__init__(config)


@dataclass
class TargetDistanceRewarderConfig(RewarderConfig):
    target_location: FVector = field(default_factory=lambda: FVector(1, 0, 0))
    # Add TargetDistanceRewarderConfig specific configs here
    # Mandatory parent config
    python_class = TargetDistanceRewarder
    ue_class_name = "target_distance"
