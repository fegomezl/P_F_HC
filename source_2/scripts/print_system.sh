DT=`sed -n 10p data/init_data.txt | tr -d -c 0-9.`
iterations=`sed -n 11p data/init_data.txt | tr -d -c 0-9.`
resolution=`sed -n 12p data/init_data.txt | tr -d -c 0-9.`
END=`echo $iterations/$resolution-1 | bc`

#Toma los archivos de cada tiempo y los ordena en un solo archivo
echo -n > data/results/system_state.txt
for NTH in $(seq 0 $END); do
    time=`echo "$DT*$NTH*$resolution" | bc`
    echo -n -e "$time\t"
    sed '1q;d' data/results/data_system-$NTH.csv
done >> data/results/system_state.txt
