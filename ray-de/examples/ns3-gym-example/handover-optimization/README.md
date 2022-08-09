For inbuilt handover algorithms i.e.:

1. A3-rsrp-handover
2. A2A4-rsrq-handover

To run this example, copy this directory inside the ns3-gym scratch directory:
```
cp -r ../handover-optimization $NS3_INSTALLED_DIR/scratch/handover-optimization
```

Configure and build the ns3 module from the ns3 source directory:
```
# Configure with your own arguments, if required:
python3 waf configure --build-profile=optimized --enable-sudo --disable-werror --enable-mpi --with-nsc --with-nsclick --with-brite --force-planetlab

# build the ns3 module:
python3 waf build
```


For A3-rsrp algorithm, run and save log as:
```
python3 waf --run "handover-optimization --handover_algo=A3-rsrp" >> scratch/handover-optimization/plots/A3-rsrp-handover.txt
```

Alternatively, for A2A4-rsrq algorithm, run and save log as:
```
python3 waf --run "handover-optimization --handover_algo=A2A4-rsrq" >> scratch/handover-optimization/plots/A2A4-rsrq-handover.txt
```

To generate plots of rsrp, rsrq and serving cell states, run the following script from inside the `scratch/handover-optimization/plots` directory for the required handover log file:
```
python3 inbuilt-handover-plots.py A3-rsrp-handover.txt
```