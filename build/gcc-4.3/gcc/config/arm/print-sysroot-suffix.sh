#! /bin/sh
# Script to generate SYSROOT_SUFFIX equivalent to MULTILIB_OSDIRNAMES
# Arguments are MULTILIB_ALIASES, MULTILIB_OSDIRNAMES and MULTILIB_OPTIONS.

set -e

aliases="$1"
dirnames="$2"
options="$3"
matches="$4"

cat > print-sysroot-suffix3.sh <<\EOF
#! /bin/sh
# Print all the multilib matches for this option
result="$1"
EOF
for x in $matches; do
  l=`echo $x | sed -e 's/=.*$//' -e 's/?/=/g'`
  r=`echo $x | sed -e 's/^.*=//' -e 's/?/=/g'`
  echo "[ \"\$1\" = \"$l\" ] && result=\"\$result|$r\"" >> print-sysroot-suffix3.sh
done
echo 'echo $result' >> print-sysroot-suffix3.sh
chmod +x print-sysroot-suffix3.sh

cat > print-sysroot-suffix2.sh <<\EOF
#! /bin/sh
# Recursive script to enumerate all multilib combinations, match against
# multilib directories and optut a spec string of the result.
# Will fold identical trees.

padding="$1"
optstring="$2"
shift 2
n="\" \\
$padding\""
if [ $# = 0 ]; then
  case $optstring in
EOF
for x in $aliases; do
  l=`echo $x | sed -e 's/=.*$//' -e 's/?/=/g'`
  r=`echo $x | sed -e 's/^.*=//' -e 's/?/=/g'`
  echo "/$r/) optstring=\"/$l/\" ;;" >> print-sysroot-suffix2.sh
done

pat=
for x in $dirnames; do
  p=`echo $x | sed -e 's,=!,/$=/,'`
  pat="$pat -e 's=^//$p='"
done
echo "  esac" >> print-sysroot-suffix2.sh
echo '  optstring=`echo "/$optstring" | sed '"$pat\`" >> print-sysroot-suffix2.sh
cat >> print-sysroot-suffix2.sh <<\EOF
  case $optstring in
  //*)
    ;;
  *)
    echo "$optstring"
    ;;
  esac
else
  thisopt="$1"
  shift
  bit=
  lastcond=
  result=
  for x in `echo "$thisopt" | sed -e 's,/, ,g'`; do
    case $x in
EOF
for x in `echo "$options" | sed -e 's,/, ,g'`; do
  match=`./print-sysroot-suffix3.sh "$x"`
  echo "$x) optmatch=\"$match\" ;;" >> print-sysroot-suffix2.sh
done
cat >> print-sysroot-suffix2.sh <<\EOF
    esac
    bit=`"$0" "$padding  " "$optstring$x/" "$@"`
    if [ -z "$lastopt" ]; then
      lastopt="$optmatch"
    else
      if [ "$lastbit" = "$bit" ]; then
	lastopt="$lastopt|$optmatch"
      else
	result="$result$lastopt:$lastbit;$n"
	lastopt="$optmatch"
      fi
    fi
    lastbit="$bit"
  done
  bit=`"$0" "$padding  " "$optstring" "$@"`
  if [ "$bit" = "$lastbit" ]; then
    if [ -z "$result" ]; then
      echo "$bit"
    else
      echo "$n%{$result:$bit}"
    fi
  else
    echo "$n%{$result$lastopt:$lastbit;$n:$bit}"
  fi
fi
EOF

chmod +x ./print-sysroot-suffix2.sh
result=`./print-sysroot-suffix2.sh "" "/" $options`
echo "#undef SYSROOT_SUFFIX_SPEC"
echo "#define SYSROOT_SUFFIX_SPEC \"$result\""
rm print-sysroot-suffix2.sh
rm print-sysroot-suffix3.sh
