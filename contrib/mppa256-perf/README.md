Performance Monitoring Library for MPPA-256
===========================================

What Is This Project About?
---------------------------

This project delivers a lightweight performance monitoring library for
the Kalray MPPA-256 manycore processor. This library is built on top
of hardware counters that are natively available in the processor and
exposes an API for monitoring the following performance events with
64-bit precision:

- Clock Cycles
- Instruction Cache Hits
- Instruction Cache Misses
- Instruction Cache Stalls
- Data Cache Hits
- Data Cache Misses
- Data Cache Stalls
- Bundles Executed
- Branches Taken
- Branch Stalls
- Register Dependence Stalls
- Instruction TLB Stalls
- Data TLB Stalls
- Stream Buffer Stalls

Building & Installing
---------------------

**1. Clone This Repository**

```
export WORKDIR=$HOME                                        # Change this at your will
cd $WORKDIR                                                 # Go to working directory
git clone https://github.com/cart-pucminas/mppa256-perf.git # Clone the source tree
```

**2. Build & Install the Library**

```
export PREFIX=$HOME  # Set installation directory
make all             # Build
make install         # Install

```

**3. Uninstall the Library (optional)**

```
export PREFIX=$HOME  # Set installation directory
make install         # Uninstall 
```

License & Maintainers
---------------------

This project is distributed under the [MIT
License](https://raw.githubusercontent.com/cart-pucminas/mppa256-perf/master/LICENSE).
It was developed and it is currently maintained by [Pedro Henrique
Penna](https://sites.google.com/view/ppenna). For any questions you
may directly address him through
[pedrohenriquepenna@gmail.com](mailto:pedrohenriquepenna@gmail.com).
