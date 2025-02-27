#!/bin/bash

if [[ ! -f ./build/as/x86/asx86 ]]; then
    echo "Error: No assembler built!"
    exit 1
fi

cd test

echo "Running all tests..."
echo ""

total=0

for d in ./as-x86/*; do
    for file in $d/*; do
        echo "`basename $d` `basename $file`"
        
        OBJS=""
        if [[ `basename $d` == "extern" ]] ; then
            as lib_test.asm -o build/lib_test.o
            OBJS="build/lib_test.o"
        fi
        
        ../build/as/x86/asx86 $file a.out
        ld a.out $OBJS -o out
        
        as $file -o test.out
        ld test.out $OBJS -o test.bin
        
        ./out
        R1=$?
        
        ./test.bin
        R2=$?
        
        NM1=`nm a.out`
        NM2=`nm test.out`
        
        OUTPUT1=`./out`
        OUTPUT2=`./test.bin`
        
        rm a.out
        rm out
        rm test.out
        rm test.bin
        
        if [[ $R1 == $R2 ]] ; then
            if [[ $NM1 == $NM2 ]] ; then
                if [[ `basename $d` == "output" || `basename $d` == "extern" ]] ; then
                    if [[ $OUTPUT1 == $OUTPUT2 ]] ; then
                        echo "[OUTPUT] Pass"
                        echo ""
                        
                        total=$((total+1))
                    else
                        echo "Output test failed."
                        echo "Expected:"
                        echo "$OUTPUT2"
                        echo "Actual:"
                        echo "$OUTPUT1"
                        echo ""
                        exit 1
                    fi
                else
                    echo "Pass"
                    echo ""
                    
                    total=$((total+1))
                fi
            else
                echo "Test failed- symbol check."
                echo "Expected":
                echo "$NM2"
                echo "Actual:"
                echo "$NM1"
                echo ""
                exit 1
            fi
        else
            echo "Test failed."
            echo "Expected: $R2"
            echo "Actual: $R1"
            exit 1
        fi
    done
done

echo ""
echo "$total tests passed!"

echo ""
echo "Done"

cd ..


