from sentinelhub import SHConfig, SentinelHubRequest, DataCollection, MimeType, CRS, BBox
import matplotlib.pyplot as plt
import numpy as np

config = SHConfig()

config.sh_client_id = 'c2b509b2-88ae-4f1e-bea6-d44be11dda3c'
config.sh_client_secret = 'IeVZ1W0Znsf5YN0QAWG54qRpsyckNgzs'

lon_min, lon_max = -122.184079, -80.095431
lat_min, lat_max = 32.011063, 48.158852
box_size = 0.5

boxes = []
for _ in range(100):
    left = np.random.uniform(lon_min, lon_max - box_size)
    bottom = np.random.uniform(lat_min, lat_max - box_size)
    right = left + box_size
    top = bottom + box_size
    boxes.append([left, bottom, right, top])

evalscript_true_color = """
//VERSION=3
function setup() {
  return {
    input: ["B04", "B03", "B02"],
    output: { bands: 3 }
  };
}

function evaluatePixel(sample) {
  return [sample.B04, sample.B03, sample.B02];
}
"""

evalscript_clm = """
//VERSION=3
function setup() {
  return {
    input: ["B02", "B03", "B04", "CLM"],
    output: { bands: 3 }
  }
}

function evaluatePixel(sample) {
  if (sample.CLM == 1) {
    return [0.75 + sample.B04, sample.B03, sample.B02]
  }
  return [3.5*sample.B04, 3.5*sample.B03, 3.5*sample.B02];
}
"""

# images
image_idx = 1
mask_idx = 1
for box in boxes:
    bbox = BBox(bbox=box, crs=CRS.WGS84)
    
    request_image = SentinelHubRequest(
        evalscript=evalscript_true_color,
        input_data=[
            SentinelHubRequest.input_data(
                data_collection=DataCollection.SENTINEL2_L1C,
                time_interval=('2024-01-01', '2025-01-01'),
            )
        ],
        responses=[SentinelHubRequest.output_response("default", MimeType.PNG)],
        bbox=bbox,
        size=(512, 512),
        config=config,
    )

    image_data = request_image.get_data()
    plt.imsave(f"data/images/image_{image_idx}.png", image_data[0])
    image_idx += 1
    
    request_clm = SentinelHubRequest(
        evalscript=evalscript_clm,
        input_data=[
            SentinelHubRequest.input_data(
                data_collection=DataCollection.SENTINEL2_L1C,
                time_interval=('2024-01-01', '2025-01-01'),
            )
        ],
        responses=[SentinelHubRequest.output_response("default", MimeType.PNG)],
        bbox=bbox,
        size=(512, 512),
        config=config,
    )
    
    mask_data = request_clm.get_data()
    plt.imsave(f"data/masks/mask_{mask_idx}.png", mask_data[0])
    mask_idx += 1