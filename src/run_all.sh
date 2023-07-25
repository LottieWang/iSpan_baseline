#!/bin/bash
declare -a dir_graph=(
  #"soc-LiveJournal1.bin"
  #"sd_arc.bin"
  #"twitter.bin"
  #"clueweb.bin"
  #"hyperlink2014.bin"
  #"hyperlink2012.bin"
  "CHEM_2.bin"
  "CHEM_5.bin"
  "CHEM_10.bin"
  "Cosmo50_2.bin"
  "Cosmo50_5.bin"
  "Cosmo50_10.bin"
  "GeoLifeNoScale_2.bin"
  "GeoLifeNoScale_5.bin"
  "GeoLifeNoScale_10.bin"
  "HT_2.bin"
  "HT_5.bin"
  "HT_10.bin"
	#"Household.lines_2.bin"
	#"Household.lines_5.bin"
	#"Household.lines_10.bin"
	#"grid_10000_10000.bin"
	#"grid_10000_10000_03.bin"
	#"grid_1000_10000.bin"
	#"grid_1000_10000_03.bin"
	#"grid_100_100000.bin"
	#"grid_100_100000_03.bin"
	#"grid_4000_4000.bin"
	#"grid_4000_4000_03.bin"
	#"GeoLifeNoScale_15.bin"
	#"GeoLifeNoScale_20.bin"
)

declare graph_path="/data/graphs/bin/"

declare numactl="numactl -i all"

declare thread_count=192

declare alpha=30

declare beta=200

declare gamma=10

declare theta=0.10

declare run_times=10

#truncate -s 0 ispan.dat

make

for graph in "${dir_graph[@]}"; do
  echo "cmd = \"${numactl} ./ispan ${graph_path}${graph} ${thread_count} ${alpha} ${beta} ${gamma} ${theta} ${run_times}\""
  echo "${graph}" >> ispan.dat
  ${numactl} ./ispan ${graph_path}${graph} ${thread_count} ${alpha} ${beta} ${gamma} ${theta} ${run_times} >> ispan.dat
done

