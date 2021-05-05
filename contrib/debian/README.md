
Debian
====================
This directory contains files used to package hodlcashd/hodlcash-qt
for Debian-based Linux systems. If you compile hodlcashd/hodlcash-qt yourself, there are some useful files here.

## hodlcash: URI support ##


hodlcash-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install hodlcash-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your hodlcash-qt binary to `/usr/bin`
and the `../../share/pixmaps/hodlcash128.png` to `/usr/share/pixmaps`

hodlcash-qt.protocol (KDE)

