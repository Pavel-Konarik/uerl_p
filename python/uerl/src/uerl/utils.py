from IPython.display import display
from PIL import Image
import numpy as np
from typing import Tuple, Optional
import socket

def display_image_in_jupyter(image_array, target_size: Optional[Tuple[int, int]] = None):
    """
    Displays a NumPy array representing an image using PIL and IPython display.
    If the array has 1 channel, displays it in grayscale. If it has 3 channels, displays it in RGB.
    If target_size is provided, scales the image to the given width and height.
    
    :param image_array: NumPy array of the image to be displayed
    :param target_size: Optional tuple (width, height) to resize the image
    """
    if image_array.shape[-1] == 1:
        # Grayscale image
        image = Image.fromarray(image_array.squeeze(), mode='L')
    elif image_array.shape[-1] == 3:
        # RGB image
        image = Image.fromarray(np.uint8(image_array))
    else:
        raise ValueError("Invalid number of channels: expected 1 or 3")

    # Resize the image if target_size is provided
    if target_size:
        image = image.resize(target_size)

    display(image)




def find_unused_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('', 0))
        return s.getsockname()[1]