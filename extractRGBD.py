#!/usr/bin/env python2
# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
import scipy.io as sio
import h5py
import os 
from PIL import Image
f=h5py.File("nyu_depth_v2_labeled.mat")
images=f["images"]
labels=f["labels"]
depths=f["depths"]
images=np.array(images)
depths=np.array(depths)
 
rgb_path_converted='NYU2/RGB/'
depth_path_converted = 'NYU2/depth/'
if not os.path.isdir(rgb_path_converted):
    os.makedirs(rgb_path_converted)
if not os.path.isdir(depth_path_converted):
    os.makedirs(depth_path_converted)
 

max = depths.max()
print(depths.shape)
print(depths.max())
print(depths.min())
 
depths = depths / max * 255
depths = depths.transpose((0,2,1))
 
print(depths.max())
print(depths.min())
print(len(depths))
for i in range(len(depths)):
    print(str(i) + '.png')
    #im = Image.fromarray(np.flipud(img_data))
    depths_img= Image.fromarray(np.uint8(depths[i]))
    depths_img = depths_img.transpose(Image.FLIP_LEFT_RIGHT)
    iconpath=depth_path_converted + str(i)+'.png'
    depths_img.save(iconpath, 'PNG', optimize=True)


images_number=[]
for i in range(len(images)):
    images_number.append(images[i])
    a=np.array(images_number[i])
    r = Image.fromarray(a[0]).convert('L')
    g = Image.fromarray(a[1]).convert('L')
    b = Image.fromarray(a[2]).convert('L')
    img = Image.merge("RGB", (r, g, b))
    img = img.transpose(Image.ROTATE_270)
    iconpath=rgb_path_converted+str(i)+'.jpg'
    img.save(iconpath,optimize=True)
