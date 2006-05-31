#!/bin/bash

echo Running from $PWD\n
#just in case the network drive wasn't up when we booted...
sudo mount -a
sh ./linux_build_clanlib.sh
sh ./linux_svn_update.sh
sh ./linux_build_release.sh
sh ./linux_media_update.sh
sh ./linux_package_release.sh

Pause
