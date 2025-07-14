import os
import torch
from torch.utils.data import Dataset
from PIL import Image
import torchvision.transforms as transforms
import numpy as np
from pathlib import Path


class CloudDetectionDataset(Dataset):
    """
    PyTorch Dataset for cloud detection using image-mask pairs.
    
    This dataset loads satellite images and their corresponding cloud masks,
    applies transformations, and returns them as PyTorch tensors.
    """
    
    def __init__(self, data_dir, transform=None, target_size=(256, 256)):
        """
        Initialize the dataset.
        
        Args:
            data_dir (str): Path to the data directory containing 'images' and 'masks' subdirectories
            transform (callable, optional): Additional transformations to apply
            target_size (tuple): Target size for images and masks (width, height)
        """
        self.data_dir = Path(data_dir)
        self.transform = transform
        self.target_size = target_size
        
        # Define base transforms for images and masks
        self.image_transform = transforms.Compose([
            transforms.Resize(target_size),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
        ])
        
        self.mask_transform = transforms.Compose([
            transforms.Resize(target_size, interpolation=transforms.InterpolationMode.NEAREST),
            transforms.ToTensor()
        ])
        
        # Get all image and mask file paths
        self.image_dir = self.data_dir / "processed_images"
        self.mask_dir = self.data_dir / "processed_masks"
        
        # Get sorted list of image files
        self.image_files = sorted([f for f in self.image_dir.glob("*.png")])
        self.mask_files = sorted([f for f in self.mask_dir.glob("*.png")])
        
        # Verify we have matching pairs
        if len(self.image_files) != len(self.mask_files):
            raise ValueError(f"Number of images ({len(self.image_files)}) doesn't match number of masks ({len(self.mask_files)})")
        
        print(f"Loaded {len(self.image_files)} image-mask pairs from {data_dir}")
    
    def __len__(self):
        """Return the total number of samples in the dataset."""
        return len(self.image_files)
    
    def __getitem__(self, idx):
        """
        Load and return a single sample (image, mask pair).
        
        Args:
            idx (int): Index of the sample to load
            
        Returns:
            tuple: (image_tensor, mask_tensor) where both are PyTorch tensors
        """
        # Load image and mask
        image_path = self.image_files[idx]
        mask_path = self.mask_files[idx]
        
        # Load as PIL Images
        image = Image.open(image_path).convert('RGB')
        mask = Image.open(mask_path).convert('L')  # Grayscale for binary mask
        
        # Apply base transforms
        image_tensor = self.image_transform(image)
        mask_tensor = self.mask_transform(mask)
        
        # Ensure mask is binary (0 or 1)
        mask_tensor = (mask_tensor > 0.5).float()
        
        return image_tensor, mask_tensor
    
    def get_sample_info(self, idx):
        """
        Get information about a specific sample without loading the full data.
        
        Args:
            idx (int): Index of the sample
            
        Returns:
            dict: Information about the sample
        """
        if idx >= len(self):
            raise IndexError(f"Index {idx} out of range for dataset of size {len(self)}")
        
        image_path = self.image_files[idx]
        mask_path = self.mask_files[idx]
        
        return {
            'image_path': str(image_path),
            'mask_path': str(mask_path),
            'image_name': image_path.name,
            'mask_name': mask_path.name
        }


def create_data_loaders(data_dir, batch_size=8, train_split=0.7, val_split=0.15, test_split=0.15, shuffle=True):
    """
    Create train, validation, and test data loaders.
    
    Args:
        data_dir (str): Path to the data directory
        batch_size (int): Batch size for training
        train_split (float): Fraction of data for training
        val_split (float): Fraction of data for validation
        test_split (float): Fraction of data for testing
        shuffle (bool): Whether to shuffle the data
        
    Returns:
        tuple: (train_loader, val_loader, test_loader)
    """
    from torch.utils.data import DataLoader, random_split
    
    # Create the full dataset
    full_dataset = CloudDetectionDataset(data_dir)
    
    # Calculate split sizes
    total_size = len(full_dataset)
    train_size = int(train_split * total_size)
    val_size = int(val_split * total_size)
    test_size = total_size - train_size - val_size
    
    # Split the dataset
    train_dataset, val_dataset, test_dataset = random_split(
        full_dataset, [train_size, val_size, test_size]
    )
    
    # Create data loaders
    train_loader = DataLoader(
        train_dataset, 
        batch_size=batch_size, 
        shuffle=shuffle,
        num_workers=0,  # Set to 0 to avoid multiprocessing issues on macOS
        pin_memory=False  # Disable pin_memory on macOS
    )
    
    val_loader = DataLoader(
        val_dataset, 
        batch_size=batch_size, 
        shuffle=False,
        num_workers=0,
        pin_memory=False
    )
    
    test_loader = DataLoader(
        test_dataset, 
        batch_size=batch_size, 
        shuffle=False,
        num_workers=0,
        pin_memory=False
    )
    
    print(f"Dataset splits: Train={len(train_dataset)}, Val={len(val_dataset)}, Test={len(test_dataset)}")
    
    return train_loader, val_loader, test_loader


# Example usage and testing
if __name__ == "__main__":
    # Test the dataset
    data_dir = "data"  # Adjust path as needed
    
    try:
        # Create dataset
        dataset = CloudDetectionDataset(data_dir)
        print(f"Dataset size: {len(dataset)}")
        
        # Test loading a sample
        image, mask = dataset[0]
        print(f"Image shape: {image.shape}, dtype: {image.dtype}")
        print(f"Mask shape: {mask.shape}, dtype: {mask.dtype}")
        print(f"Mask unique values: {torch.unique(mask)}")
        
        # Test data loaders
        train_loader, val_loader, test_loader = create_data_loaders(data_dir, batch_size=4)
        
        # Test a batch
        for batch_idx, (images, masks) in enumerate(train_loader):
            print(f"Batch {batch_idx}: Images shape: {images.shape}, Masks shape: {masks.shape}")
            if batch_idx >= 2:  # Just test first few batches
                break
                
    except Exception as e:
        print(f"Error testing dataset: {e}")
        print("Make sure your data directory structure is correct:")
        print("data/")
        print("  ├── processed_images/")
        print("  └── processed_masks/") 