cat a &&(sort < a || echo hihi|| sleep 3 
sort a || echo hihi || sleep 3
echo hi > a || echo hihi || sleep 3) && echo line 1 && sleep 3
cat a && echo line 2 && sleep 3

