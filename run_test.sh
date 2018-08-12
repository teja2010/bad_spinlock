
NN=2
while [ $NN -lt 103 ]
do
	aa=0
	while [ $aa -lt 30 ]
	do
		./a.out $NN
		aa=`expr $aa + 1`
	done
	NN=`expr $NN + 10`
done
