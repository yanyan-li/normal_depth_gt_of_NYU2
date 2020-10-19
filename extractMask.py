#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
import scipy.io as sio
import h5py
import os 
from PIL import Image
f=h5py.File("masks.mat")
maskData=f["masks"]
images=np.array(maskData)

 
rgb_path_converted='mask/'
if not os.path.isdir(rgb_path_converted):
    os.makedirs(rgb_path_converted)

images_number=[]
for i in range(len(maskData)):
    images_number.append(images[i])
    mask = np.uint8(images_number[i])*255
    new_mask = Image.fromarray(mask).convert('P')
    img = new_mask.transpose(Image.ROTATE_270)
    iconpath = rgb_path_converted + str(i) + '.png'
    img.save(iconpath, optimize=True)


