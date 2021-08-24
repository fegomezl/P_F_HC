NT=(2116 4096 6084 8100 10000) #49 64 81 100 121 144 169 196 225
NP=(1 2 3 4 5 6 7 8 10 12)

echotxt()
(
    echo -e "$@" >> $text_file_name
)

for nt in ${NT[@]}; do
    text_file_name="data/scaling/metrics-$nt.txt"
    sed -i "/nt/c $nt	    #nt" data/init_data.txt
    allAvg=() #Average running times with np processors
    allDev=() #Average running times deviations
    allSca=() #Scalings with np processors
    allScaDev=() #Scaling deviations
    allEffi=() #Parallel efficiencies
    for np in ${NP[@]}; do
	echo -e "Computing $nt balls with $np processors.\n"
	sed -i "/nproc/c $np	#nproc" data/init_data.txt
	sum=0 #Sum of all runtimes
	sumsq=0 #Sum of squares of all runtimes
	count=0 #Total repetitions
	for rep in 1 2 3 4 5 6 7 8 9 10; do
	    let "count++"
	    echo -n "Running $count/10."
	    mpirun -np $(sed -n 25p data/init_data.txt | tr -d -c 0-9) --oversubscribe ./main.x 
	    rm -f visual/*.csv
	done
	echo -e "\n"
	sum=$(awk '{sum+=$2}END{print sum}' data/scaling/runtimes-$nt.txt)
	avg=$(echo "$sum/$count" | bc -l)
	allAvg+=( $avg ) #Save average times
	echo "SAverage runtime = $avg."
	sumsq=$(awk '{sumsq+=$2*$2}END{print sumsq}' data/scaling/runtimes-$nt.txt)
	dev=$(echo "sqrt($sumsq/$count-($sum/$count)^2)" | bc -l)
	allDev+=( $dev ) #Save average time deviations
	echo "Deviation of average runtime = $dev."
	> data/scaling/runtimes-$nt.txt
    done
    echo "Compute scalings."
    for a in ${!allAvg[@]}; do
	allSca[$a]=$( echo "${allAvg[0]}/${allAvg[$a]}" | bc -l )
    done
    echo "Compute scaling deviations."
    for i in ${!allAvg[@]}; do
	allScaDev[$i]=$( echo "${allAvg[$i]}*sqrt((${allDev[0]}/${allAvg[0]})^2+(${allDev[$i]}/${allAvg[$i]})^2)" | bc -l )
    done
    echo "Compute parallel efficiencies."
    for j in ${!allSca[@]}; do
	allEffi[$j]=$( echo "${allSca[$j]}/${NP[$j]}" | bc -l )
    done
    for k in ${!NP[@]}; do
	echotxt "$nt ${NP[$k]} ${allSca[$k]} ${allEffi[$k]} ${allScaDev[$k]}"
    done
done

sed -i "/nt/c 100	#nt" data/init_data.txt
sed -i "/nproc/c 2	#nproc" data/init_data.txt
