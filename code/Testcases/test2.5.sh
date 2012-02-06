cat a && echo line 1
cat a &&(sort < a || echo hihi|| sleep 3 
sort a || echo hihi || sleep 3
echo hi > a || echo hihi || sleep 3) && echo line 2 && sleep 3
cat a && echo line 3 && sleep 3
cat a && echo line 4 && sleep 3

