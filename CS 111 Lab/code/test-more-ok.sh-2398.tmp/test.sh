a;b
a||b||c||d||e;

a && b && c|
 d &&
 e && f

echo lol|cat>foo.txt&&echo ok&&
  ls

cat<bar.txt||ls&&pwd||echo what

a && b &&
# comments shouldn't break this up
c && d


(ls;cat > foo.txt)>out.txt && echo ok || echo what

(
		ls && pwd && a>b|c<d;
		cat > foo.txt
) < in.txt||
echo abc||echo def

a && b && c|(d && e; f) && g

(a;)>b

(a && b | (c || d; e) > f; g || h) < i
