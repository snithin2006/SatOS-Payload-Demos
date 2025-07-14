import sys
import torch
import segmentation_models_pytorch as smp
from torchvision import transforms
from PIL import Image
import os

MODEL_PATH = os.path.join(os.path.dirname(__file__), "unet_cloud_best.pth")
THRESHOLD = 0.3  # 30% cloud coverage

def preprocess_image(image_path, size=(256, 256)):
    image = Image.open(image_path).convert("RGB")
    transform = transforms.Compose([
        transforms.Resize(size),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])
    return transform(image).unsqueeze(0)

def main():
    if len(sys.argv) != 2:
        print("Usage: python cloud_filter.py <image_path>")
        sys.exit(1)
        
    image_path = sys.argv[1]
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = smp.Unet(
        encoder_name="resnet34",
        encoder_weights=None,
        in_channels=3,
        classes=1,
        activation=None
    )
    
    model.load_state_dict(torch.load(MODEL_PATH, map_location=device))
    model.eval()
    model.to(device)

    image_tensor = preprocess_image(image_path).to(device)
    with torch.no_grad():
        output = model(image_tensor)
        mask = torch.sigmoid(output).cpu().squeeze().numpy()
        cloud_fraction = (mask > 0.5).mean()
        if cloud_fraction > THRESHOLD:
            print("cloudy")
        else:
            print("ok")

if __name__ == "__main__":
    main()
