from __future__ import annotations
from uerl.components.component_base import TerminatorBase, TerminatorConfig
from dataclasses import dataclass


class StepCounterTerminator(TerminatorBase):
    def __init__(self, config: StepCounterTerminatorConfig) -> None:
        super().__init__(config)


@dataclass
class StepCounterTerminatorConfig(TerminatorConfig):
    max_step_count: int = 2000
    # Mandatory parent config
    python_class = StepCounterTerminator
    ue_class_name = "step_counter"