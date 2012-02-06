cat a && echo line 1 && sleep 3 
cat a && ( echo inside && (echo hi > a)) > b && echo line 2 && sleep 3
cat a && echo line 3 && sleep 3
cat b && echo line 4 && sleep 3 

