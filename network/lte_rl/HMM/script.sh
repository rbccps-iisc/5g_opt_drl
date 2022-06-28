# change flows to vary number of UEs

# For Grid Scenario
netgenerate --grid --grid.number=3 -L=1 --grid.length=50 --output-file=grid.net.xml

#netconvert --osm-files map.osm -o map.net.xml
randomTrips.py -n grid.net.xml -o flows.xml --begin 0 --end 1 --period 1 --flows 1
jtrrouter --flow-files=flows.xml --net-file=grid.net.xml --output-file=grid.rou.xml --begin 0 --end 210 --accept-all-destinations
generateContinuousRerouters.py -n grid.net.xml --end 210 -o rerouter_grid.add.xml
