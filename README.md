## TODO: Update readme

`git submodule update --remote .\third_party\ovc`

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


### 3. Build Framework
```cmd
mkdir build 
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --target ovb_send
cmake --build . --target ovb_recv