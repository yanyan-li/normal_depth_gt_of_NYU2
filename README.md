# Normal/Depth Images Generating from NYU2 (labeled dataset) 

<img src="teaser/0.jpg" alt="0" style="zoom:25%;" /><img src="teaser/0.png" alt="0" style="zoom:25%;"/>

It is fucking struggling to find a suitable pre-processed labeled NYU2 dataset for your DL model. This repo recovers those images that can be evaluated in all types of deep learning frameworks.

A few steps to achieve the goal.  I hope it can make your research easier.

**Step 1: download the .mat file**

The NYUv2 dataset images can be downloaded from the [NYU website](https://cs.nyu.edu/~silberman/datasets/nyu_depth_v2.html). We use the 1449 images (795 images for training and 654 images for testing).

**Step 2: generate RGB and depth images**

```
python extractImages.py
```

Please make sure that the .mat file is in the same folder with the extractImages.py. 

```
# The following libs are needed.
numpy 
matplotlib.pyplot 
scipy.io 
h5py
PIL 
```

**Step 3: generate normal maps and planar masks**

