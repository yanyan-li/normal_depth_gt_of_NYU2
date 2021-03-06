# Normal/Depth Images Generating from NYU2 (labeled dataset) 

![nyu2](teaser/nyu2.png)

---

It is fucking struggling to find a suitable pre-processed labeled NYU2 dataset for your DL model. This repo recovers those images that can be evaluated in all types of deep learning frameworks.

A few steps to achieve the goal.  I hope it can make your research easier.

**Step 1: download two .mat files**

The NYUv2 dataset images can be downloaded from the [NYU website](https://cs.nyu.edu/~silberman/datasets/nyu_depth_v2.html), which has 1449 images.

The per-pixel normal maps and masks can be downloaded from [here](https://drive.google.com/file/d/1FbOaH0G5_BI6yOjuzgBRcMuKjFSoUWB0/view?usp=sharing) and [here](https://drive.google.com/file/d/1Uo85eWtrTd4BWsyamCuBXP9UiB8MLEPh/view?usp=sharing), respectively. 

**Step 2: generate RGB, depth and per-pixel normal images**

Let *extractRGBD.py*,  *extractNormal.py* and *extractMask.py* be in the same folder with three .mat files.  Please make sure that you installed following libs.

```
# The following libs are needed.
numpy 
matplotlib.pyplot 
scipy.io 
h5py
PIL 
```

Then, run following commends to generate images:

```
python extractRGBD.py

python extractNormal.py

python extractMask.py
```

----

**Step 3: generate plane-based normal maps and planar masks**

This part comes from C++ code, which requires the OpenCV lib. 

RGB and Depth frames have been synchronized. Then we can obtain the intrinsic parameters for normal estimation.  

```
% The maximum depth used, in meters.
maxDepth = 10;

% RGB Intrinsic Parameters
fx_rgb = 5.1885790117450188e+02;
fy_rgb = 5.1946961112127485e+02;
cx_rgb = 3.2558244941119034e+02;
cy_rgb = 2.5373616633400465e+02;

% Parameters for making depth absolute.
depthParam1 = 351.3;
depthParam2 = 1092.5;
```

1. download this repo  **normal_depth_gt_of_NYU2**

```
cd normal_depth_gt_of_NYU
./build_linux.sh 
```

2. extract planes

```
cd NYU2
mkdir mask
mkdir instance
mkdir fittedNormal
cd ..
./RGBDPlaneDetection/build/RGBDPlaneDetect NYU2 NYU2/associate.txt  NYU2/
```

