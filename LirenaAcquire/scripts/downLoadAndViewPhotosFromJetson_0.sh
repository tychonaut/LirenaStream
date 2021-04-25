#!/bin/bash

# download all instansave images (rightCtrl + Enter)
scp -r nvidia@192.168.0.90:/home/nvidia/Documents/instantsave/ /home/lighthouse/Pictures/photosDownloadedFromJetson/

#start image viewer (Eye Of Gnome)
 eog /home/lighthouse/Pictures/photosDownloadedFromJetson/instantsave
