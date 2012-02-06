cat a && echo line 1 && sleep 3
(echo 1 > a && echo inside) && echo line 2 && sleep 3
sort a && echo line 3 && sleep 3
(echo inside ) > a && echo line 4 && sleep 3
cat a  && sleep 3 

