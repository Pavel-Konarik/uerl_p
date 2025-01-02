import os
from enum import Enum
from subprocess import DEVNULL, PIPE, STDOUT, Popen, check_call
from typing import Dict, List

from uerl.types import RenderType


class UERunner:
    @staticmethod
    def run(map_name:str, ws_port: int, render_type: RenderType, resx: int, resy: int,
            output_ue_log: bool, file_path: str):
        args = f"-unattended -nosound -ws_port {ws_port}"
        if render_type == RenderType.NO_RENDER:
            args = "-nullrhi -onethread -NoVSync -ReduceThreadUsage " + args
        elif render_type == RenderType.GPU_ON_SCREEN:
            args = args + f" -WINDOWED -ResX={resx} -ResY={resy}"
        elif render_type == RenderType.GPU_OFF_SCREEN:
            args = args + f" -RenderOffscreen -WINDOWED -ResX={resx} -ResY={resy}"
        else:
            raise Exception(f"Unknown RenderType {render_type}")

        commandline = f'{file_path} {map_name} {args}'
        print(f'Launching UnrealEngine instance with commandline:\n\t {commandline}')

        if output_ue_log:
            return Popen(commandline.split(" "))
        return Popen(commandline.split(" "), stdout=DEVNULL)
