cat a || echo line 1 && sleep 3
cat b || echo line 2 && sleep 3
echo hello > a && echo hi > b && echo line 3 && sleep 3
sort < a && cat b && echo line 4 && sleep 3
echo hey > a && echo aha > b && echo line 5 && sleep 3
sort < b && cat b && echo line 6 && sleep 3

