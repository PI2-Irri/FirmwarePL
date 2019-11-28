# FirmwarePl

## Install RF24
```console
  cd libs/RF24
  sudo ./configure
  sudo make install -B
```
## Install PLOG
```console
  git clone https://github.com/SergiusTheBest/plog
  mkdir build 
  cd build
  cmake ..
  make -j
  sudo make install
```

## Build mainIrri
```console
  cd build
  make
  sudo ../bin/mainIrri.out
```
