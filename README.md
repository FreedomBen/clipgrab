clipgrab
========

Slightly customized version of ClipGrab (http://clipgrab.org/)

ClipGrab is a GUI frontend for [youtube-dl](https://youtube-dl.org).

See [README](https://github.com/FreedomBen/clipgrab/blob/master/README) for build instructions if you are building yourself.

Unless you want to hack on the code, I would recommend [downloading a pre-built AppImage file](https://clipgrab.org/).

You'll need to make the AppImage executable, then you can run it:

```sh
# Set version of ClipGrab.  As of Oct 31, 2021 latest version is 3.9.7
CLIPGRAB_VER=3.9.7

# Change directory to ~/Downloads and download latest AppImage
cd ~/Downloads
wget "https://download.clipgrab.org/ClipGrab-${CLIPGRAB_VER}-x86_64.AppImage"

# Make downloaded AppImage executable
chmod +x "ClipGrab-${CLIPGRAB_VER}-x86_64.AppImage"

# Run it!
./ClipGrab-${CLIPGRAB_VER}-x86_64.AppImage
```

If you find this software useful, please consider making [a donation](https://clipgrab.org/donate).
