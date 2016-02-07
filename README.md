###About
This project aims to asses the performance of the [SCTP](https://en.wikipedia.org/wiki/Stream_Control_Transmission_Protocol) network transport-layer protocol during different scenarios, using the network simulator [NS-3](https://www.nsnam.org/). More specifically, the Linux kernel implementation of SCTP called [lksctp](http://lksctp.sourceforge.net/) is accessed using the NS-3 framework [DCE](https://www.nsnam.org/overview/projects/direct-code-execution/).

###Dependencies
Using Ubuntu 14.04, install the dependencies:

```
apt-get install gcc g++ python python-dev qt4-dev-tools libqt4-dev mercurial bzr cmake \
                libc6-dev libc6-dev-i386 g++-multilib gsl-bin libgsl0-dev libgsl0ldbl \
                flex bison libfl-dev tcpdump sqlite sqlite3 libsqlite3-dev libxml2 \
                libxml2-dev libgtk2.0-0 libgtk2.0-dev vtun lxc doxygen graphviz imagemagick \
                git python-pygraphviz python-pygoocanvas libpcap-dev libdb-dev libssl-dev \
                lksctp-tools libsctp-dev tshark
```
Optional: ```apt-get install python-pygccxml```, but having it installed may cause build errors...

###Building
[Build DCE advanced mode  (with Linux kernel)](https://www.nsnam.org/docs/dce/manual/html/getting-started.html#building-dce-advanced-mode-with-linux-kernel).
If you get a bulid error about compiler problems, add ```compiler-gcc5.h``` to ```source/net-next-sim-2.6.36/include/linux/compiler-gcc5.h```

###Downloading the project
Assuming the NS3 DCE installation directory
```
NS3_HOME="$HOME/ns3-dce-linux"
```
```
cd $NS3_HOME/source/ns-3-dce
git init
git remote add origin https://github.com/jsjolund/D0020E-Simulations_Application_Transport.git
git fetch
git checkout -t origin/master
```

###Running
```
./waf configure --with-ns3=$NS3_HOME/build --prefix=$NS3_HOME/build \
                --enable-kernel-stack=$NS3_HOME/source/net-next-sim-2.6.36/arch
./waf build
./waf --run "my-sctp-test"
```
