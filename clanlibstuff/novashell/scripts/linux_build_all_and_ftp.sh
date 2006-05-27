#!/bin/bash

echo Running from $PWD - building $filename
sh ./linux_build_svn.sh
sh ./linux_svn_update.sh
sh ./linux_build_release.sh
sh ./linux_media_update.sh
sh ./linux_package_release.sh

Pause
