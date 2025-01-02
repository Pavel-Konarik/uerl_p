from __future__ import annotations
from dataclasses import dataclass
from uerl.components.component_base import RewarderBase, RewarderConfig


class PickupRewarder(RewarderBase):
    def __init__(self, config: PickupRewarderConfig) -> None:
        super().__init__(config)

    def get_debug_texts(self) -> tuple[str, ...]:
        return () #("PickupRewarder Debug Text 1", "PickupRewarder Debug Text 2")

@dataclass
class PickupRewarderConfig(RewarderConfig):
    # Add PickupRewarderConfig specific configs here
    # Mandatory parent config
    python_class = PickupRewarder
    ue_class_name = "pickup_reward"
