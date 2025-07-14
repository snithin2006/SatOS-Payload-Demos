import os
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from tqdm import tqdm
import segmentation_models_pytorch as smp

from cloud_dataset import create_data_loaders

# Config
DATA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../data'))
BATCH_SIZE = 8
NUM_EPOCHS = 20
LEARNING_RATE = 1e-3
DEVICE = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
MODEL_SAVE_PATH = 'unet_cloud_best.pth'

def main():
    # 1. Create data loaders with num_workers=0 to avoid multiprocessing issues
    train_loader, val_loader, test_loader = create_data_loaders(DATA_DIR, batch_size=BATCH_SIZE)

    # 2. Instantiate U-Net model (SMP)
    model = smp.Unet(
        encoder_name="resnet34",        # You can change encoder
        encoder_weights="imagenet",     # Use ImageNet pre-trained weights
        in_channels=3,
        classes=1,                      # Binary segmentation
        activation=None                 # We'll use BCEWithLogitsLoss
    )
    model = model.to(DEVICE)

    # 3. Loss, optimizer, metrics
    criterion = nn.BCEWithLogitsLoss()
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)

    # Simple IoU metric for binary masks
    def iou_score(preds, targets, threshold=0.5):
        preds = torch.sigmoid(preds)
        preds = (preds > threshold).float()
        intersection = (preds * targets).sum(dim=(1,2,3))
        union = ((preds + targets) > 0).float().sum(dim=(1,2,3))
        iou = (intersection + 1e-6) / (union + 1e-6)
        return iou.mean().item()

    # 4. Training loop
    def train_one_epoch(model, loader, criterion, optimizer, device):
        model.train()
        running_loss = 0.0
        running_iou = 0.0
        for images, masks in tqdm(loader, desc='Train', leave=False):
            images = images.to(device)
            masks = masks.to(device)
            optimizer.zero_grad()
            outputs = model(images)
            loss = criterion(outputs, masks)
            loss.backward()
            optimizer.step()
            running_loss += loss.item() * images.size(0)
            running_iou += iou_score(outputs, masks) * images.size(0)
        epoch_loss = running_loss / len(loader.dataset)
        epoch_iou = running_iou / len(loader.dataset)
        return epoch_loss, epoch_iou

    def eval_one_epoch(model, loader, criterion, device):
        model.eval()
        running_loss = 0.0
        running_iou = 0.0
        with torch.no_grad():
            for images, masks in tqdm(loader, desc='Val', leave=False):
                images = images.to(device)
                masks = masks.to(device)
                outputs = model(images)
                loss = criterion(outputs, masks)
                running_loss += loss.item() * images.size(0)
                running_iou += iou_score(outputs, masks) * images.size(0)
        epoch_loss = running_loss / len(loader.dataset)
        epoch_iou = running_iou / len(loader.dataset)
        return epoch_loss, epoch_iou

    # 5. Main training loop
    best_val_iou = 0.0
    for epoch in range(NUM_EPOCHS):
        print(f"Epoch {epoch+1}/{NUM_EPOCHS}")
        train_loss, train_iou = train_one_epoch(model, train_loader, criterion, optimizer, DEVICE)
        val_loss, val_iou = eval_one_epoch(model, val_loader, criterion, DEVICE)
        print(f"  Train Loss: {train_loss:.4f}, IoU: {train_iou:.4f}")
        print(f"  Val   Loss: {val_loss:.4f}, IoU: {val_iou:.4f}")
        # Save best model
        if val_iou > best_val_iou:
            best_val_iou = val_iou
            torch.save(model.state_dict(), MODEL_SAVE_PATH)
            print(f"  Saved new best model to {MODEL_SAVE_PATH}")

    # 6. Evaluate on test set
    print("\nEvaluating best model on test set...")
    model.load_state_dict(torch.load(MODEL_SAVE_PATH))
    test_loss, test_iou = eval_one_epoch(model, test_loader, criterion, DEVICE)
    print(f"Test Loss: {test_loss:.4f}, IoU: {test_iou:.4f}")

if __name__ == '__main__':
    main() 