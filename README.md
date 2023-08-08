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
##### Modify libde265/Makefile.cv7
Remove /DNDEBUG
Add /DDE265_LOG_INFO /DDE265_LOG_DEBUG /DDE265_LOG_ERROR

```cmd
cd ./third_party/libde265
./build.bat
```
From the repo's parent directory
```cmd
cp ./third_party\libde265/bin_x86/libde265.dll ./bin
```


### 3. Build Framework