cat a && cat b && echo line 1
cat b && (echo hihi|| sleep 3 
sort a || echo hihi || sleep 3
echo hi > a || echo hihi || sleep 3) && echo line 2
cat a && cat b && echo line 3

