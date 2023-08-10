## TODO: Update readme

`git submodule update --remote .\third_party\bvc`

## Buildings steps
### 1. Build BVC
```cmd
cd ./third_party/bvc
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A Win32 ..
cmake --build . --target bvc_enc
cmake --build . --target bvc_dec
```

### 2. Build XVC
```cmd
cd ./third_party/xvc
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A Win32 ..
cmake --build . --target xvc_enc
cmake --build . --target xvc_dec
```

### 3. Build Libde265
##### Modify libde265/Makefile.cv7
Add /DDE265_LOG_INFO /DDE265_LOG_DEBUG /DDE265_LOG_ERROR to DEFINES

Add /Zi to CFLAGS

Add DLLExport to de265_image constructor, destructor and alloc_image

```
cd ./third_party/libde265
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A Win32 ..
cmake --build . --target libde265
```


From the repo's parent directory
```cmd
cp .\third_party\libde265\build\libde265\Debug\libde265.dll .\bin
```


### 3. Build Framework