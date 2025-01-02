import os
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union
import numpy as np
import imageio
import random
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
import shutil
import datetime
import random
import string

from dataclasses import asdict, dataclass

from uerl.agent.agent_base import UEAgent



@dataclass
class RendererViewConfig():
    title:str
    agent_id:str
    base_image_comp:str # Name of the component that provides the base (background) image
    obs_image_comp:Optional[str] = None # Name of the component that provides the observation image (that small image in the top right corner)
    display_texts_components:Optional[List[str]] = None # A list of components to gather the texts to display on the view
    

@dataclass
class EpisodeRendererConfig():
    views:List[RendererViewConfig]
    render_every_nth_episode:int = 1
    track_rewards:bool = True
    base_image_shape:Tuple[int, int, int] = (512,512,3)

    tensorboard_logdir:Optional[str] = None
    tensorboard_trial_name:Optional[str] = None
    step_count:Optional[int] = None
    



class View():
    def __init__(self, name:str, base_img:Image.Image | None, obs_img:Image.Image | None, texts:List[str], target_size:Tuple[int, int, int] = (512,512,3)) -> None:
        self.name = name
        self.base_img:Image.Image | None = base_img
        self.obs_img:Image.Image | None = obs_img
        self.texts:List[str] = texts
        self.target_size:Tuple[int, int, int] = target_size
    
    def get_frame(self, render_text:bool = True) -> np.ndarray:
        # Returns actual frame or black frame. Filename includes extension
        assert self.base_img is not None or self.obs_img is not None
        
        img = None
        if self.base_img is not None:
            img = self.base_img
        else:
            img = self.obs_img

        if img is None:
            img = Image.new('RGB', self.target_size[:2], color = 0)

        # Ensure target size
        img = img.resize(self.target_size[:2])

        # We have both views, add obs img to the top right corner
        if self.base_img and self.obs_img:
            # Add observation view to the top right corner
            # Calculate the coordinates of the top right corner of the main image
            self.obs_img = self.obs_img.resize((150,150))
            x = img.size[0] - self.obs_img.size[0]
            y = 0

            # TODO: hardcoded

            img.paste(self.obs_img, (x, y))


        if render_text:
            # Render the view name
            I1 = ImageDraw.Draw(img)
            font_size = int((35 / 512) * np.max(self.target_size[:2]))
            font_path = os.path.join(Path(__file__).parent, 'fonts', 'OpenSans-Bold.ttf')
            font = ImageFont.truetype(font_path, font_size)
            offset = (28 / 512 * self.target_size[0], 36 / 512 * self.target_size[1])
            I1.text(offset, self.name, fill=(50, 50, 50), font=font)
            

            additonal_lines = "\n".join(self.texts)

            # Render all other lines provided by the texts variable
            font_size = int((30 / 512) * np.max(self.target_size[:2]))
            font = ImageFont.truetype(font_path, font_size)
            offset = (28 / 512 * self.target_size[0], 90 / 512 * self.target_size[1])
            I1.text(offset, additonal_lines, fill=(50, 50, 50), font=font)


        return np.array(img)[:,:,:3]


class EpisodeVideoRenderer():
    def __init__(self, view_configs:List[RendererViewConfig], base_image_shape=(512,512,3), track_rewards:bool = True, render_every_nth_episode:int = 1,
                    tensorboard_logdir:Optional[str]=None,
                    tensorboard_trial_name:Optional[str]=None,
                    step_count:Optional[int]=None) -> None:
        """_summary_

        Args:
            base_image_shape (tuple, optional): _description_. Defaults to (512,512,3).
        """
            
        # Shape of each view frame
        self.frame_shape = base_image_shape
        self.frame_dtype = np.uint8

        self.view_configs = view_configs
        self.view_names:List[str] = [view.title for view in view_configs]

        # Tensorboard stuff
        self.tensorboard_logdir = tensorboard_logdir
        self.tensorboard_trial_name = tensorboard_trial_name    
        self.step_count = step_count
        self.all_frames:List[np.ndarray] = []

        # Calculate the grid dimensions based on the number of images
        self.grid_cols = int(np.ceil(np.sqrt(len(self.view_names))))
        self.grid_rows = int(np.ceil(len(self.view_names) / self.grid_cols))
        # Size in pixels of the grid view
        self.grid_width = self.grid_cols * self.frame_shape[0]
        self.grid_height = self.grid_rows * self.frame_shape[1]

        # Should we display and track rewards over the episode
        self.track_rewards:bool = track_rewards
        self.cum_rewards:Dict[str, float] = {}

        ## Episode variables
        self.current_video = None
        # Agent id -> view
        self.current_step_views:Dict[str, View] = {}


        self.render_every_nth_episode = render_every_nth_episode
        self.episode_count = 0

    @staticmethod
    def from_config(config:EpisodeRendererConfig):
        return EpisodeVideoRenderer(view_configs=config.views, track_rewards=config.track_rewards,
            render_every_nth_episode=config.render_every_nth_episode,
            base_image_shape=config.base_image_shape,
            tensorboard_logdir=config.tensorboard_logdir,
            tensorboard_trial_name=config.tensorboard_trial_name,
            step_count=config.step_count
          )

    def reset(self):
        self.all_frames = []
        self.episode_count += 1

        if self.is_recording_episode():
            # Episode name
            episode_name = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            random_chars = ''.join(random.choices(string.ascii_lowercase + string.digits, k=5))
            episode_name = episode_name + "_" + random_chars

            print(f"Starting recording episode {episode_name}")

            video_path = "videos/{}.mp4".format(episode_name)
            if not os.path.exists("videos/"):
                os.makedirs("videos/")

            self.current_video = imageio.get_writer(video_path, fps=24)
            self.current_step_views = {}
            self.cum_rewards = {}

    def step(self, agents:Dict[str, UEAgent], terminated:Dict, truncated:Dict):
        if self.is_recording_episode():
            for view in self.view_configs:
                agent = agents[view.agent_id]

                base_img = None
                obs_img = None

                base_comp = agent.get_component_by_name(view.base_image_comp)
                assert base_comp is not None
                base_img = base_comp.get_debug_image()

                obs_comp = None
                if view.obs_image_comp:
                    obs_comp = agent.get_component_by_name(view.obs_image_comp)
                    assert obs_comp is not None
                    obs_img = obs_comp.get_debug_image()

                display_texts = []
                if view.display_texts_components:
                    for comp_name in view.display_texts_components:
                        comp = agent.get_component_by_name(comp_name)
                        assert comp is not None
                        display_texts += comp.get_debug_texts()

                self.add_view_step(view.title, base_img, obs_img, 0.0, terminated[view.agent_id], display_texts)

            # Perform rendering tick
            frame = self.get_grid_frame()

            self.render_frame_to_video(frame)

            if self.is_outputing_to_tensorboard():
                # Log the frame to tensorboard
                self.all_frames.append(frame)


            # Inform renderer that episode ended
            if terminated["__all__"] or truncated["__all__"]:
                self.on_episode_end()

                if self.is_outputing_to_tensorboard():
                    from torch.utils.tensorboard.writer import SummaryWriter
                    import torch

                    # Create a SummaryWriter instance (it will create a new experiment)
                    writer = SummaryWriter(self.tensorboard_logdir)

                    all_frames_tensors = [torch.from_numpy(frame) for frame in self.all_frames]

                    video_tensor = torch.stack(all_frames_tensors).unsqueeze(0)
                    video_tensor = video_tensor.permute(0, 1, 4, 2, 3)

                    # tensorboard takes video_array of shape (B,T,C,H,W)
                    writer.add_video(
                        tag=self.tensorboard_trial_name,
                        vid_tensor=video_tensor,
                        global_step=self.step_count,
                        fps=24)

                    writer.close()




    def is_outputing_to_tensorboard(self) -> bool:
        return self.tensorboard_logdir is not None and self.tensorboard_trial_name is not None and self.step_count is not None

    def is_recording_episode(self) -> bool:
        return self.episode_count % self.render_every_nth_episode == 0

    def on_episode_end(self):
        print("Epsidoe end")

        # Just close the video writer
        if self.is_recording_episode() and self.current_video:
            print("Closing video")
            self.current_video.close()


    def add_view_step(self, view_name:str, base_image: Image.Image | None, observation: Image.Image | None, reward:float, done:bool, display_texts:Optional[List[str]] = None):
    
        if self.is_recording_episode():
            assert view_name in self.view_names

            # Because Python creates a persistant display_texts for defaults, we don't want to overwrite it
            display_texts = display_texts or []
            # Handle rewards
            if self.track_rewards:
                # Track cumulative sum of reward
                self.add_reward(view_name, reward)
                # Display it rendered video
                display_texts.append(f"Reward: {self.cum_rewards[view_name]:.3f}")

            view = View(view_name, base_image, observation, display_texts)
            self.current_step_views[view_name] = view
        
        
    def add_reward(self, view_name:str, reward:float):
        assert view_name in self.view_names
        if view_name not in self.cum_rewards:
            self.cum_rewards[view_name] = 0
        
        self.cum_rewards[view_name] += reward

    def render_frame_to_video(self, frame):
        # Get frames from all collected views
        # Render them to file
        
        if self.is_recording_episode():
            assert self.current_video
            self.current_video.append_data(frame)
            # Consume the views
            self.current_step_views = {}
        


    def get_black_grid_frame(self) -> np.ndarray:
        return np.zeros((self.grid_height, self.grid_width, self.frame_shape[2]), dtype=self.frame_dtype)
         

    def get_grid_frame(self) -> np.ndarray:
        """ frame_filename includes extension """
        grid_frame = self.get_black_grid_frame()

        # Populate the grid with the images
        for view in self.current_step_views.values():
            # Get the numerical index of view name
            i = self.view_names.index(view.name)
            # Calculate the row and column of this image
            row = i // self.grid_cols
            col = i % self.grid_cols
            
            # Calculate the coordinates of the top-left corner of this cell
            x = col * self.frame_shape[1]
            y = row * self.frame_shape[0]
            
            # Copy the image into the cell
            grid_frame[y:y+view.target_size[0], x:x+view.target_size[1], :] = view.get_frame()
        return grid_frame
    




