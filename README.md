###About
This project aims to asses the performances of the [SCTP](https://en.wikipedia.org/wiki/Stream_Control_Transmission_Protocol), [DCCP](https://en.wikipedia.org/wiki/Datagram_Congestion_Control_Protocol) and [TCP](https://en.wikipedia.org/wiki/Transmission_Control_Protocol) network transport-layer protocol during different scenarios, using the network simulator [NS-3](https://www.nsnam.org/). The NS-3 framework  [DCE](https://www.nsnam.org/overview/projects/direct-code-execution/) is used to simulate client/server applications which generate network traffic. For the DCCP and TCP protocols, the standard Linux kernel implementations are used, while the SCTP protocol uses the external library called [lksctp](http://lksctp.sourceforge.net/).

###Dependencies
Using Ubuntu 14.04, install the dependencies:

```
apt-get install gcc g++ python python-dev qt4-dev-tools libqt4-dev mercurial bzr cmake \
                libc6-dev libc6-dev-i386 g++-multilib gsl-bin libgsl0-dev libgsl0ldbl \
                flex bison libfl-dev tcpdump sqlite sqlite3 libsqlite3-dev libxml2 \
                libxml2-dev libgtk2.0-0 libgtk2.0-dev vtun lxc doxygen graphviz imagemagick \
                git python-pygraphviz python-pygoocanvas libpcap-dev libdb-dev libssl-dev \
                lksctp-tools libsctp-dev tshark gnuplot
```
Optional: ```apt-get install python-pygccxml```, but having it installed may cause build errors...

###Building
[Install the Bake build tool](https://www.nsnam.org/docs/dce/manual/html/getting-started.html#building-dce-basic-mode), then [build DCE advanced mode  (with Linux kernel)](https://www.nsnam.org/docs/dce/manual/html/getting-started.html#building-dce-advanced-mode-with-linux-kernel).
If you get a bulid error about compiler problems, add [compiler-gcc5.h](compiler-gcc5.h) to ```source/net-next-sim-2.6.36/include/linux/compiler-gcc5.h```

###Required system settings
Most Linux systems place restrictions on how many user processes can be run at the same time. This project needs to run multiple NS-3 simulation instances in order to generate useful network statistics, which creates a lot of processes. Therefore it is necessary to append the following lines to the end of ```/etc/security/limits.conf```:
```
*         hard    nproc     65536
*         soft    nproc      65536
```
For more information see the [DCE manual](https://www.nsnam.org/docs/dce/release/1.4/manual/singlehtml/index.html#processes-limit-resource-temporarily-unavailable).

###Downloading the project
Assuming the NS-3 DCE installation directory
```
export NS3_HOME="$HOME/dce"
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
./waf --run "my-simulator"
```

###Plotting
The plotting program is separate from the simulation. To plot the results from a simulation, run:
```
cd $NS3_HOME/source/ns-3-dce/build/myscripts/my-sctp-test/bin/
./NSplot output.png ../../../../my-simulator-output/*.dat -2d
```

###Useful commands
Output from the ```DceApplicationHelper``` processes, i.e. the SCTP server and client programs, are stored in the folders ```$NS3_HOME/source/ns-3-dce/files-*``` where the star is the NS-3 ```NodeContainer``` id number.

To print what was written to standard output with e.g. ```printf("debug output")``` run
```
cd $NS3_HOME/source/ns-3-dce
find files-* -name 'stdout' -exec cat {} \;
```
Searching for all lines from standard output which contain the word "debug" can be done with
```
find files-* -name 'stdout' -exec grep 'debug' {} \;
```
