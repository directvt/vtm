#!/bin/bash
mkdir /var/vtm
cp vtm /var/vtm/
cp vtmd /var/vtm/
ln -s /var/vtm/vtm /usr/bin/vtm
ln -s /var/vtm/vtmd /usr/bin/vtmd
cat ./readme.txt
