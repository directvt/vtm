#!/bin/bash

# Desktopio client
vtm='vtm'

# Desktopio server
vtmd='vtmd'

# Installation path
var_vtm='/var/vtm/'

# (Linux) System-wide accessible binaries path
usr_bin='/usr/bin/'

# (macOS) System-wide accessible binaries path
#usr_bin='/usr/bin/'

confirm () {
	if [ -a "$1" ]; then
		echo -n "Object '$1' already exists. It will be modified. Continue? [y/N]:"
		read -r
		if [ "$REPLY" != "y" ]; then
			echo "...aborted"
			exit 1
		fi
		return 1
	else
		return 0
	fi
}
symlink () {
	confirm "$2"
	echo -n "create symlink "; ln -v -s -f "$1" "$2"
}
copy () {
	echo -n "copy "; cp -v "$1" "$2"
}

confirm "$var_vtm" && mkdir -v "$var_vtm"
copy "$vtm"  "$var_vtm"
copy "$vtmd" "$var_vtm"

symlink "${var_vtm}${vtm}"  "${usr_bin}${vtm}"
symlink "${var_vtm}${vtmd}" "${usr_bin}${vtmd}"
