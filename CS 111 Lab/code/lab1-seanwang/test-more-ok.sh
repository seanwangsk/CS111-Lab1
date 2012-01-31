#! /bin/sh

# UCLA CS 111 Lab 1 - Test that valid syntax is processed correctly.

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
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
EOF

cat >test.exp <<'EOF'
# 1
  a
# 2
  b
# 3
    a \
  ||
    b \
  ||
    c \
  ||
    d \
  ||
    e
# 4
    a \
  &&
    b \
  &&
      c \
    |
      d \
  &&
    e \
  &&
    f
# 5
      echo lol \
    |
      cat>foo.txt \
  &&
    echo ok \
  &&
    ls
# 6
        cat<bar.txt \
      ||
        ls \
    &&
      pwd \
  ||
    echo what
# 7
    a \
  &&
    b \
  &&
    c \
  &&
    d
# 8
      (
         ls \
       ;
         cat>foo.txt
      )>out.txt \
    &&
      echo ok \
  ||
    echo what
# 9
    (
         ls \
       &&
         pwd \
       &&
           a>b \
         |
           c<d \
     ;
       cat>foo.txt
    )<in.txt \
  ||
    echo abc \
  ||
    echo def
# 10
    a \
  &&
    b \
  &&
      c \
    |
      (
           d \
         &&
           e \
       ;
         f
      ) \
  &&
    g
# 11
  (
   a
  )>b
# 12
  (
       a \
     &&
         b \
       |
         (
              c \
            ||
              d \
          ;
            e
         )>f \
   ;
       g \
     ||
       h
  )<i
EOF

../timetrash -p test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"
