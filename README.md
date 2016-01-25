###Downloading
Follow https://www.nsnam.org/docs/dce/manual/html/getting-started.html#building-dce-advanced-mode-with-linux-kernel
On build error add ```compiler-gcc5.h``` to ```source/net-next-sim-2.6.36/include/linux/compiler-gcc5.h```
```
cd /home/d0020e/ns3-dce-linux/source/ns-3-dce
git init
git remote add origin https://github.com/jsjolund/D0020E-Simulations_Application_Transport.git
git fetch
git checkout -t origin/master
```
###Running
```
./waf configure --with-ns3=/home/d0020e/ns3-dce-linux/build/
./waf --run "myscripts-sctp-sim"
```
