###Downloading
[Build DCE advanced mode  (with Linux kernel)](https://www.nsnam.org/docs/dce/manual/html/getting-started.html#building-dce-advanced-mode-with-linux-kernel)
If you get a bulid error about compiler problems, add ```compiler-gcc5.h``` to ```source/net-next-sim-2.6.36/include/linux/compiler-gcc5.h```.

Assuming the NS3 DCE installation directory

```NS3_HOME = $HOME/ns3-dce-linux/```


```
cd $NS3_HOME/source/ns-3-dce
git init
git remote add origin https://github.com/jsjolund/D0020E-Simulations_Application_Transport.git
git fetch
git checkout -t origin/master
```

###Running
```
./waf configure --with-ns3=$NS3_HOME/build --prefix=$NS3_HOME/build \\
                --enable-kernel-stack=$NS3_HOME/source/net-next-sim-2.6.36/arch
./waf build
./waf --run "my-sctp-test"
```
