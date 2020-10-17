#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
import scipy.io as sio
import h5py
import os 
from PIL import Image
f=h5py.File("norm_gt_l.mat")
normal=f["norm_gt_l"]
images=np.array(normal)

 
rgb_path_converted='normal/'
if not os.path.isdir(rgb_path_converted):
    os.makedirs(rgb_path_converted)

print(len(normal))


images_number=[]
for i in range(len(normal)):
    images_number.append(normal[i])
    a=np.array(images_number[i])
    r = Image.fromarray(128*(a[2]+1)).convert('L')
    g = Image.fromarray(128*(a[1]+1)).convert('L')
    b = Image.fromarray(128*(-a[0]+1)).convert('L')
    img = Image.merge("RGB", (r, g, b))
    img = img.transpose(Image.ROTATE_270)
    iconpath=rgb_path_converted+str(i)+'.png'
    img.save(iconpath,optimize=True)
