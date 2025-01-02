from __future__ import annotations
from uerl.components.component_base import TerminatorBase, TerminatorConfig
from dataclasses import dataclass


class ActorsOfClassFinishedTerminator(TerminatorBase):
    def __init__(self, config: ActorsOfClassFinishedTerminatorConfig) -> None:
        super().__init__(config)


@dataclass
class ActorsOfClassFinishedTerminatorConfig(TerminatorConfig):
    actors_class_to_check: str = "" 
    """ For example: '/CollectApples/Blueprints/BP_AppleReward.BP_AppleReward_C'"""
    # Mandatory parent config
    python_class = ActorsOfClassFinishedTerminator
    ue_class_name = "actors_finished"