from __future__ import annotations
from uerl.components.component_base import TerminatorBase, TerminatorConfig
from dataclasses import dataclass


class OnHitTerminator(TerminatorBase):
    def __init__(self, config: OnHitTerminatorConfig) -> None:
        super().__init__(config)


@dataclass
class OnHitTerminatorConfig(TerminatorConfig):
    
    # Mandatory parent config
    python_class = OnHitTerminator
    ue_class_name = "on_hit_terminator"