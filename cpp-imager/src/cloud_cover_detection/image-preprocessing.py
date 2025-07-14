import os
import numpy as np
from PIL import Image

base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../data"))
input_img_dir = os.path.join(base_dir, "images")
input_mask_dir = os.path.join(base_dir, "masks")
output_img_dir = os.path.join(base_dir, "processed_images")
output_mask_dir = os.path.join(base_dir, "processed_masks")

os.makedirs(output_img_dir, exist_ok=True)
os.makedirs(output_mask_dir, exist_ok=True)

for i in range(1, 101):
    img = np.array(Image.open(f"{input_img_dir}/image_{i}.png")).astype(np.float32)
    img = img / 255.0
    Image.fromarray((img * 255).astype(np.uint8)).save(f"{output_img_dir}/image_{i}.png")

    mask = np.array(Image.open(f"{input_mask_dir}/mask_{i}.png")).astype(np.float32)
    if mask.ndim == 3:
        mask = mask[..., 0]
    binary_mask = (mask > 127).astype(np.uint8)
    Image.fromarray((binary_mask * 255).astype(np.uint8)).save(f"{output_mask_dir}/mask_{i}.png")