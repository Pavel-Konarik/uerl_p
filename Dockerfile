FROM nvidia/cuda:12.2.0-base-ubuntu22.04
# Alternatively, you can use the following line to use the latest version of Ubuntu (if you don't need GPU support)
# FROM ubuntu:22.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Etc/UTC

# Update and install necessary packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget \
    bzip2 \
    build-essential \
    git \
    ca-certificates \
    && update-ca-certificates \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Create the user 'uerl' with a home directory
RUN useradd -m -s /bin/bash uerl

# Install Miniconda
RUN wget https://repo.anaconda.com/miniconda/Miniconda3-py312_24.11.1-0-Linux-x86_64.sh -O /tmp/miniconda.sh \
    && bash /tmp/miniconda.sh -b -p /opt/miniconda \
    && rm /tmp/miniconda.sh

# Add conda to PATH
ENV PATH="/opt/miniconda/bin:$PATH"

# Create Python 3.11.8 environment called ray1
RUN conda create -n ray1 python=3.11.8 -y \
    && conda clean -a -y 

# Activate the environment and set it as default
ENV CONDA_DEFAULT_ENV=ray1
ENV PATH="/opt/miniconda/envs/ray1/bin:$PATH"
RUN echo "source activate ray1" > /home/uerl/.bashrc

# Set user ownership for the home directory
RUN chown -R uerl:uerl /home/uerl

# Copy the uerl project to the home folder
COPY python/uerl /home/uerl/uerl_python

RUN chmod -R 777 /home/uerl/uerl_python/src

# Set the working directory
WORKDIR /home/uerl

# Switch to the 'uerl' user
USER uerl

# Install uerl in editable mode (including GPU Torch, in case users want to use it)
RUN pip cache purge
RUN pip install -e /home/uerl/uerl_python[torch]

# Install CPU version of PyTorch (for GPU version, uncomment the line below)
# RUN pip install torch==2.2.1 torchvision==0.17.1 torchaudio==2.2.1 --index-url https://download.pytorch.org/whl/cpu

# Get compiled example games (good for tutorial, but not necessary for using uerl)
# COPY games /home/uerl/games
# RUN python /home/uerl/games/download_games.py

# Copy the demo project to the home folder
COPY epic_demo /home/uerl/epic_demo


CMD ["/bin/bash"]
