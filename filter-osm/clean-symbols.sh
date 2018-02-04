# How to fix these characters?
# -e 's/´/\'/' -e 's/’/\'/'
# unexpected EOF while looking for matching `''
# syntax error: unexpected end of file

# exchange non-ascii characters
sed -e 's/ñ/n/' -e 's/é/e/' -e 's/è/e/' -e 's/ê/e/' -e 's/É/E/' -e 's/ó/o/' -e 's/ô/o/' -e 's/í/i/' -e 's/ã/a/' -e 's/â/a/' -e 's/á/a/' -e 's/ä/a/' -e 's/À/A/' -e 's/ü/u/' -e 's/ú/u/' -e 's/–/-/' -e 's/—/-/' $1 > $2
