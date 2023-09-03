## TODO: Update readme

`git submodule update --init --recursive`

## Buildings steps

### 1. Build OVC
```cmd
cd ./third_party/ovc
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target ovc_enc
cmake --build . --target ovc_dec
```

### 2. Build XVC
```cmd
cd ./third_party/xvc
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target xvc_enc_lib
cmake --build . --target xvc_dec_lib
```

### 3. Build VVC
#### 3.1 Build VVENC
```cmd
cd ./third_party/vvc/vvenc
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target vvenc_lib
```

#### 3.2 Build VVENC
```cmd
cd ./third_party/vvc/vvdec
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target vvdec_lib
```

### 4. Build AVC
1. Open ./third_party/avc/codec/build/win32/dec/WelsDecoder.sln
2. Set `WelsDecCore` and `WelsDecPlus` projects to use `MT` Runtime Library
3. Build all projects
4. Open ./third_party/avc/codec/build/win32/dec/WelsEncoder.sln
2. Set `WelsEncCore`, `WelsEncPlus` and `WelsEncVP` projects to use `MT` Runtime Library
3. Build all projects

### 4. Build HEVC
```cmd
cd ./third_party/hevc
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 .. -DENABLE_ENCODER=ON -DBUILD_SHARED_LIBS=OFF -DENABLE_SDL=OFF
cmake --build . --target de265
```

### 5. Build VMAF
Using a mingw terminal
```sh
cd ./third_party/vmaf
mkdir vmaf-install
meson setup --wipe libvmaf libvmaf/build --buildtype release --default-library static --prefix {path/to/OpenVideoBenchmark}/src/third_party/vmaf/vmaf-install
meson install -C libvmaf/build
```


### 3. Build Framework
```cmd
mkdir build 
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target ovb_send
cmake --build . --target ovb_recv