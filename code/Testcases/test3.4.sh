cat a && echo line 1 && sleep 3
cat b && echo line 2 && sleep 3
cat c && echo line 3 && sleep 3
echo 1 > a && echo 2 > b && echo line 4 && sleep 3
echo 3 > b && echo 4 > c && echo line 5 && sleep 3
echo 5 > a && echo 6 > c && echo line 6 && sleep 3
cat a && echo line 7 && sleep 3
cat b && echo line 8 && sleep 3
cat c && echo line 9 && sleep 3

