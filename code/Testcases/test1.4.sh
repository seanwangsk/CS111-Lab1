cat < a && cat < b || echo line 1 && sleep 3
cat < b && cat < c || echo line 2 && sleep 3
cat < a && cat < c || echo line 3 && sleep 3

