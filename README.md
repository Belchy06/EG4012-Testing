## TODO: Update readme

## Buildings steps
### 1. Build XVC
```cmd
cd ./third_party/xvc
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A Win32 ..
```

### 2. Build Libde265
```cmd
cd ./third_party/libde265
./build.bat
```
From the repo's parent directory
```cmd
cp ./third_party\libde265/bin_x86/libde265.dll ./bin
```


### 3. Build Framework