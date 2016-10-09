#!/bin/bash

# Parse command line
zim=$1
serve=$2
if (( $# != 2 ))
then
    echo "Usage: ./benchmark ZIM_PATH KIWIX-SERVE_PATH"
    exit 1
fi

# Constants
delay=0.01
top_log=/tmp/top.log
top_calc=/tmp/top.calc

# Available memory
memory_kb=`cat /proc/meminfo | grep "MemTotal" | tr -d " " | cut -f2 -d: | sed -e 's/[^0123456789]//g'`
memory_mb=`echo "scale=2;$memory_kb/1024" | bc -l`

# Start Kiwix-serve
echo "Starting kiwix-serve to be tested..."
$serve --port=8080 "$zim" &
serve_pid=$!

# Start top to monitor resource usage
top -d $delay -b -p $serve_pid | grep $serve_pid > "$top_log" &
top_pid=$!

# Print environnement informations
echo "Process to monitor: $serve_pid"
echo "Monitoring process: $top_pid"
echo "Monitoring delay:   $delay s"
echo "ZIM file path:      $zim"
echo "Kiwix-serve path:   $serve"
echo "Total memory:       $memory_mb MB"
echo

# Compute artice list
echo "Computing article list snippet..."
articles=`zimdump -l $zim | grep ".html" | shuf | head -n 1000`

# Run wget against kiwix-serve
start_date=`date +%s`
for LINE in $articles
do
    echo "Scrapping $LINE..."
    wget --quiet p -P /dev/shm/tmp "http://localhost:8080/wikipedia_en_medicine_2016-09/A/$LINE"
    rm -rf /dev/shm/tmp
done
end_date=`date +%s`

# Kill top instance
kill -s STOP $top_pid 2>&1 > /dev/null
sed -i '$ d' "$top_log"
times=`cat "$top_log" | wc -l`

# Compute KPI
duration=`echo "$end_date-$start_date" | bc -l`
cpu_percent=`cat "$top_log" | sed -r -e "s;\s\s*; ;g" -e "s;^ *;;" | cut -d' ' -f9 | tr '\n' '+0' | sed -r -e "s;(.*)[+]$;\1;" -e "s/.*/scale=2\n(&)\/$times\nquit\n/" > "$top_calc" ; bc -q "$top_calc"`
memory_percent=`cat "$top_log" | sed -r -e "s;\s\s*; ;g" -e "s;^ *;;" | cut -d' ' -f10 | tr '\n' '+' | sed -r -e "s;(.*)[+]$;\1;" -e "s/.*/scale=2\n(&)\/$times\nquit\n/" > "$top_calc" ; bc -q "$top_calc"`
memory_absolut=`echo "scale=2;$memory_mb/100*$memory_percent" | bc -l`
 
echo 
echo "Measure count:      $times"
echo "Duration:           $duration s"
echo "CPU (average):      $cpu_percent %"
echo "Memory (average):   $memory_absolut MB"

# Kill kiwix-serve & top instances
kill -s STOP $serve_pid 2>&1 > /dev/null
#rm "$top_log" "$top_calc"
