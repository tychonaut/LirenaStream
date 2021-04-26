#!/bin/bash

# This is actually just a text document I write like a bash script 
# for syntax highlight and future ease of turning this into a semi-automated
# script

###############################################################################
###############################################################################
# Install Ubuntu 18.04 on receiver/management machine:

# Ususal stuff of installing a Linux OS:
# Download image, create bootable USB stick (per rufus or whatever),
# select appropritae mode (GPT/MBR), probably enable/disable 
# CSM (Compatibility Support Module) in BIOS of target machine.

# IP config the host device in the range 
# IP: 192.168.0.4x
# Subnet mask: 255.255.255.0
# Current IP setup:
# Lighthouse Streaming host (in the rack that is taken onboard):
#  IP: 192.168.0.41
# Stefan's Laptop:
#  IP: 192.168.0.42
# Markus' Laptop:
#  IP: 192.168.0.43
# Markus' office desktop Workstation:
#  IP: 192.168.0.44


# (Stuff that occured at Markus' home with Vodaphone cable router, may not
# apply to you:)
# If you need internet access, provide the router's IP as default gateway
# (e.g. 192.168.0.1)
# If there are problems, connect to the router and change DHCP properties
# for certain device MAC adresses to manual, maybe also assign the same IP
# they gave themselves manually.





###############################################################################
###############################################################################
# Download and install "Jetpack SDK Manager" (or watever the name is) 
# on host machine:
# On freshly installed Ubuntu 18.04 on x86_64 machine:

# Install Jetpack SDK Manager on host:
sudo dpkg -i installers_images/sdkmanager_0.9.12-4180_amd64.deb 
# On errors about missing packages, install them (sudo aptitude install xxxx)
# There will be an awkward self-updating after installation and fist start.
# Restart SDK manager manually and don't bother about warnings/errors about 
# outdated database.


###############################################################################
# Download "Jetpack" itself to host x86 machine

# Connect Jetson to host for image cloning/flashing:
# Follow
# https://developer.download.nvidia.com/embedded/L4T/r27_Release_v1.0/Docs/Jetson_X2_Developer_Kit_User_Guide.pdf
# , section "Force Recovery Mode"

# Make sure the Jetson is connected either in the SDK GUI (click "refresh")
# or in terminal via 
lsusb | grep nvidia
# There sould be a non-empty output on success.


# _Download_ (!) Jetpack stuff for both host and jetson to host local disk,
#
# BUT DON'T INSTALL JETPACK!
#   OTHERWISE THE JETSON YOU TRY TO SAVE WILL BE OVERWRITTEN!!!!!111
# (To be save, connect a dummy-jetson that wouldn't mean desaster if its flash
# drive were accidently overwritten. Maybe the download proecdure also works
# without any jetson connected via USB, but I haven'T tried):
#:
# Click through the four steps in GUI. Leave everything at default,
# except check "download only, install later" in the end.
# After Downloading, there will be a folder structure in
# ~/nvidia/nvidia_sdk/


###############################################################################
# Install "Jetpack" itself to host x86 machine:

# Reproduce the above steps, but without checking the 
# "download only, install later" button. Either deselect jetson stuff, or flash 
# a dummy jetson, or try if installing tprocedure work wwithout any jetson
# connected. We may need the host jetpack stuff for debugging, profiling, 
# for having a compatible and hassle-free cuda installation etc.



###############################################################################
# Save existing Jetson TX2 image on host storage 
# for backup and distribution purposes:

# Save image from connected "developement Jetson" to host storage:
# Follow the relevent sections in
# https://elinux.org/Jetson/TX2_Cloning


###############################################################################
# Mirror saved image from "development Jetson" to "production jetsons":
# Also follow the relevent sections in
# https://elinux.org/Jetson/TX2_Cloning
 
# The result are almost exact clones of the "developement Jetson", 
# with at least one notable difference: The IP config is set to DHCP.
# In order to assign a manual static IP, connect to the freshly cloned jetson
# via ssh or Display+Keyboard+Mouse and either "ifconfig" your way or use
# the GUI to set a scheme like e.g.
## IP: 192.168.0.9x
# Subnet mask: 255.255.255.0
# Cameras have been decided to be in the 90's IP range.
# At the time of writing, of the seven jetsons, six are flashed and
# assigned IPs P: 192.168.0.90 to P: 192.168.0.95.
# For physical recognition, they ere marked with the last IP decimal digit 
# on their carrier board's SD card slot.
# For Internet access, the same gateway stuff os above (for x86 host machine)
# applies.



###############################################################################
# Fixing some issue after flashing: two cuda libs make dpkg/apt/apt-get/aptitude
# stop working. workarounds:
rm -i /var/lib/dpkg/info/cuda-samples-10-0.list
sudo apt-get install cuda-samples-10-0 --reinstall
rm -i /var/lib/dpkg/info/cuda-documentation-10-0.list
sudo apt-get install cuda-documentation-10-0 --reinstall

# then stuff like 
sudo aptitude update
sudo aptitude upgrade
# should work again

###############################################################################
###############################################################################
# Setup remote access: Teamviewer, SSH, ...


###############################################################################
# Remote access cluster via SSH

# -----------------------------------------------------------------------------
# Your personal machine to Streamer(x86) and Jetsons:

# enable SSH in streaming x86 machine (reqiures password):
# follow:
# https://phoenixnap.com/kb/how-to-enable-ssh-on-ubuntu
# I changed SSh port to 2222 instead of 22, so the '-p 2222' CLI option is
# mandadory for the streamer x86 machine; on jetsons, it is still port 22


# Enable passwordless ssh access on a device (streamer x86 or jetson):
# This is great for automated scripts to deploy, execute update, start and end 
# stuff on each machine of a cluster.
# some ideas from:
# https://askubuntu.com/questions/46930/how-can-i-set-up-password-less-ssh-login

 #generate ssh key on your personal computer (laptop, office PC ...):
ssh-keygen -t ed25519 -C "<Your name>: for SSH into lighthouse machines (streamer x86 and Jetsons)"
# for password-less access to streamer x86:
ssh-copy-id -p 2222 -i ~/.ssh/id_ed25519.pub lighthouse@192.168.0.41
# access via:
ssh -p 2222 lighthouse@192.168.0.41

# for password-less access to a Jetson, e.g. Jetson 0:
ssh-copy-id -p 22 -i ~/.ssh/id_ed25519.pub nvidia@192.168.0.90
# access via:
ssh -p 22  nvidia@192.168.0.90



# -----------------------------------------------------------------------------
# Frome Streamer Machine to Jetsons:

#similar stuff on the streamer machine itself in order to SSH into the jetsons:
# generate ssh key on the  streamer x86 machine:
ssh-keygen -t ed25519 -C "lighthouse-streamer(x86_64): for passwordless SSH into Jetsons"
# copy and access like above for your own machine(s) ...



###############################################################################
# Remote desktop on Streamer (x86):

# -----------------------------------------------------------------------------
# Teamviewer on host: Follow:
# https://linuxize.com/post/how-to-install-teamviewer-on-ubuntu-18-04/
# :
wget https://download.teamviewer.com/download/linux/teamviewer_amd64.deb
# etc.
# Ask Markus Schlueter about login credentials for teamviewer.




###############################################################################
# Remote desktop on Jetsons:

# (N.b. Teamviewer failed pretty hard after messing with it for hours.
# Problem is that there are only ARM binaries for 32 bit to be used on 
# raspberry pi. In 2018, there was a hacky solution of installing 32 bit stuff
# anyway: 
# https://www.myzhar.com/blog/tutorials/teamviewer-14-on-nvidia-jetson-tx2/
# While this may once have worked, a 2020 comment discouraged it.
# After hours of hacking and tricksing, I even managed to remote connect and
# get an image and interact with the remote desktop, but after a few seconds,
# it crashed. Subsequent connectsions always even crashed after about one 
# second. So, VNC + Remmina it is! This is slow, ugly and only works on local 
# networks, but at least it works.
#)


#------------------------------------------------------------------------------
# Make Remote desktop on Jetsons work without physical (HDMI) display connected:

#enable auto login after reboot:
# on jetson: open system settings
# "user accounts" -> (unlock) -> "Automatic Login" -> ON

# Follow:
#https://askubuntu.com/questions/453109/add-fake-display-when-no-monitor-is-plugged-in
sudo cp /etc/X11/xorg.conf ~/
sudo nano /etc/X11/xorg.conf
# paste missing stuff into file:
Section "Monitor"
   Identifier "DSI-0"
   Option    "Ignore"
EndSection

Section "Screen"
   Identifier    "Default Screen"
   Monitor        "Configured Monitor"
   Device        "Default Device"
   SubSection "Display"
       Depth    24
       Virtual 1680 1050
   EndSubSection
EndSection


#------------------------------------------------------------------------------
# setup VNC on Jetson:

ssh nvidia@192.168.0.9x
#(ask Markus or Stefan for credentials if no SSH key copy has happened yet
#on the particular jetson)
# follow:
# https://developer.nvidia.com/embedded/learn/tutorials/vnc-setup :
cd /usr/lib/systemd/user/graphical-session.target.wants
sudo ln -s ../vino-server.service ./.
gsettings set org.gnome.Vino prompt-enabled false
gsettings set org.gnome.Vino require-encryption false
# Replace thepassword with your desired password
gsettings set org.gnome.Vino authentication-methods "['vnc']"
gsettings set org.gnome.Vino vnc-password $(echo -n 'thepassword'|base64)
sudo reboot

# connect from x86 host via remmina (Save for easy future connect):
# Protocol: VNC
# Server 192.168.0.9x:5900
# User name: lighthouse
# password: <thepassword above>
# Display quality: best (slowest), 24bit per pixel
#  (8bpp is actually slower, as jetson has to encode it; 
#  Jetson's software-VNC is the bottleneck here, not network bandwidth)








###############################################################################
###############################################################################
# setup "Photo apparatus" hack requested by Tom: 
# setup xiCamTool via SSH to start from everywhere 
# (has to be launched from terminal though)


echo "#own stuff (Markus Schlueter): ------------------------------" >>  ~/.bashrc
echo "# make exec ximea CamTool from everywhere and also  find its own libraries:" >>  ~/.bashrc
echo "export PATH=\"/home/nvidia/devel/streaming/XimeaLinuxSDK/package/CamTool.arm64:\$PATH\"" >>  ~/.bashrc
echo "export LD_LIBRARY_PATH=\"/home/nvidia/devel/streaming/XimeaLinuxSDK/package/CamTool.arm64:\$LD_LIBRARY_PATH\"" >>  ~/.bashrc
source ~/.bashrc


###############################################################################
# Run "Photo apparatus" hack requested by Tom: 

#-----------------------------------------------------------------------------
# Manually take images:

#connect to streaming PC via teamviewer from any machine in the world

# Start Remmina on streaming PC
# connect to "Jetson 0" (the only one currently having a camera) 
# via double click

# On "Jetson 0", connects via Remmina:
#   - In Remmina, settings on the right, make sure 
#     "Grab all Keyboard events (Ctrl+R)" is enabled
#     (Keyboard symbol)

# open terminal on jetson (via remmina): leftCtrl+Alt+T; Type
xiCamTool
# Ximeas own cam tool should start

# In ximea cam tool:
# press "play" button to see cam image stream (top left)

# Adjust image quality:
# on the right, enable "auto white balance"
# on the right, set "downsampling" from 2 to 1 for full resolution
# use mouse wheel on image display to zoom out


 
# as soon as somethin interesting pops up, press "leftCtrl + enter" 
#  to instantly save an image as png
# ATTENTION: Be sure that a few seconds after pressing "ctrl + enter",
#  at the top right, a message "image xxxx" has been saved.
# ATTENTION: right ctrl does NOT work over remmina for unknown reasons!

# If you want to save it as TIFF, press "rightCtrl + S" and specify a name.
# WARNING: The image will only be saved which is current AFTER the file name
# has been specified!!11 I.e. there is a multi-second latency between 
# the decision to save an image as TIFF and the captureing of the image!
# Use png saving (rightCtrl+Enter) for quick photos!!11


#-----------------------------------------------------------------------------
# Download the taken images from Jetson to Streaming Desktop PC via scp:
# In Streaming Desktop:
# Example:
cd ~/Pictures/
mkdir jetsonPhotoDump
cd jetsonPhotoDump/
scp nvidia@192.168.0.90:/home/nvidia/Documents/instantsave/* .

#-----------------------------------------------------------------------------
# View downloaded images on Streaming PC:
eog .













###############################################################################
###############################################################################
# Setup development git repostitories on host machine and jetsons for pull & push:
# (Professionally, you would do this via CI/CD tools via DevOps teams.
# No time for that: Hence, living directly out of the development git repo 
# everywhere.

# generate SSH keys on the machines that should be able
# to git push and pull without Markus' personal password:


###############################################################################
# for streamer (x86)

ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_github_deploy_LirenaStream -C "lighthouse-streamer(x86_64): Deploy key for LirenaStream"
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_github_deploy_MultiOSCluster -C "lighthouse-streamer(x86_64): Deploy key for MultiOSCluster"

#'Register the last twoe public keys as  deploy keys on github:
# Go to github.com, login.
# E.g. the github repo for Streaming with ximea cams and Jetson TX2 (LirenaStream):
# Visit  https://github.com/tychonaut/LirenaStream/settings/keys
# work through:
# https://docs.github.com/en/developers/overview/managing-deploy-keys#deploy-keys


# config the differnt local SSH keys so that the correct one is presented to git:
# Follow:
# https://www.fabian-keller.de/blog/configuring-a-different-ssh-key-per-git-repository/

nano ~/.ssh/config_LirenaStream

#fill with:

# https://www.fabian-keller.de/blog/configuring-a-different-ssh-key-per-git-repository/
Host github.com
    HostName github.com
    Port 22
    User git
    IdentityFile ~/.ssh/id_ed25519_github_deploy_LirenaStream



nano ~/.ssh/config_MultiOSCluster

#fill with:

# https://www.fabian-keller.de/blog/configuring-a-different-ssh-key-per-git-repository/
Host github.com
    HostName github.com
    Port 22
    User git
    IdentityFile ~/.ssh/id_ed25519_github_deploy_MultiOSCluster



cd ~/devel/streaming
git clone git@github.com:tychonaut/multiOSCluster.git
cd multiOSCluster
git config core.sshCommand "ssh -F ~/.ssh/config_MultiOSCluster"

cd ~/devel/streaming
git clone git@github.com:tychonaut/LirenaStream.git
cd LirenaStream
git config core.sshCommand "ssh -F ~/.ssh/config_LirenaStream"



###############################################################################
# for Jetsons:


# generate ssh key on a Jetson:
# (Special file name here, because the default filename belongs to Markus' personal 
#  "Master key for all his github repos")
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_github_deploy_LirenaStream -C "Jetson TX2: Deploy key for LirenaStream"
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_github_deploy_MultiOSCluster -C "Jetson TX2: Deploy key for MultiOSCluster"
# copy and access like above for your own machine(s) ...

# 
git clone git@github.com:tychonaut/LirenaStream.git
# etc. pp, just like on x86


###############################################################################
###############################################################################
# Hardware accellerated nvenc/nvdec on x86:

# http://lifestyletransfer.com/how-to-install-nvidia-gstreamer-plugins-nvenc-nvdec-on-ubuntu/






