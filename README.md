# Minecraft Clone - Optimised Voxel Engine

Full Minecraft replica with infinite terrain, greedy meshing, palette storage, multithreading, and networking support.

## Build Locally
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./blockclone
```

## Cloud Compilation (GitHub Actions)
Push to GitHub and download the compiled artifact from Actions.
