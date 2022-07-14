## Bing daily wallpaper daemon

### Build:
```bash
mkdir build && cd build
cmake ..
```

### Run:
```bash
./build/bing_wallpaper_daemon
```

### Example config:
##### $HOME/.config/bdw.cfg
```
interval 10000
path /home/me/Wallpapers/wallpaper.jpg
command swaybg $WALLPAPER_PATH
region en-US
resolution 1920x1080
```
