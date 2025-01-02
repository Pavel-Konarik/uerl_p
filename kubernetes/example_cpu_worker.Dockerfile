FROM rayproject/ray:2.40.0
RUN pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
