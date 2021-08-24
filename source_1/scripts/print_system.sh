echo -n > data/system_parameters/system_state.txt
echo -n > data/visual/box.txt

Lx=`sed -n 1p data/init_data.txt | tr -d -c 0-9.`
Ly=`sed -n 1p data/init_data.txt | tr -d -c 0-9.`
Lz=`sed -n 2p data/init_data.txt | tr -d -c 0-9.`

DT=`sed -n 10p data/init_data.txt | tr -d -c 0-9.`
iterations=`sed -n 11p data/init_data.txt | tr -d -c 0-9.`
resolution=`sed -n 12p data/init_data.txt | tr -d -c 0-9.`
END=`echo $iterations/$resolution-1 | bc`

#Toma los archivos de cada tiempo y los ordena en un solo archivo
for NTH in $(seq 0 $END); do
    time=`echo "$DT*$NTH*$resolution" | bc`
    echo -n -e "$time\t"
    sed '1q;d' data/system_parameters/data_system-$NTH.csv
done >> data/system_parameters/system_state.txt

#Escribe las coordenadas para la caja de las pelotas
echo -e "0 \t0 \t 0" >> data/visual/box.txt
echo -e "$Lx \t0 \t 0" >> data/visual/box.txt
echo -e "$Lx \t$Ly \t 0" >> data/visual/box.txt
echo -e "0 \t$Ly \t 0" >> data/visual/box.txt
echo -e "0 \t0 \t 0" >> data/visual/box.txt
echo -e "0 \t0 \t $Lz" >> data/visual/box.txt
echo -e "$Lx \t0 \t $Lz" >> data/visual/box.txt
echo -e "$Lx \t$Ly \t $Lz" >> data/visual/box.txt
echo -e "0 \t$Ly \t $Lz" >> data/visual/box.txt
echo -e "0 \t0 \t $Lz" >> data/visual/box.txt
echo -e "0 \t0 \t 0" >> data/visual/box.txt
echo -e "0 \t$Ly \t 0" >> data/visual/box.txt
echo -e "0 \t$Ly \t $Lz" >> data/visual/box.txt
echo -e "0 \t0 \t $Lz" >> data/visual/box.txt
echo -e "$Lx \t0 \t $Lz" >> data/visual/box.txt
echo -e "$Lx \t0 \t 0" >> data/visual/box.txt
echo -e "$Lx \t$Ly \t 0" >> data/visual/box.txt
echo -e "$Lx \t$Ly \t $Lz" >> data/visual/box.txt
