cat < a && (echo hi ||  echo hello) && echo line 1
(echo hey && cat  < b) < c && echo line 2

